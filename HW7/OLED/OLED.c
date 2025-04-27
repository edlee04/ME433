#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "font.h"
#include "ssd1306.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define HEART_PIN 25 // for testing heartbeat
#define ADC0 26

void heartbeat();
void drawMessage(int x, int y, char * m);
void drawLetter(int x, int y, char c);

int main()
{
    stdio_init_all();

    // initialize the HEARTBEAT pin
    gpio_init(HEART_PIN);
    gpio_set_dir(HEART_PIN, GPIO_OUT);

    // initialize the ADC0 pin
    adc_init(); // init the adc module
    adc_gpio_init(26); // set ADC0 pin to be adc input instead of GPIO
    adc_select_input(0); // select to read from ADC0

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    ssd1306_setup(); // init function
    ssd1306_clear(); // clears all pixels
    ssd1306_update(); // updates to clear

    while (true) {

        unsigned int t1 = to_us_since_boot(get_absolute_time());

        char message[25];
        float voltage = (adc_read() / 4095.0f) * 3.3f;

        sprintf(message, "Voltage: %.9f", voltage);
        drawMessage(0, 0, message);
        ssd1306_update();

        unsigned int t2 = to_us_since_boot(get_absolute_time());  
        unsigned int delta = t2 - t1;
        float fps = 1000000.0 / delta;
        sprintf(message, "FPS: %.8f", fps);
        drawMessage(0, 9, message);
        ssd1306_update();

        heartbeat();
    }
}

void heartbeat() {
    gpio_put(HEART_PIN, true);
    sleep_ms(25);
    gpio_put(HEART_PIN, false);
    sleep_ms(25);
}

void drawMessage(int x, int y, char * m) {
    int i = 0;
    while (m[i] != 0) {
        drawLetter(x + i*5, y, m[i]);
        i++;
    }
}

void drawLetter(int x, int y, char c) {
    int row, col;

    row = c - 0x20; // takes ASCII number of c and subtracts 0x20 to use in font.h

    for (int i=0; i<5; i++) {
        char byte = ASCII[row][i]; // sets 8bit

        for (int j=0; j<8; j++) {
            char command = (byte >> j) & 0b1;
            ssd1306_drawPixel(x+i, y+j, command);
        }

    }

}