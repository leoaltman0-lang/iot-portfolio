#include "control.h"

enum {
    DEBOUNCE_MS = 20,
    PRECHARGE_MS = 10,
    RESTART_LOCKOUT_MS = 800
};

static const uint8_t duty_table[] = {30, 50, 75};

void motor_init(MotorControl *motor, uint8_t pressed)
{
    motor->mode = MOTOR_OFF;
    motor->reverse = false;
    motor->start_armed = false;
    motor->speed = 0;
    motor->stable_keys = pressed & KEY_MASK;
    for (unsigned i = 0; i < 4; ++i) {
        motor->key_age[i] = 0;
    }
    motor->wait_ms = 0;
}

static uint8_t update_keys(MotorControl *motor, uint8_t pressed)
{
    uint8_t rising = 0;
    for (unsigned i = 0; i < 4; ++i) {
        uint8_t bit = (uint8_t)(1u << i);
        if (((motor->stable_keys ^ pressed) & bit) == 0u) {
            motor->key_age[i] = 0;
        } else if (++motor->key_age[i] == DEBOUNCE_MS) {
            motor->stable_keys ^= bit;
            motor->key_age[i] = 0;
            rising |= pressed & bit;
        }
    }
    return rising;
}

MotorOutput motor_tick(MotorControl *motor, uint8_t pressed)
{
    uint8_t rising = update_keys(motor, pressed);
    uint8_t active = pressed | motor->stable_keys;

    if (motor->wait_ms != 0u) {
        --motor->wait_ms;
    }

    // A raw stop press takes priority before the debounce interval expires.
    if ((active & KEY_STOP) != 0u) {
        if (motor->mode != MOTOR_OFF) {
            motor->mode = MOTOR_OFF;
            motor->wait_ms = RESTART_LOCKOUT_MS;
        }
        motor->start_armed = false;
        return (MotorOutput){false, 0, 0};
    }

    if ((active & KEY_START) == 0u) {
        motor->start_armed = true;
    }
    if ((rising & KEY_SPEED) != 0u) {
        motor->speed = (uint8_t)((motor->speed + 1u) % 3u);
    }
    if ((rising & KEY_DIR) != 0u && motor->mode == MOTOR_OFF &&
        motor->wait_ms == 0u && (active & KEY_START) == 0u) {
        motor->reverse = !motor->reverse;
    }

    if ((rising & KEY_START) != 0u) {
        if (motor->start_armed && motor->mode == MOTOR_OFF &&
            motor->wait_ms == 0u && (active & KEY_DIR) == 0u) {
            motor->mode = MOTOR_PRECHARGE;
            motor->wait_ms = PRECHARGE_MS;
        }
        motor->start_armed = false;
    }
    if (motor->mode == MOTOR_PRECHARGE && motor->wait_ms == 0u) {
        motor->mode = MOTOR_RUNNING;
    }

    MotorOutput out = {motor->mode != MOTOR_OFF, 0, 0};
    if (motor->mode == MOTOR_RUNNING) {
        if (motor->reverse) {
            out.duty_b = duty_table[motor->speed];
        } else {
            out.duty_a = duty_table[motor->speed];
        }
    }
    return out;
}
