#include "buzzer.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

static int buzzer_pin;

void buzzer_init(int pin) {
    buzzer_pin = pin;
    gpio_init(buzzer_pin);
    gpio_set_dir(buzzer_pin, GPIO_OUT);
}

void tocar_buzzer(int duracao_ms, int repeticoes, int intervalo_ms) {
    for (int i = 0; i < repeticoes; i++) {
        gpio_put(buzzer_pin, 1);
        sleep_ms(duracao_ms);
        gpio_put(buzzer_pin, 0);
        sleep_ms(intervalo_ms);
    }
}

void som_borda_esquerda() {
    tocar_buzzer(100, 1, 0);
}

void som_borda_direita() {
    tocar_buzzer(80, 2, 100);
}

void som_borda_cima() {
    tocar_buzzer(60, 3, 80);
}

void som_borda_baixo() {
    tocar_buzzer(150, 1, 0);
}
