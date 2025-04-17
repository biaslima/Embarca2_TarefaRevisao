#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h> 
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "lib/matriz_leds.h"
#include "lib/buzzer.h"
#include "lib/display.h"

//ATRIBUIÇÃO DE PINOS
//LEDS
#define LED_PIN_RED 13    
#define LED_PIN_BLUE 12        

//Joystick     
#define JOYSTICK_PIN_X 26      
#define JOYSTICK_PIN_Y 27      
#define JOYSTICK_PIN_BTN 22  
//BUTTON
#define BUTTON_PIN_A 5   
#define BUTTON_PIN_B 6               

// Variáveis globais
static volatile uint32_t last_interrupt_time_joystick = 0;  
static volatile uint32_t last_interrupt_time_A = 0;     
bool leds_ligados = true;       

// Função LED(PWM) - JOYSTICK
uint16_t calcular_brilho_led(uint16_t valor_joystick) {
    // Apagar LED na zona morta
    if (abs(valor_joystick - CENTRO_JOYSTICK) < ZONA_MORTA) 
        return 0;
    
    // Calcula o brilho do LED
    uint32_t diff = valor_joystick > CENTRO_JOYSTICK ? valor_joystick - CENTRO_JOYSTICK : CENTRO_JOYSTICK - valor_joystick;
    uint32_t alcance = valor_joystick > CENTRO_JOYSTICK ? 4095 - CENTRO_JOYSTICK : CENTRO_JOYSTICK;
                      
    return (diff * 255) / alcance; 
}

// Função de interrupção dos botões
static void buttons_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    
    if (gpio == BUTTON_PIN_B && current_time - last_interrupt_time_joystick > 200000) {
        last_interrupt_time_joystick = current_time;
        estilo_borda = !estilo_borda;
        printf("Estilo de borda alterado: %s\n", estilo_borda ? "Pontilhado" : "Contínuo");
        if (leds_ligados) {
            matriz_exibir_padrao(PADRAO_CORACAO_ROSA);
        } else {
            matriz_exibir_padrao(PADRAO_NENHUM);
        }
    }
    
    else if (gpio == BUTTON_PIN_A && current_time - last_interrupt_time_A > 200000) {
        last_interrupt_time_A = current_time;
        leds_ligados = !leds_ligados;
        printf("LEDs %s\n", leds_ligados ? "ligados" : "desligados");
        if (leds_ligados) {
            matriz_exibir_padrao(PADRAO_CORACAO);
        } else {
            matriz_exibir_padrao(PADRAO_NENHUM);
        }
    }
}

void setup() {
    stdio_init_all();
    sleep_ms(2000); 
    
    printf("\n\n=== Inicializando o Projeto ===\n");
    
    // Inicializa I2C 
    i2c_init(i2c1, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);
    printf("I2C inicializado\n");
    
    // Inicializa ADC 
    adc_init();
    adc_gpio_init(JOYSTICK_PIN_X);
    adc_gpio_init(JOYSTICK_PIN_Y);
    printf("ADC inicializado\n");

    // Inicializa o buzzer
    buzzer_init(21); 
    printf("Buzzer inicializado\n");
    
    // Iniciar matriz de LEDs
    printf("Inicializando matriz de LEDs...\n");
    iniciar_matriz_leds(pio0, 0, led_matrix_pin);
    matriz_exibir_padrao(PADRAO_NENHUM);
    
    // Configura os LEDs PWM 
    gpio_set_function(LED_PIN_RED, GPIO_FUNC_PWM);
    gpio_set_function(LED_PIN_BLUE, GPIO_FUNC_PWM);
    
    uint slice_vermelho = pwm_gpio_to_slice_num(LED_PIN_RED);
    uint slice_azul = pwm_gpio_to_slice_num(LED_PIN_BLUE);
    
    pwm_set_wrap(slice_vermelho, 255);
    pwm_set_wrap(slice_azul, 255);
    
    pwm_set_enabled(slice_vermelho, true);
    pwm_set_enabled(slice_azul, true);
    printf("LEDs PWM inicializados\n");
    
    // Inicializa os botões
    gpio_init(BUTTON_PIN_A);
    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(BUTTON_PIN_A);
    gpio_pull_up(BUTTON_PIN_B);
    printf("Botões inicializados\n");
    
    // Configura as interrupções 
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true, buttons_callback);
    gpio_set_irq_enabled(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true);
    printf("Interrupções configuradas\n");
    
    // Inicializa o display
    printf("Inicializando display SSD1306...\n");
    ssd1306_init(&display, WIDTH, HEIGHT, false, ENDERECO_DISPLAY, i2c1);
    ssd1306_config(&display);
    printf("Display inicializado\n");
    
    // Define a posição inicial do quadrado no centro da tela
    posicao_x_atual = posicao_x_alvo = WIDTH / 2 - TAMANHO_QUADRADO / 2;
    posicao_y_atual = posicao_y_alvo = HEIGHT / 2 - TAMANHO_QUADRADO / 2;

    printf("Setup completo!\n");
}

int main() {
    setup();
    sleep_ms(500);  
    
    printf("Iniciando loop principal\n");
    
    // Loop principal
    while (true) {
        desenhar_borda(estilo_borda);
        
        // Lê os valores do joystick
        adc_select_input(0);  
        uint16_t valor_x = adc_read();
        adc_select_input(1);  
        uint16_t valor_y = adc_read();
        
        // Log dos valores do joystick (apenas ocasionalmente para não sobrecarregar)
        static int contador = 0;
        if (++contador >= 100) {
            printf("Joystick: X=%d, Y=%d\n", valor_x, valor_y);
            contador = 0;
        }
        
        // Atualiza as posições alvo do quadrado
        if (abs(valor_y - CENTRO_JOYSTICK) < ZONA_MORTA) {
            posicao_x_alvo = WIDTH / 2 - TAMANHO_QUADRADO / 2;  // Centraliza se joystick parado
        } else {
            posicao_x_alvo = converter_posicao_display(valor_y, WIDTH);
        }

        if (abs(valor_x - CENTRO_JOYSTICK) < ZONA_MORTA) {
            posicao_y_alvo = HEIGHT / 2 - TAMANHO_QUADRADO / 2;  
        } else {
            posicao_y_alvo = converter_posicao_display(valor_x, HEIGHT);
        }
        
        // Move quadrado 
        posicao_x_atual = movimento_suave(posicao_x_atual, posicao_x_alvo);
        posicao_y_atual = movimento_suave(posicao_y_atual, posicao_y_alvo);
        
        // Desenha o quadrado 
        ssd1306_rect(&display, posicao_y_atual, posicao_x_atual, TAMANHO_QUADRADO, TAMANHO_QUADRADO, true, true);
        ssd1306_send_data(&display);
        
        // Verificação de borda - adicione um log para depuração
        printf("Verificando borda: x=%d, y=%d\n", posicao_x_atual, posicao_y_atual);
        verificar_borda(posicao_x_atual, posicao_y_atual);

        // Atualiza os LEDs 
        if (leds_ligados) {
            pwm_set_gpio_level(LED_PIN_RED, calcular_brilho_led(valor_x));
            pwm_set_gpio_level(LED_PIN_BLUE, calcular_brilho_led(valor_y));
        } else {
            pwm_set_gpio_level(LED_PIN_RED, 0);
            pwm_set_gpio_level(LED_PIN_BLUE, 0);
        }
        
        sleep_ms(10);  
    }
}