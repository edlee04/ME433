#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>

#define ENABLE 16 // GPIO with PWM for speed control
#define PHASE  17 // GPIO for direction

// Set motor speed and direction
// speed_percent: 0-100 (duty cycle)
// forward: true = forward, false = reverse
void set_speed(uint8_t speed, bool forward) {
    if (speed > 100) {
        speed = 100;
    }
    if (speed < 0) {
        speed = 0;
    }

    // Set direction
    gpio_put(PHASE, !(forward)); // LOW = forward, HIGH = reverse

    // Calculate and set PWM duty cycle
    uint16_t actual = (speed * 65535) / 100;
    pwm_set_gpio_level(ENABLE, actual);
}

int main() {
    stdio_init_all();

    // Set PHASE as GPIO output
    gpio_init(PHASE);
    gpio_set_dir(PHASE, GPIO_OUT);

    // Set ENABLE as PWM
    gpio_set_function(ENABLE, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(ENABLE);

    // Set PWM frequency (e.g., ~10 kHz)
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 4.0f);  // adjust for desired freq
    pwm_init(slice, &config, true);

    set_speed(0, true);

    int speed = 0;
    bool direction = true;

    while (true) {

        // ask user for character input
        char input;
        scanf("%c", &input);

        if (input == 'a') { // press 'a' to add to speed int
            speed += 1;
        }
        else if (input == 's') {  // press 's' to subtract from speed int
            speed -= 1;
        }
        else {
            printf("Invalid Char\n");
        }

        // cap the speeds
        if (speed > 100) {
            speed = 100;
        }
        if (speed < -100) {
            speed = -100;
        }

        printf("Speed is now: %d\n", speed);

        // change direction if needed
        if (speed > 0) {
            direction = false; // false is forward
        }
        else {
            direction = true; // true is reverse
        }

        set_speed(abs(speed), direction);

        sleep_ms(100);
    }
}
