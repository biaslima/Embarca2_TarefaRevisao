#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int buzzer_pin;
static uint slice_num;
static uint channel;

void buzzer_init(int pin) {
    buzzer_pin = pin;
    
    // Configurar para PWM
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    channel = pwm_gpio_to_channel(buzzer_pin);
    
    // Configuração inicial do PWM
    pwm_set_clkdiv(slice_num, 125.0);  // Divisor de clock 125MHz/125 = 1MHz
    pwm_set_wrap(slice_num, 1000);     // Frequência de 1kHz
    pwm_set_chan_level(slice_num, channel, 500); // 50% duty cycle
    
    printf("Buzzer inicializado no pino %d (PWM slice %d, canal %d)\n", 
           pin, slice_num, channel);
}

// Método 1: Usando PWM para frequências específicas
void tocar_frequencia(int frequencia, int duracao_ms) {
    // Frequências comuns para buzzer: 500-2000 Hz
    
    // Calcular o valor de wrap para a frequência desejada
    // Fórmula: wrap = 1_000_000 / frequência
    uint32_t wrap = 1000000 / frequencia;
    
    printf("Tocando frequência %d Hz (wrap=%d)\n", frequencia, wrap);
    
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, channel, wrap / 2); // 50% duty cycle
    pwm_set_enabled(slice_num, true);
    
    sleep_ms(duracao_ms);
    
    pwm_set_enabled(slice_num, false);
}

// Método 2: Alternar o pino rapidamente para criar um som
void tocar_buzzer_alternando(int duracao_ms, int repeticoes, int intervalo_ms) {
    // Desabilitar PWM temporariamente
    pwm_set_enabled(slice_num, false);
    
    // Configurar como GPIO para alternar manualmente
    gpio_set_function(buzzer_pin, GPIO_FUNC_SIO);
    gpio_set_dir(buzzer_pin, GPIO_OUT);
    
    printf("Tocando buzzer com alternância: duracao=%d, repeticoes=%d, intervalo=%d\n", 
           duracao_ms, repeticoes, intervalo_ms);
    
    for (int r = 0; r < repeticoes; r++) {
        // Gerar aproximadamente 1kHz manualmente 
        // (ciclo completo em ~1ms = frequência de ~1kHz)
        absolute_time_t end_time = make_timeout_time_ms(duracao_ms);
        while (!time_reached(end_time)) {
            gpio_put(buzzer_pin, 1);
            sleep_us(500); // 0.5ms ligado
            gpio_put(buzzer_pin, 0);  
            sleep_us(500); // 0.5ms desligado
        }
        
        if (r < repeticoes - 1) {
            sleep_ms(intervalo_ms);
        }
    }
    
    // Retornar para modo PWM
    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
}

void som_borda_esquerda() {
    printf("Som borda esquerda\n");
    tocar_frequencia(800, 100);
}

void som_borda_direita() {
    printf("Som borda direita\n");
    tocar_frequencia(1000, 80);
    sleep_ms(100);
    tocar_frequencia(1000, 80);
}

void som_borda_cima() {
    printf("Som borda cima\n");
    tocar_frequencia(1200, 60);
    sleep_ms(80);
    tocar_frequencia(1200, 60);
    sleep_ms(80);
    tocar_frequencia(1200, 60);
}

void som_borda_baixo() {
    printf("Som borda baixo\n");
    tocar_frequencia(600, 150);
}