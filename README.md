## Monitoramento de Usina PWR - Projeto de Sistemas Embarcados

Este projeto implementa um sistema de monitoramento para uma usina PWR (Reator de Água Pressurizada) utilizando um microcontrolador Raspberry Pi Pico. O sistema coleta dados de temperatura e nível de água simulados por um joystick, exibe as informações em um display OLED e aciona alertas visuais e sonoros em condições críticas.

### 📌 Funcionalidades

- Leitura de Sensores: O joystick é utilizado para simular a temperatura e o nível de água da usina.

- Exibição de Dados: Os valores de temperatura e nível de água são mostrados em um display OLED SSD1306.

- Alertas Visuais e Sonoros: Leds, buzzer e uma matriz de LED WS2812 indicam condições críticas.

- Gráficos em Tempo Real: O display apresenta gráficos históricos das medições.

- Mudança de Tela: Botões físicos permitem alternar entre as telas de exibição.

- Modo de Simulação: Permite testar diferentes cenários de temperatura e nível de água.

- Geração de Logs: Dados são impressos no console para análise.

### 🛠️ Hardware Utilizado

- Microcontrolador: Raspberry Pi Pico

- Display OLED: SSD1306 (I2C)

- Joystick: Usado para simular valores

- LEDs: Verde e vermelho para indicar status

- Buzzer: Alarmes sonoros

- Matriz de LED WS2812: Exibição de alertas visuais

- Botões: Controle de interface

### 🏗️ Instalação e Uso

#### 1️⃣ Conectar os componentes:

| Componente | Pino no Pico 
| -----------|-------------
| Joystick VRX | GPIO27 (ADC1)
| Joystick VRY | GPIO26 (ADC0)
| Botão A | GPIO5
| Botão B | GPIO6
| Botão Joystick | GPIO22
| Buzzer A | GPIO10 (PWM)
| Buzzer B | GPIO21 (PWM)
| LED Verde | GPIO11
| LED Vermelho | GPIO13
| Matriz WS2812 | GPIO7
| Display SSD1306 SDA | GPIO14
| Display SSD1306 SCL | GPIO15

#### 2️⃣ Configurar o ambiente de desenvolvimento:

- Instale o SDK do Raspberry Pi Pico e configure o ambiente.

- Compile e transfira o código para o Pico usando picotool ou outra ferramenta adequada.

- Conecte um monitor serial (exemplo: minicom, PuTTY ou screen) para visualizar os logs.

#### 3️⃣ Uso do sistema:

- O sistema inicia no modo de monitoramento em tempo real.

- Pressione os botões para alternar entre as telas:

- Botão A: Exibe o gráfico de temperatura.

- Botão B: Exibe o gráfico do nível de água.

- Botão do Joystick: Retorna para a tela principal.

- Modos de simulação:

    Enviar os números 1-5 pelo terminal serial altera entre os modos:

        1 → Controle via joystick

        2 → Simulação de temperatura normal

        3 → Simulação de temperatura crítica

        4 → Simulação de nível de água normal

        5 → Simulação de nível de água crítica

### 🚨 Alertas

Se a temperatura ou nível de água estiverem em valores críticos:

- LED vermelho acende.

- Buzzers emitem som intermitente.

- Matriz WS2812 exibe alerta visual.

- Valores considerados críticos:

    - Temperatura: Abaixo de 220°C ou acima de 355°C

    - Nível de água: Abaixo de 35% ou acima de 85%

### 📹 Vídeo Demonstração

Segue o link do vídeo demonstrando o funcionamento do código na placa BitDogLab.

[Clique aqui para assistir o vídeo!](https://www.youtube.com/watch?v=608l9sRqQ6g)

#### 🎯 Desenvolvido para o projeto final de sistemas embarcados.

