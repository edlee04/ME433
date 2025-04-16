#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include <math.h>

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define PIN_MISO 16
#define PIN_CS   17
#define PIN_SCK  18
#define PIN_MOSI 19

static float sine_wave[50];
static float triangle_wave[100];

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

void writeDac(int channel, float voltage);

void makeSin() {
    float t = 0;
    for (int i = 0; i < 50; i++) {
        sine_wave[i] = 1.65 * sin(t) + 1.65;
        t += 0.1256637; // 2pi / 50 = .1256637
    }
}

void makeTriangle() {
    for (int i = 0; i < 25; i++) {
        triangle_wave[i] = (1.65 * i / 25.0) + 1.65;
        triangle_wave[i+25] = (3.3 - (1.65 * i / 25.0));
        triangle_wave[i+50] = (-1.65 * i / 25.0) + 1.65;
        triangle_wave[i+75] = (1.65 * i/25.0);
    }
}

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS,   GPIO_FUNC_SIO);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    makeSin();
    makeTriangle();

    while (true) {

        for (int i = 0; i < 100; i++) {
            printf("Sine Wave: %f\n", sine_wave[i]);
            printf("Triangle Wave: %f\n", triangle_wave[i]);

            writeDac(0, sine_wave[(i % 50)]);
            writeDac(1, triangle_wave[i]); 
            sleep_ms(10);
        }
    }
}

void writeDac(int channel, float voltage) {
    uint8_t data[2];
    int len = 2;

    uint16_t val = (uint16_t) ((voltage / 3.3) * 1023.0);

    data[0] = ((channel & 0x01) << 7) | (0b111 << 4) | ((val >> 6) & 0x0F);
    data[1] = ((val & 0x3F) << 2);

    printf("Data0: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (data[0] >> i) & 1);
    }
    printf("\n");

    printf("Data1: ");
    for (int i = 7; i >= 0; i--) {
        printf("%d", (data[1] >> i) & 1);
    }
    printf("\n");
    
    cs_select(PIN_CS);
    spi_write_blocking(SPI_PORT, data, len);
    cs_deselect(PIN_CS);
}