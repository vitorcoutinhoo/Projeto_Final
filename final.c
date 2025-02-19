#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define vrx 27
#define vry 26

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(vrx);
    adc_gpio_init(vry);

    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read();

        adc_select_input(0);
        uint16_t vry_value = adc_read();

        printf("vrx: %d | vry: %d\n", vrx_value, vry_value);
        sleep_ms(10);
    }
}
