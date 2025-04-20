#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
    stdio_init_all();

    while (!stdio_usb_connected()) { // waits for USB connection
        sleep_ms(100);
    }

    volatile float f1, f2;
    printf("Enter two floats to use: \n");
    scanf("%f %f", &f1, &f2);
    volatile float f_add, f_sub, f_mult, f_div;

    // for addition

    absolute_time_t tadd = get_absolute_time();
    uint64_t t = to_us_since_boot(tadd);
    for (int i = 0; i < 1000; i++) {
        f_add = f1+f2;
    }
    absolute_time_t tadd_after = get_absolute_time();
    uint64_t t_after = to_us_since_boot(tadd_after);
    uint64_t result = ((t_after - t) / 1000.0) / (1.00/150.0);

    printf("Clock Cycles for Addition: %llu\n", result);

    tadd = get_absolute_time();
    t = to_us_since_boot(tadd);
    for (int i = 0; i < 1000; i++) {
        f_sub = f1-f2;
    }
    tadd_after = get_absolute_time();
    t_after = to_us_since_boot(tadd_after);
    result = ((t_after - t) / 1000.0) / (1.00/150.0);

    printf("Clock Cycles for Subtraction: %llu\n", result);

    tadd = get_absolute_time();
    t = to_us_since_boot(tadd);
    for (int i = 0; i < 1000; i++) {
        f_mult = f1*f2;
    }
    tadd_after = get_absolute_time();
    t_after = to_us_since_boot(tadd_after);
    result = ((t_after - t) / 1000.0) / (1.00/150.0);

    printf("Clock Cycles for Multiplication: %llu\n", result);

    tadd = get_absolute_time();
    t = to_us_since_boot(tadd);
    for (int i = 0; i < 1000; i++) {
        f_div = f1/f2;
    }
    tadd_after = get_absolute_time();
    t_after = to_us_since_boot(tadd_after);
    result = ((t_after - t) / 1000.0) / (1.00/150.0);

    printf("Clock Cycles for Division: %llu\n", result);

    printf("\nResults: \n%f+%f=%f \n%f-%f=%f \n%f*%f=%f \n%f/%f=%f\n", f1,f2,f_add, f1,f2,f_sub, f1,f2,f_mult, f1,f2,f_div);
}
