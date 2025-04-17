#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int buzzer_pin;

void buzzer_init(int pin) {
    buzzer_pin = pin;
    gpio_init(buzzer_pin);
    gpio_set_dir(buzzer_pin, GPIO_OUT);
    printf("Buzzer inicializado no pino %d\n", pin);
}

void tocar_buzzer(int duracao_ms, int repeticoes, int intervalo_ms) {
    printf("Tocando buzzer: duracao=%d, repeticoes=%d, intervalo=%d\n", 
        duracao_ms, repeticoes, intervalo_ms);
    
    for (int i = 0; i < repeticoes; i++) {
        gpio_put(buzzer_pin, 1);
        sleep_ms(duracao_ms);
        gpio_put(buzzer_pin, 0);
        
        if (i < repeticoes - 1) {
            sleep_ms(intervalo_ms);
        }
    }
}

void som_borda_esquerda() {
    printf("Som borda esquerda\n");
    tocar_buzzer(100, 1, 0);
}

void som_borda_direita() {
    printf("Som borda direita\n");
    tocar_buzzer(80, 2, 100);
}

void som_borda_cima() {
    printf("Som borda cima\n");
    tocar_buzzer(60, 3, 80);
}

void som_borda_baixo() {
    printf("Som borda baixo\n");
    tocar_buzzer(150, 1, 0);
}