#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define servoPin 15
#define theWrap 50000

void servo_init();
void servoMove();

int main()
{
    stdio_init_all();
    servo_init(); // init servo

    uint16_t wrap = theWrap;

    while (true) {
        servoMove();
        sleep_ms(500);
    }
}

void servo_init() {
    gpio_set_function(servoPin, GPIO_FUNC_PWM); // Set the LED Pin to be PWM
    uint slice_num = pwm_gpio_to_slice_num(servoPin); // Get PWM slice number
    float div = 60; // must be between 1-255
    pwm_set_clkdiv(slice_num, div); // divider
    uint16_t wrap = theWrap; // when to rollover, must be less than 65535
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true); // turn on the PWM
}

void servoMove() {

    uint16_t wrap = theWrap;
    
    for (int i=0; i<100; i++) {
        pwm_set_gpio_level(servoPin, wrap * (.025+.001*i));
        sleep_ms(20);
    }
    for (int i=0; i<100; i++) {
        pwm_set_gpio_level(servoPin, wrap * (.125-.001*i));
        sleep_ms(20);
    }
}