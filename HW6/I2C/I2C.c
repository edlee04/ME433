#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define I2C_PORT i2c0
#define I2C_SDA 8
#define I2C_SCL 9
#define HEART_PIN 25 // for testing heartbeat
#define ADDR 0b0100000

// prototypes
void heartbeat();
void setPin(uint8_t chip_adr, uint8_t reg, uint8_t value);
uint8_t readPin(uint8_t chip_adr, uint8_t reg);

int main()
{
    stdio_init_all();

    // initialize the HEARTBEAT pin
    gpio_init(HEART_PIN);
    gpio_set_dir(HEART_PIN, GPIO_OUT);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400*1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

   setPin(ADDR, 0x00, 0b00000001); // initializes the GP0 and GP7 pins

    while (true) {
        heartbeat(); // heartbeat GP25 testing

        uint8_t input = readPin(ADDR, 0x09);
        int press = input & 0b1;

        if (press) {
            setPin(ADDR, 0x0A, 0b10000000);
        }
        else {
            setPin(ADDR, 0x0A, 0b00000000);
        }

    }
}

void heartbeat() {
    gpio_put(HEART_PIN, true);
    sleep_ms(25);
    gpio_put(HEART_PIN, false);
    sleep_ms(25);
}

void setPin(uint8_t chip_adr, uint8_t reg, uint8_t value) {
    uint8_t buffer[2];
    buffer[0] = reg;
    buffer[1] = value;

    i2c_write_blocking(I2C_PORT, chip_adr, buffer, 2, false);
}

uint8_t readPin(uint8_t chip_adr, uint8_t reg) {
    uint8_t result = 0;
    i2c_write_blocking(I2C_PORT, chip_adr, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, chip_adr, &result, 1, false);

    return result;
}