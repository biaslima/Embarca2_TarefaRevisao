#ifndef MATRIZ_LEDS_H
#define MATRIZ_LEDS_H
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"

#define NUM_LEDS 25
#define IS_RGBW false
#define led_matrix_pin 7

// Padrões de exibição
#define PADRAO_NENHUM 0
#define PADRAO_CORACAO 1
#define PADRAO_RAIO_1 2
#define PADRAO_RAIO_2 3
#define PADRAO_RAIO_3 4
#define PADRAO_RAIO_4 5
#define PADRAO_CORACAO_ROSA 6

// Declaração de variáveis externas
extern uint32_t leds[NUM_LEDS];
extern PIO pio;
extern int sm;

// Protótipos de funções
void iniciar_matriz_leds(PIO pio_inst, uint sm_num, uint pin);
uint8_t matriz_posicao_xy(uint8_t x, uint8_t y);
uint32_t create_color(uint8_t green, uint8_t red, uint8_t blue);
void clear_matrix(PIO pio_inst, uint sm_num);
void update_leds(PIO pio_inst, uint sm_num);
void matriz_exibir_coracao(void);
void matriz_exibir_raio(void);
void matriz_exibir_padrao(uint8_t padrao);

#endif