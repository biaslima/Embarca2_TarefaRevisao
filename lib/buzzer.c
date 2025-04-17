#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include <stdio.h>

static int buzzer_pin;
static uint slice_num;
static uint channel;

void buzzer_init(int pin) {
    buzzer_pin = pin;

    gpio_set_function(buzzer_pin, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(buzzer_pin);
    channel = pwm_gpio_to_channel(buzzer_pin);
    
    pwm_set_clkdiv(slice_num, 125.0);  
    pwm_set_wrap(slice_num, 1000);     
    pwm_set_chan_level(slice_num, channel, 500); 
    
    printf("Buzzer inicializado no pino %d (PWM slice %d, canal %d)\n", 
           pin, slice_num, channel);
}


void tocar_frequencia(int frequencia, int duracao_ms) {
    // Fórmula: wrap = 1_000_000 / frequência
    uint32_t wrap = 1000000 / frequencia;
    
    printf("Tocando frequência %d Hz (wrap=%d)\n", frequencia, wrap);
    
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, channel, wrap / 2);
    pwm_set_enabled(slice_num, true);
    
    sleep_ms(duracao_ms);
    
    pwm_set_enabled(slice_num, false);
}

// Funções de efeitos sonoros 
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