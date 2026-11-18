#include "control.h"

#define REG32(address) (*(volatile uint32_t *)(address))
#define RCC_APB2ENR   REG32(0x40021018u)
#define RCC_APB1ENR   REG32(0x4002101cu)
#define GPIOA_CRL     REG32(0x40010800u)
#define GPIOA_IDR     REG32(0x40010808u)
#define GPIOA_BSRR    REG32(0x40010810u)
#define GPIOB_CRH     REG32(0x40010c04u)
#define GPIOB_BSRR    REG32(0x40010c10u)
#define TIM3_CR1      REG32(0x40000400u)
#define TIM3_SR       REG32(0x40000410u)
#define TIM3_EGR      REG32(0x40000414u)
#define TIM3_CCMR1    REG32(0x40000418u)
#define TIM3_CCER     REG32(0x40000420u)
#define TIM3_PSC      REG32(0x40000428u)
#define TIM3_ARR      REG32(0x4000042cu)
#define TIM3_CCR1     REG32(0x40000434u)
#define TIM3_CCR2     REG32(0x40000438u)
#define SYST_CSR      REG32(0xe000e010u)
#define SYST_RVR      REG32(0xe000e014u)
#define SYST_CVR      REG32(0xe000e018u)

enum {
    CLOCK_HZ = 8000000u,        // Reset HSI clock with APB1 prescaler at /1.
    PWM_HZ = 20000u,
    PWM_COUNTS = CLOCK_HZ / PWM_HZ,
    EN_SET = 1u << 12,
    EN_RESET = 1u << (12 + 16),
    SYST_COUNTFLAG = 1u << 16
};

_Static_assert(PWM_COUNTS % 100u == 0u, "PWM period must support integer percent");

static uint8_t read_keys(void)
{
    return (uint8_t)(~GPIOA_IDR & KEY_MASK);
}

static void gpio_timer_init(void)
{
    RCC_APB2ENR |= (1u << 2) | (1u << 3);  // IOPAEN, IOPBEN
    RCC_APB1ENR |= 1u << 1;                // TIM3EN
    (void)RCC_APB2ENR;
    (void)RCC_APB1ENR;

    GPIOB_BSRR = EN_RESET;
    GPIOB_CRH = (GPIOB_CRH & ~(0xfu << 16)) | (0x2u << 16);  // PB12 push-pull.
    GPIOA_BSRR = KEY_MASK;
    GPIOA_CRL = (GPIOA_CRL & ~0xffffu) | 0x8888u;           // PA0-3 input pull-up.

    TIM3_PSC = 0;
    TIM3_ARR = PWM_COUNTS - 1u;
    TIM3_CCR1 = 0;
    TIM3_CCR2 = 0;
    TIM3_CCMR1 = 0x6868u;  // PWM mode 1 and compare preload on CH1/CH2.
    TIM3_CCER = (1u << 0) | (1u << 4);
    TIM3_EGR = 1u;         // Transfer the initial preload values.
    TIM3_SR = 0;
    TIM3_CR1 = 1u;
    GPIOA_CRL = (GPIOA_CRL & 0x00ffffffu) | 0xaa000000u;    // PA6/7 AF push-pull.
}

static void apply_output(MotorOutput out)
{
    if (!out.enable) {
        GPIOB_BSRR = EN_RESET;
    }
    TIM3_CCR1 = out.duty_a * (PWM_COUNTS / 100u);
    TIM3_CCR2 = out.duty_b * (PWM_COUNTS / 100u);
    if (out.enable) {
        GPIOB_BSRR = EN_SET;
    }
}

void fault_stop(void)
{
    RCC_APB2ENR |= 1u << 3;
    (void)RCC_APB2ENR;
    GPIOB_BSRR = EN_RESET;
    GPIOB_CRH = (GPIOB_CRH & ~(0xfu << 16)) | (0x2u << 16);
    for (;;) {
    }
}

int main(void)
{
    MotorControl motor;
    gpio_timer_init();
    motor_init(&motor, read_keys());
    SYST_RVR = CLOCK_HZ / 1000u - 1u;
    SYST_CVR = 0;
    SYST_CSR = (1u << 2) | 1u;  // Core clock, no interrupt.
    for (;;) {
        if ((SYST_CSR & SYST_COUNTFLAG) != 0u) {
            apply_output(motor_tick(&motor, read_keys()));
        }
    }
}
