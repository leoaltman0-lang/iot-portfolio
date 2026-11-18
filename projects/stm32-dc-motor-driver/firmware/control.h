#ifndef CONTROL_H
#define CONTROL_H

#include <stdbool.h>
#include <stdint.h>

enum {
    KEY_START = 1u << 0,
    KEY_STOP  = 1u << 1,
    KEY_DIR   = 1u << 2,
    KEY_SPEED = 1u << 3,
    KEY_MASK  = 0x0f
};

typedef enum { MOTOR_OFF, MOTOR_PRECHARGE, MOTOR_RUNNING } MotorMode;

typedef struct {
    MotorMode mode;
    bool reverse;
    bool start_armed;
    uint8_t speed;
    uint8_t stable_keys;
    uint8_t key_age[4];
    uint16_t wait_ms;
} MotorControl;

typedef struct {
    bool enable;
    uint8_t duty_a;
    uint8_t duty_b;
} MotorOutput;

void motor_init(MotorControl *motor, uint8_t pressed);
MotorOutput motor_tick(MotorControl *motor, uint8_t pressed);

#endif
