#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de pinos
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define SSD1306_ADDRESS 0x3C

#define JOYSTICK_X_PIN 26  
#define JOYSTICK_Y_PIN 27  
#define JOYSTICK_PB 22     
#define BUTTON_A_PIN 5     

#define LED_R_PIN 12  
#define LED_B_PIN 13  
#define LED_G_PIN 11  

// Variáveis globais
volatile bool button_pressed = false;  
volatile bool green_led_state = false; 
volatile bool all_leds_on = false;    
volatile bool border_style = false;   
int x_pos = (128 - 8) / 2;  // Centralização do quadrado no início
int y_pos = (64 - 8) / 2;    // Centralização do quadrado no início

// Função de debouncing
bool debounce() {
    static uint32_t last_press_time = 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_press_time > 200) {  
        last_press_time = current_time;
        return true;
    }
    return false;
}

// Interrupção para o botão do joystick
void joystick_button_isr(uint gpio, uint32_t events) {
    if (debounce()) {
        button_pressed = true;
        green_led_state = !green_led_state;  // Alterna o estado do LED Verde
        gpio_put(LED_G_PIN, green_led_state);  // Atualiza o LED Verde
        border_style = !border_style;  // Alterna a borda do display
    }
}

// Interrupção para o botão A
void button_a_isr(uint gpio, uint32_t events) {
    if (debounce()) {
        all_leds_on = !all_leds_on;  
    }
}

// Inicialização do PWM
uint pwm_init_gpio(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_enabled(slice_num, true);
    return slice_num;
}

// Inicialização do ADC
void init_adc() {
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);
}

// Leitura do ADC
uint16_t read_adc(uint adc_channel) {
    adc_select_input(adc_channel);
    return adc_read();
}

// Controle dos LEDs RGB
void control_leds(uint16_t vrx_value, uint16_t vry_value) {
    if (all_leds_on) {
        // Acende todos os LEDs quando o botão A é pressionado
        pwm_set_gpio_level(LED_R_PIN, 4095);  
        pwm_set_gpio_level(LED_B_PIN, 4095);  
        gpio_put(LED_G_PIN, true);            
    } else {
        // LED Vermelho controlado pelo eixo X
        if (vrx_value > 1500 && vrx_value < 2500) {
            pwm_set_gpio_level(LED_R_PIN, 0);  // Desliga o LED no centro
        } else {
            pwm_set_gpio_level(LED_R_PIN, abs(vrx_value - 2048) * 2);
        }

        // LED Azul controlado pelo eixo Y
        if (vry_value > 1500 && vry_value < 2500) {
            pwm_set_gpio_level(LED_B_PIN, 0);  // Desliga o LED  no centro
        } else {
            pwm_set_gpio_level(LED_B_PIN, abs(vry_value - 2048) * 2);
        }

        // LED Verde controlado pelo botão do joystick
        gpio_put(LED_G_PIN, green_led_state);
    }
}

// Atualiza a borda do display
void update_border(ssd1306_t *ssd) {
    if (border_style) {
        ssd1306_rect(ssd, 0, 0, 128, 64, true, false);  // Borda simples
    } else {
        ssd1306_line(ssd, 0, 0, 128, 0, true);  
        ssd1306_line(ssd, 0, 63, 128, 63, true);  
        ssd1306_line(ssd, 0, 0, 0, 64, true);  
        ssd1306_line(ssd, 127, 0, 127, 64, true);  
    }
}

// Desenha o quadrado no display e o movimenta conforme o joystick
void draw_square(ssd1306_t *ssd, int x, int y) {
    ssd1306_fill(ssd, false);  // Limpa o display
    ssd1306_rect(ssd, x, y, 8, 8, true, true);  // Quadrado preenchido
    update_border(ssd);  // Atualiza a borda
    ssd1306_send_data(ssd);  // Envia os dados para o display
}

// Função principal
int main() {
    stdio_init_all();

    // Inicialização do I2C para o display SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicialização do display SSD1306
    ssd1306_t ssd;
    ssd1306_init(&ssd, 128, 64, false, SSD1306_ADDRESS, I2C_PORT);
    ssd1306_fill(&ssd, false);  
    ssd1306_send_data(&ssd);

    // Inicialização do ADC
    init_adc();

    // Inicialização do PWM para os LEDs
    pwm_init_gpio(LED_R_PIN, 4095);  
    pwm_init_gpio(LED_B_PIN, 4095);  
    pwm_init_gpio(LED_G_PIN, 4095);  

    // Configuração das interrupções para os botões
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &joystick_button_isr);

    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_isr);

    // Loop principal
    while (true) {
        // Leitura dos valores do joystick
        uint16_t vrx_value = read_adc(0);  
        uint16_t vry_value = read_adc(1);  

        // Controle dos LEDs
        control_leds(vrx_value, vry_value);

        // Atualiza a posição do quadrado no display
        x_pos = ((vrx_value * 120) / 4095) + 4;
        y_pos = ((vry_value * 56) / 4095) + 4;        
        draw_square(&ssd, x_pos, y_pos);

        // Debouncing e tratamento de interrupções
        if (button_pressed) {
            button_pressed = false;
        }

        sleep_ms(10);
    }

    return 0;
}
