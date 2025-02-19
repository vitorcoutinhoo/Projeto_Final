#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define vrx 27
#define vry 26

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
ssd1306_t ssd;

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(vrx);
    adc_gpio_init(vry);

    // Taxa de atualização do display em 400khz
    i2c_init(I2C_PORT, 400 * 1000);

    // Seta a função i²c e coloca os pinos do display em pull up
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); 
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA); 
    gpio_pull_up(I2C_SCL);

    // Inicializa e configura o display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); 
    ssd1306_config(&ssd); 
    ssd1306_send_data(&ssd);

    char vrx_str[5];
    char vry_str[5];

    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read();
        sprintf(vrx_str, "%d", vrx_value);

        adc_select_input(0);
        uint16_t vry_value = adc_read();
        sprintf(vry_str, "%d", vry_value);

        ssd1306_fill(&ssd, true);
        ssd1306_rect(&ssd, 0, 0, 128, 64, false, false);
        ssd1306_draw_string(&ssd, "Vrx ", 20, 20);
        ssd1306_draw_string(&ssd, vrx_str, 52, 20);
        ssd1306_draw_string(&ssd, "Vry ", 20, 40);
        ssd1306_draw_string(&ssd, vry_str, 52, 40);
        ssd1306_send_data(&ssd);


        printf("vrx: %d | vry: %d\n", vrx_value, vry_value);
        sleep_ms(10);
    }
}
