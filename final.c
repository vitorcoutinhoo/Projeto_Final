#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "ssd1306.h"

#define VRX 27
#define VRY 26
#define BUTTON 22

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

ssd1306_t ssd;

#define WIDTH 128
#define HEIGHT 64
#define GRAPH_WIDTH 100  // Largura do gráfico
#define GRAPH_X_OFFSET 16 // Offset horizontal para não colidir com o texto
#define GRAPH_Y_OFFSET 14 // Offset vertical para evitar sobreposição

// Buffers para armazenar valores dos gráficos
uint8_t vrx_graph[GRAPH_WIDTH] = {0};  
uint8_t vry_graph[GRAPH_WIDTH] = {0};

uint32_t volatile current = 0;
bool volatile graph = true;

void update_graph(uint8_t *graph, uint16_t value) {
    // Normaliza o valor para caber no display (invertido para gráfico ficar correto)
    uint8_t pixel_value = (HEIGHT - GRAPH_Y_OFFSET - 4) - (value * (HEIGHT - GRAPH_Y_OFFSET - 4) / 4095);
    
    // Move os pontos do gráfico para a esquerda
    for (int i = 0; i < GRAPH_WIDTH - 1; i++) {
        graph[i] = graph[i + 1];
    }
    graph[GRAPH_WIDTH - 1] = pixel_value;  // Adiciona o novo valor na última posição
}

void draw_graph(bool graph) {
    ssd1306_fill(&ssd, true);
    ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, false, false);

    if(graph){
        ssd1306_draw_string(&ssd, "Vrx:", 3, 4);

        // Desenha o gráfico Vrx abaixo do texto
        for (int i = 0; i < GRAPH_WIDTH - 1; i++) 
            ssd1306_line(&ssd, GRAPH_X_OFFSET + i, vrx_graph[i] + GRAPH_Y_OFFSET, GRAPH_X_OFFSET + i + 1, vrx_graph[i + 1] + GRAPH_Y_OFFSET, false);
        
    }
    else {
        ssd1306_draw_string(&ssd, "Vry:", 3, 4);

        // Desenha o gráfico Vry abaixo do texto
        for (int i = 0; i < GRAPH_WIDTH - 1; i++) 
            ssd1306_line(&ssd, GRAPH_X_OFFSET + i, vry_graph[i] + GRAPH_Y_OFFSET, GRAPH_X_OFFSET + i + 1, vry_graph[i + 1] + GRAPH_Y_OFFSET, false);
        
    }

    ssd1306_send_data(&ssd);
}

void irq_graph(){
    uint32_t time = to_us_since_boot(get_absolute_time());

    if (time - current > 250000){
        current = time;

        graph = !graph;
    }
}


int main() {
    stdio_init_all();
    
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    gpio_init(BUTTON);
    gpio_set_dir(BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON);

    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    gpio_set_irq_enabled_with_callback(BUTTON, GPIO_IRQ_EDGE_FALL, true, &irq_graph);

    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read();

        adc_select_input(0);
        uint16_t vry_value = adc_read();

        update_graph(vrx_graph, vrx_value);
        update_graph(vry_graph, vry_value);

        draw_graph(graph);

        printf("vrx: %d | vry: %d\n", vrx_value, vry_value);
        sleep_ms(100);
    }
}

