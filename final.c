#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "ws2812.pio.h"
#include "ssd1306.h"

// Definição dos pinos do joystick
#define VRX 27
#define VRY 26

// Definição dos pinos dos botões
#define ABUTTON 5
#define BBUTTON 6
#define JBUTTON 22

// Definição dos pinos dos buzzers
#define ABUZZER 10
#define BBUZZER 21

// Definição dos pinos dos leds
#define GREEN 11
#define RED 13
#define LED_MATRIX 7

// Definição dos pinos do display
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C

ssd1306_t ssd; // Estrutura do display

#define WIDTH 128
#define HEIGHT 64
#define GRAPH_WIDTH 100  // Largura do gráfico
#define GRAPH_X_OFFSET 16 // Offset horizontal para não colidir com o texto
#define GRAPH_Y_OFFSET 14 // Offset vertical para evitar sobreposição

// Buffers para armazenar valores dos gráficos
uint8_t vrx_graph[GRAPH_WIDTH] = {0};  
uint8_t vry_graph[GRAPH_WIDTH] = {0};
char temp_str[6];
char water_level_str[6];

// Controle de tempo e do gráfico atual
uint32_t volatile current = 0;
uint8_t volatile graph = 0;

// Valores mínimos e máximos para temperatura e nivel de agua
#define MIN_TEMP 200
#define MAX_TEMP 330 * 1.15
#define MIN_WATER_LEVEL 20
#define MAX_WATER_LEVEL 100

// Função para colocar a cor do LED na matriz
void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb);
}

// Função para representar a cor em formato RGB
uint32_t color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(g) << 24) | ((uint32_t)(r) << 16) | ((uint32_t)(b) << 8);
}

// Função para mostrar um símbolo na matriz de leds
void show_alert(){
    uint32_t simbol[25] = {1, 0, 1, 0, 1,  0, 0, 0, 0, 0,  1, 0, 1, 0, 1,  1, 0, 1, 0, 1,  1, 0, 1, 0, 1}; // ! ! !

    for(uint i = 0; i < 25; i++){
        if(simbol[i])
            put_pixel(color(20, 0, 0));
        else
            put_pixel(0);
    }
}

// Função para apagar o display
void display_off(){
    for(int i = 0; i < 25; i++)
        put_pixel(0);
}

// Função para inicial um pino pwm
void init_pwm(uint gpio){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_set_clkdiv(slice, 38.0f);   
    pwm_set_wrap(slice, 12500);      
    pwm_set_enabled(slice, true);
}

// Função para criar os gráficos
void update_graph(uint8_t *graph, float value, float min_value, float max_value) {
    // Normaliza os valores para caber no gráfico, considerando a altura do display
    uint8_t pixel_value = (HEIGHT - GRAPH_Y_OFFSET - 7) - ((value - min_value) / (max_value - min_value)) * (HEIGHT - GRAPH_Y_OFFSET - 7);

    if (pixel_value < 3) pixel_value = 3;
    if (pixel_value > (HEIGHT - GRAPH_Y_OFFSET - 7)) pixel_value = HEIGHT - GRAPH_Y_OFFSET - 7;

    // Move o gráfico para a esquerda
    for (int i = 0; i < GRAPH_WIDTH - 1; i++) 
        graph[i] = graph[i + 1];

    // Adiciona o novo valor na última posição
    graph[GRAPH_WIDTH - 1] = pixel_value;  
}

// Mapeia os valores do joystick para uma faixa de valores especificados
float map_value(uint16_t value, float min_out, float max_out) {
    return min_out + ((float)value / 4095) * (max_out - min_out);
}

// Desenha a tela principal
void main_screen(char* str1, char* str2){
    ssd1306_fill(&ssd, true);
    ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, false, false);
    
    ssd1306_draw_string(&ssd, "Usina Pwr", 28, 5);
    ssd1306_hline(&ssd, 1, 126, 15, false);
    ssd1306_vline(&ssd, 63, 15, 62, false);
    
    ssd1306_draw_string(&ssd, "Temp", 15, 18);
    ssd1306_draw_string(&ssd, "H2O", 86, 18); 
    ssd1306_draw_string(&ssd, str1, 13, 36);
    ssd1306_draw_string(&ssd, str2, 81, 36);
}

