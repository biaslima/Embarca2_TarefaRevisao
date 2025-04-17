 #ifndef MATRIZ_LEDS_H
 #define MATRIZ_LEDS_H
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ws2812.pio.h"
#include "hardware/pio.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "ssd1306.h"
#include "hardware/i2c.h"

#define NUM_LEDS 25
#define IS_RGBW false
#define led_matrix_pin 7

extern uint32_t leds[NUM_LEDS];
extern PIO pio;
extern int sm;

// Padrões de exibição
#define PADRAO_NENHUM 0
#define PADRAO_CORACAO 1
#define PADRAO_RAIO 2

// Funções de exibição
void matriz_exibir_coracao();
void matriz_exibir_raio();
void matriz_exibir_padrao(uint8_t padrao);

void iniciar_matriz_leds(PIO pio, uint sm, uint pin);
uint8_t matriz_posicao_xy(uint8_t x, uint8_t y);
uint32_t create_color(uint8_t green, uint8_t red, uint8_t blue);
void clear_matrix(PIO pio, uint sm);
void update_leds(PIO pio, uint sm);


 #endif 