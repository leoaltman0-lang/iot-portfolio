#include "control.h"
#include <assert.h>

static void sample(MotorControl *motor, uint8_t keys, unsigned ticks,
                   bool enabled, uint8_t duty_a, uint8_t duty_b)
{
    for (unsigned i = 0; i < ticks; ++i) {
        MotorOutput out = motor_tick(motor, keys);
        assert(out.enable == enabled);
        assert(out.duty_a == duty_a);
        assert(out.duty_b == duty_b);
    }
}

int main(void)
{
    MotorControl motor;
    motor_init(&motor, KEY_START);
    sample(&motor, KEY_START, 100, false, 0, 0);
    sample(&motor, 0, 20, false, 0, 0);

    // A short bounce does not start the bridge.
    sample(&motor, KEY_START, 8, false, 0, 0);
    sample(&motor, 0, 1, false, 0, 0);
    sample(&motor, KEY_START, 19, false, 0, 0);
    sample(&motor, KEY_START, 10, true, 0, 0);
    sample(&motor, KEY_START, 50, true, 30, 0);
    sample(&motor, 0, 20, true, 30, 0);
    sample(&motor, KEY_DIR, 30, true, 30, 0);
    sample(&motor, 0, 20, true, 30, 0);

    sample(&motor, KEY_SPEED, 19, true, 30, 0);
    sample(&motor, KEY_SPEED, 40, true, 50, 0);
    sample(&motor, 0, 20, true, 50, 0);
    sample(&motor, KEY_SPEED, 19, true, 50, 0);
    sample(&motor, KEY_SPEED, 40, true, 75, 0);
    sample(&motor, 0, 20, true, 75, 0);
    sample(&motor, KEY_SPEED, 19, true, 75, 0);
    sample(&motor, KEY_SPEED, 40, true, 30, 0);

    // Stop wins immediately and a held START cannot cause a restart.
    sample(&motor, KEY_STOP | KEY_START, 25, false, 0, 0);
    sample(&motor, KEY_START, 1000, false, 0, 0);
    sample(&motor, 0, 20, false, 0, 0);
    sample(&motor, KEY_DIR, 25, false, 0, 0);
    sample(&motor, 0, 20, false, 0, 0);
    sample(&motor, KEY_START, 19, false, 0, 0);
    sample(&motor, KEY_START, 10, true, 0, 0);
    sample(&motor, KEY_START, 5, true, 0, 30);
    sample(&motor, KEY_STOP, 1, false, 0, 0);

    // An early reverse command is discarded, not queued.
    sample(&motor, 0, 25, false, 0, 0);
    sample(&motor, KEY_DIR, 25, false, 0, 0);
    sample(&motor, 0, 800, false, 0, 0);
    sample(&motor, KEY_START, 19, false, 0, 0);
    sample(&motor, KEY_START, 10, true, 0, 0);
    sample(&motor, KEY_START, 5, true, 0, 30);

    motor_init(&motor, 0);
    sample(&motor, 0, 20, false, 0, 0);
    sample(&motor, KEY_START, 19, false, 0, 0);
    sample(&motor, KEY_START, 1, true, 0, 0);
    sample(&motor, KEY_STOP, 1, false, 0, 0);
    return 0;
}
