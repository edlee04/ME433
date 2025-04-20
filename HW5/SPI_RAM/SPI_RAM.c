#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_CS_RAM 13
#define PIN_SCK  18
#define PIN_MOSI 19

static float sine[1000];
static float read_sine[1000];
static float radian = 0;

static inline void cs_select(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 0);
    asm volatile("nop \n nop \n nop"); // FIXME
}

static inline void cs_deselect(uint cs_pin) {
    asm volatile("nop \n nop \n nop"); // FIXME
    gpio_put(cs_pin, 1);
    asm volatile("nop \n nop \n nop"); // FIXME
}

union FloatInt {
    float f;
    uint32_t i;
};

// function prototypes
void spi_ram_init(); // to enable sequential operation
void spi_ram_write(uint16_t address, float voltage); // sends float voltage numbers to the RAM (float = 4 bytes)
float spi_ram_read(uint16_t address); // reads 4 bytes (float) from the RAM
void writeDac(int channel, float voltage);
void sineWave();

int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) { // waits for USB connection
        sleep_ms(100);
    }

    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_CS_RAM,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);
    gpio_set_dir(PIN_CS_RAM, GPIO_OUT);
    gpio_put(PIN_CS_RAM, 1);
    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    spi_ram_init();
    sineWave();

    for (int i = 0; i < 1000; i++) {
        uint16_t address = 4*i;
        spi_ram_write(address, sine[i]);
        printf("Writing Sin Wave %.4f\n", sine[i]);
    }

    printf("Writing into RAM Done!\n");

    while (true) {
        for (int i = 0; i < 1000; i++) {
            uint16_t address = 4*i;
            printf("Address: %d\n", address);
            read_sine[i] = spi_ram_read(address);

            printf("Original sine wave: %.4f \nRead sine wave: %.4f\n\n", sine[i], read_sine[i]);

            writeDac(0, read_sine[i]); // send to the DAC
            writeDac(1, sine[i]);
            sleep_ms(1);
        }
    }
    
}

void spi_ram_init() { // sends two bytes to set the mode to sequential
    uint8_t initb[2]; 
    initb[0] = 0b00000001;
    initb[1] = 0b01000000;

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, initb, 2); // send init_bytes
    cs_deselect(PIN_CS_RAM);
}

void spi_ram_write(uint16_t address, float voltage) { // takes address, pointer to data
    uint8_t write_init[3], write_data[4];

    write_init[0] = 0b00000010;
    write_init[1] = ((address >> 8) & 0b11111111);
    write_init[2] = (address & 0b11111111);

    union FloatInt num;
    num.f = voltage;

    write_data[0] = (num.i >> 24) & 0b11111111;
    write_data[1] = (num.i >> 16) & 0b11111111;
    write_data[2] = (num.i >> 8) & 0b11111111;
    write_data[3] = (num.i & 0b11111111);

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, write_init, 3);
    spi_write_blocking(SPI_PORT, write_data, 4);
    cs_deselect(PIN_CS_RAM);
}

float spi_ram_read(uint16_t address) { // takes 16bit address
    uint8_t write[3], read[4];

    write[0] = 0b00000011;
    write[1] = (address >> 8) & 0xFF; // gets first 8 bits of address
    write[2] = address & 0xFF; // gets last 8 bits of address

    cs_select(PIN_CS_RAM);
    spi_write_blocking(SPI_PORT, write, 3);
    spi_read_blocking(SPI_PORT, 0, read, 4);
    cs_deselect(PIN_CS_RAM);

    union FloatInt num;
    num.i = 0;
    // reconstruct a float number by leftmost 8 bits being read[3], etc
    num.i = num.i | (read[0]) << 24;
    num.i = num.i | (read[1]) << 16;
    num.i = num.i | (read[2]) << 8;
    num.i = num.i | (read[3]);

    return num.f;
}

void writeDac(int channel, float voltage) {

    if (voltage < 0) {
        voltage = 0;
    }
    if (voltage > 3.3) {
        voltage = 3.3;
    }

    uint8_t data[2];
    int len = 2;

    uint16_t val = (uint16_t) ((voltage / 3.3) * 1023.0);

    data[0] = ((channel & 0x01) << 7) | (0b111 << 4) | ((val >> 6) & 0x0F);
    data[1] = ((val & 0x3F) << 2);

    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS);
}

void sineWave() {
    for (int i = 0; i <1000; i++) {
        sine[i] = 1.65*sin(radian) + 1.65;
        radian += 0.00628318;
    }
}