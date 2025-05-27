#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "math.h"
#include <stdlib.h>

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 16
#define I2C_SCL 17
#define HEART_PIN 25 // for testing heartbeat

#define WHO_AM_I_REG 0x75
#define MPU6050_ADDR 0x68
#define PWR_MGMT_1_REG 0x6B
#define ACCEL_CONFIG_REG 0x1C
#define GYRO_CONFIG_REG 0x1B
#define CONFIG_REG 0x1A

// sensor data register, 0x3B is ACCEL_XOUT_H
#define DATA_REG 0x3B

void heartbeat();
void mpu6050_init();
void read_data(int16_t* output);
void draw(float ax, float ay);

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

    uint8_t reg = WHO_AM_I_REG;
    uint8_t identity = 0;

    // receive back address of MPU6050 (which is 0x68)
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &identity, 1, false);

    printf("MPU6050 Address: %02X\n", identity);

    // stores all data from MPU6050
    // first 3 are ACCEL XYZ, next is TEMP, last 3 are GYRO XYZ
    int16_t allData[7];

    // init the MPU6050
    mpu6050_init();
    sleep_ms(100);

    ssd1306_setup(); // init function
    ssd1306_clear(); // clears all pixels
    ssd1306_update(); // updates to clear

    while (true) {

        heartbeat();

        read_data(allData);

        printf("ACCELX: %.3f\n", allData[0]*0.000061);
        printf("ACCELY: %.3f\n", allData[1]*0.000061);
        printf("ACCELZ: %.3f\n", allData[2]*0.000061);

        ssd1306_drawPixel(64, 16, true);
        ssd1306_update();

        draw(allData[0]*0.000061, allData[1]*0.000061); //draws pixels based on y or x acceleration
    }
}

void heartbeat() {
    gpio_put(HEART_PIN, true);
    sleep_ms(25);
    gpio_put(HEART_PIN, false);
    sleep_ms(25);
}

void mpu6050_init() {

    // wake up call
    uint8_t data[2];
    data[0] = PWR_MGMT_1_REG;
    data[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, data, 2, false);

    // enable accelerometer, sets sensitivity to 2G
    data[0] = ACCEL_CONFIG_REG;
    data[1] = 0x00;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, data, 2, false);

    // enable gyroscope, sets sensitivity to +/- 2000 dps
    data[0] = GYRO_CONFIG_REG;
    data[1] = 0b00011000;
    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, data, 2, false);
}

void read_data(int16_t* output) {

    uint8_t reg = DATA_REG;
    uint8_t data[14];

    i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &reg, 1, true);
    i2c_read_blocking(I2C_PORT, MPU6050_ADDR, data, 14, false);

    for (int i = 0; i < 7; i++) {
        output[i] = (data[2*i] << 8) | data[2*i+1];
    }
}

void draw(float ax, float ay) {
    // Clamp acceleration to [-2g, 2g]
    if (ax > 2.0) ax = 2.0;
    if (ax < -2.0) ax = -2.0;
    if (ay > 2.0) ay = 2.0;
    if (ay < -2.0) ay = -2.0;

    // Clear display before drawing
    ssd1306_clear();

    // Convert acceleration to pixel lengths (scale to reasonable lengths)
    int len_x = (int)(ax * 30); // horizontal scale
    int len_y = (int)(ay * 20); // vertical scale

    int x0 = 64; // center X
    int y0 = 16; // center Y

    int dir = 0;

    // Draw horizontal line (accel X)
    if (len_x != 0) {
        if (len_x > 0) {
            dir = -1
        }
        else {
            dir = 1;
        }
        for (int i = 1; i <= abs(len_x); i++) {
            int x = x0 + i * dir;
            if (x >= 0 && x < 128)
                ssd1306_drawPixel(x, y0, true);
        }
    }

    // Draw vertical line (accel Y)
    if (len_y != 0) {
        if (len_y > 0) {
            dir = 1;
        }
        else {
            dir = -1;
        }
        for (int i = 1; i <= abs(len_y); i++) {
            int y = y0 + i * dir;
            if (y >= 0 && y < 64)
                ssd1306_drawPixel(x0, y, true);
        }
    }

    // Draw center point
    ssd1306_drawPixel(x0, y0, true);

    // Update display with all the new pixels
    ssd1306_update();
}