// Desenha os gráficos
void draw_graph(uint8_t graph) {
    ssd1306_fill(&ssd, true);
    ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, false, false);

    switch (graph) {
        case 1:
            ssd1306_draw_string(&ssd, "Temp", 4, 5);
            ssd1306_draw_string(&ssd, temp_str, 85, 5);

            // Desenha o gráfico Vrx abaixo do texto
            for (int i = 0; i < GRAPH_WIDTH - 1; i++) 
                ssd1306_line(&ssd, GRAPH_X_OFFSET + i, vrx_graph[i] + GRAPH_Y_OFFSET, GRAPH_X_OFFSET + i + 1, vrx_graph[i + 1] + GRAPH_Y_OFFSET, false);

            break;
        case 2:
            ssd1306_draw_string(&ssd, "H20", 4, 5);
            ssd1306_draw_string(&ssd, water_level_str, 93, 5);

            // Desenha o gráfico Vry abaixo do texto
            for (int i = 0; i < GRAPH_WIDTH - 1; i++) 
                ssd1306_line(&ssd, GRAPH_X_OFFSET + i, vry_graph[i] + GRAPH_Y_OFFSET, GRAPH_X_OFFSET + i + 1, vry_graph[i + 1] + GRAPH_Y_OFFSET, false);

            break;
        case 0:
            main_screen(temp_str, water_level_str);
            break;
    }

    ssd1306_send_data(&ssd);
}

// Função de alerta de valores críticos
void update_alert_status(float temp, float water) {
    bool alert = (temp >= 345 || temp <= 240) || (water >= 75 || water <= 40);
    
    if (alert) {
        gpio_put(RED, alert);
        gpio_put(GREEN, !alert);

        pwm_set_gpio_level(ABUZZER, 6250);
        pwm_set_gpio_level(BBUZZER, 6250);
        
        show_alert();
    }
    else{
        gpio_put(RED, alert);
        gpio_put(GREEN, !alert);
        
        pwm_set_gpio_level(ABUZZER, 0);
        pwm_set_gpio_level(BBUZZER, 0);

        display_off();
    }
}

// interrupção pra alterar a tela do display
void irq_graph(uint gpio, uint32_t event){
    uint32_t time = to_us_since_boot(get_absolute_time());

    if (time - current > 350000){
        current = time;

        if (gpio == ABUTTON)
            graph = 1;

        if (gpio == BBUTTON)
            graph = 2;
        
        if (gpio == JBUTTON)
            graph = 0;
        
    }
}

int main() {
    stdio_init_all();
    uint wrap = 12500;
    
    // inicia o ADC
    adc_init();
    adc_gpio_init(VRX);
    adc_gpio_init(VRY);

    // inicia os botões
    gpio_init(ABUTTON);
    gpio_set_dir(ABUTTON, GPIO_IN);
    gpio_pull_up(ABUTTON);

    gpio_init(BBUTTON);
    gpio_set_dir(BBUTTON, GPIO_IN);
    gpio_pull_up(BBUTTON);

    gpio_init(JBUTTON);
    gpio_set_dir(JBUTTON, GPIO_IN);
    gpio_pull_up(JBUTTON);

    // inicia os leds
    gpio_init(GREEN);
    gpio_set_dir(GREEN, GPIO_OUT);

    gpio_init(RED);
    gpio_set_dir(RED, GPIO_OUT);

    // inicia os buzzers como pwm
    init_pwm(ABUZZER);
    init_pwm(BBUZZER);

    // Configurações do display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);

    // Configura a função de interrupção
    gpio_set_irq_enabled_with_callback(ABUTTON, GPIO_IRQ_EDGE_FALL, true, &irq_graph);
    gpio_set_irq_enabled_with_callback(BBUTTON, GPIO_IRQ_EDGE_FALL, true, &irq_graph);
    gpio_set_irq_enabled_with_callback(JBUTTON, GPIO_IRQ_EDGE_FALL, true, &irq_graph);

    PIO pio = pio0;
    uint sm = 0;

    // Configura a máquina de estados PIO para começar a controlar os LEDs WS2812.
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, LED_MATRIX, 800000, false);

    while (true) {
        adc_select_input(1);
        uint16_t vrx_value = adc_read(); // Le o valor de temperatura
        
        float temperature = map_value(vrx_value, MIN_TEMP, MAX_TEMP);
        snprintf(temp_str, sizeof(temp_str), "%.1f", temperature);

        adc_select_input(0);
        uint16_t vry_value = adc_read(); // le o valor de nivel de agua
        
        float water_level = map_value(vry_value, MIN_WATER_LEVEL, MAX_WATER_LEVEL);
        snprintf(water_level_str, sizeof(water_level_str), "%.1f", water_level);

        // da update nos gráficos dos 2 parâmetros
        update_graph(vrx_graph, temperature, MIN_TEMP, MAX_TEMP);
        update_graph(vry_graph, water_level, MIN_WATER_LEVEL, MAX_WATER_LEVEL);

        // verifica se está em nivel crítico
        update_alert_status(temperature, water_level);

        // desenha o gráfico atual no display
        draw_graph(graph);

        printf("Temp: %.1fC | H2O: %.1f%%\n", temperature, water_level);
        sleep_ms(100);
    }
}

