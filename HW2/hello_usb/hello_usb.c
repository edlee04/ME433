/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
 
// Initialise pins
#define LED_PIN 16
#define GPIO_WATCH_PIN 2
  
volatile int count = 0;
volatile int button_press = 0;
 
void pico_led_init(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}
  
// Function to turn the LED on and off
void pico_set_led(bool led_on) {
    gpio_put(LED_PIN, led_on);
}
  
// ISR
void gpio_callback(uint gpio, uint32_t events) {
    button_press = 1;

    count += 1;
    // toggle the led
    if (count % 2 == 0) {
        pico_set_led(false);
    }
    else {
        pico_set_led(true);
    }

    // print the total # of button presses
    printf("Button Presses: %d\n", count);
}
  
int main() {
    pico_led_init();
    stdio_init_all();
 
    // configure button
    gpio_init(GPIO_WATCH_PIN);
    gpio_set_dir(GPIO_WATCH_PIN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(GPIO_WATCH_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while(1) { 
        // button debouncing
        if (button_press) {
            sleep_ms(100);
            button_press = 0;
        }
    }
}