#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

int main() {
    stdio_init_all();
    while (!stdio_usb_connected()) { // waits for USB connection
        sleep_ms(100);
    }

    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    gpio_init(16); // init the LED pin
    gpio_set_dir(16, GPIO_OUT);

    gpio_init(2); // init the button
    gpio_set_dir(2, GPIO_IN);

    gpio_put(16, 1); // turn led on
    printf("Press the Button to Begin Program!\n");
    while (!gpio_get(2)) { // waits for button press
        ;
    }
    printf("Button has been Pressed!\n");
    gpio_put(16, 0); // turn off led
 
    while (1) {

        printf("Enter a number of Analog Samples to Take (1-100): \n");

        int times;
        scanf("%d", &times); // get user input

        printf("Entered: %d\n", times); // print back user input

        if ((times < 1) | (times > 100)) { // check for bounds
            printf("Invalid Input\n");
            return 1; // return 1 if invalid
        }

        for (int i=0; i < times; i++) { // send ADC value with 100Hz rate
            uint16_t result = adc_read();
            float voltage = (result / 4096.0) * 3.3;
            printf("ADC Value: %.2f\r\n", voltage);
            sleep_ms(10);
        }
    }
}