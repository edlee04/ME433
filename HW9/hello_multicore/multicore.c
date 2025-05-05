#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/adc.h"

#define FLAG_VALUE 123
#define LED_PIN 15

#define ADC_READ 0
#define TURN_ON 1
#define TURN_OFF 2
#define CORE1_RETURN 321

uint16_t adc0_voltage = 0;

void core1_entry() {

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // initialize the ADC0 pin
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    while (1) {
        uint32_t g = multicore_fifo_pop_blocking();

        if (g == ADC_READ) {
            adc0_voltage = adc_read();
            multicore_fifo_push_blocking(CORE1_RETURN);

        }

        else if (g == TURN_ON) {
            gpio_put(LED_PIN, true);
            multicore_fifo_push_blocking(CORE1_RETURN);
        }

        else if (g == TURN_OFF) {
            gpio_put(LED_PIN, false);
            multicore_fifo_push_blocking(CORE1_RETURN);
        }

        else {
            printf("Something wrong on core 1\n");
            break;
        }
    }
}

int main() {
    
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }
    
    printf("Hello, multicore!\n");

    multicore_launch_core1(core1_entry);

    char message[50];
    int user;

    while (1) {
        printf("Enter a Number (0, 1, 2): \n");
        scanf("%s", message);
        printf("You entered %s\n", message);

        user = atoi(message);

        if (user == 0) {
            multicore_fifo_push_blocking(ADC_READ);
            uint32_t g = multicore_fifo_pop_blocking();

            if (g != CORE1_RETURN) {
                printf("Something wrong on core 0.\n");
                break;
            }

            float voltage = adc0_voltage * 3.3 / 4095.0;
            printf("ADC0 Voltage: %lf \n", voltage);
        }

        else if (user == 1) {
            multicore_fifo_push_blocking(TURN_ON);

            uint32_t g = multicore_fifo_pop_blocking();

            if (g != CORE1_RETURN) {
                printf("Something wrong on core 0.\n");
                break;
            }
        }

        else if (user == 2) {
            multicore_fifo_push_blocking(TURN_OFF);
        
            uint32_t g = multicore_fifo_pop_blocking();
        
            if (g != CORE1_RETURN) {
                printf("Something wrong on core 0.\n");
                break;
            }
        }
    }

}
