#include "matriz_leds.h"
#include "ws2812.pio.h"

uint32_t leds[NUM_LEDS];
PIO pio = pio0;
int sm = 0;

// Padrões para a matriz LED 5x5
// 1 = LED aceso, 0 = LED apagado
const uint8_t padrao_coracao[5][5] = {
    {0, 1, 0, 1, 0},
    {1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1},
    {0, 1, 1, 1, 0},
    {0, 0, 1, 0, 0}
};

const uint8_t padrao_raio[5][5] = {
    {0, 0, 1, 0, 0},
    {0, 1, 1, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0},
    {0, 0, 1, 0, 0}
};

void iniciar_matriz_leds(PIO pio, uint sm, uint pin) {
    uint offset = pio_add_program(pio, &ws2812_program);
    printf("Loaded program at %d\n", offset);
    ws2812_program_init(pio, sm, offset, led_matrix_pin, 800000, IS_RGBW);

    for (int i = 0; i < NUM_LEDS; i++) {
     leds[i] = 0; 
    }
    clear_matrix(pio, sm);
    update_leds(pio, sm);
}

//Função para localizar LEDs da matriz através de linhas e colunas
uint8_t matriz_posicao_xy(uint8_t x, uint8_t y) {
    if (y % 2 == 0) {
        // Linhas pares, da esquerda para a direita
        return y * 5 + x;
    } else {
        // Linhas ímpares, da direita para a esquerda
        return y * 5 + (4 - x);
    }
}


// Função para criar uma cor GRB na matriz
uint32_t create_color(uint8_t green, uint8_t red, uint8_t blue) {
    return ((uint32_t)green << 16) | ((uint32_t)red << 8) | blue;
}

// Limpa todos os LEDs
void clear_matrix(PIO pio, uint sm) {
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = 0;
    }
}

void update_leds(PIO pio, uint sm) {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, leds[i] << 8u);
    }
}

void matriz_exibir_coracao() {
    clear_matrix(pio, sm);
    
    // Cor vermelha para o coração
    uint32_t cor_coracao = create_color(150, 0, 0);
    
    for (uint8_t y = 0; y < 5; y++) {
        for (uint8_t x = 0; x < 5; x++) {
            if (padrao_coracao[y][x]) {
                leds[matriz_posicao_xy(x, y)] = cor_coracao;
            }
        }
    }
    
    update_leds(pio, sm);
}

void matriz_exibir_raio() {
    clear_matrix(pio, sm);
    
    // Cor amarela para o raio
    uint32_t cor_raio = create_color(150, 120, 0);
    
    for (uint8_t y = 0; y < 5; y++) {
        for (uint8_t x = 0; x < 5; x++) {
            if (padrao_raio[y][x]) {
                leds[matriz_posicao_xy(x, y)] = cor_raio;
            }
        }
    }
    
    update_leds(pio, sm);
}

void matriz_exibir_padrao(uint8_t padrao) {
    switch (padrao) {
        case PADRAO_CORACAO:
            matriz_exibir_coracao();
            break;
            
        case PADRAO_RAIO:
            matriz_exibir_raio();
            break;
            
        case PADRAO_NENHUM:
        default:
            clear_matrix(pio, sm);
            update_leds(pio, sm);
            break;
    }
}


