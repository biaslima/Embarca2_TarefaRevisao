#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definição de pinos
#define LED_PIN_RED 13    
#define LED_PIN_BLUE 12        
#define LED_PIN_GREEN 11

#define JOYSTICK_PIN_X 26      
#define JOYSTICK_PIN_Y 27      
#define JOYSTICK_PIN_BTN 22

#define BUTTON_PIN_A 5 // Inicia jogo
#define BUTTON_PIN_B 6 // Para o jogo 

#define TAMANHO_QUADRADO 8      
#define ENDERECO_DISPLAY 0x3C

#define CENTRO_JOYSTICK 2048    // Valor central do joystick 
#define ZONA_MORTA 100  

#define BUZZER_PIN 21
#define BUZZER_FREQUENCY 100

#define LED_MATRIX_PIN 7
#define NUM_LEDS 25
#define IS_RGBW false

// Constantes do Jogo
#define TEMPO_MAXIMO 5000  // 5 segundos para responder
#define PONTOS_BASE 100 
#define cor_verde 0x00FF00

// Definição de tipos
typedef enum { 
    CIMA,
    BAIXO,
    DIREITA,
    ESQUERDA,
    NENHUMA
} direcao_t;

typedef enum { 
    START,
    DURING,
    END
} game_state_t;

// Variáveis globais
static volatile uint32_t last_interrupt_time_joystick = 0;
static volatile uint32_t last_interrupt_time_B = 0;  
static volatile uint32_t last_interrupt_time_A = 0;  
ssd1306_t display;            
uint32_t leds[NUM_LEDS];

// Variáveis de estado
game_state_t estado_atual = START;
bool som_ativado = true;
uint16_t pontuacao = 0;
uint16_t melhor_pontuacao = 0;
uint8_t nivel_atual = 1;
direcao_t direcao_atual = NENHUMA;
uint32_t tempo_inicio_rodada = 0;
bool acertou_direcao = false;

// Variáveis da posição do quadrado
static int posicao_x_atual;     
static int posicao_y_atual;     
static int posicao_x_alvo;      
static int posicao_y_alvo;    

// Padrões de setas para a matriz de LEDs
uint32_t seta_cima[NUM_LEDS] = {
    0, 0, cor_verde, 0, 0,
    0, cor_verde, cor_verde, cor_verde, 0,
    cor_verde, 0, cor_verde, 0, cor_verde,
    0, 0, cor_verde, 0, 0,
    0, 0, cor_verde, 0, 0
};

uint32_t seta_baixo[NUM_LEDS] = {
    0, 0, cor_verde, 0, 0,
    0, 0, cor_verde, 0, 0,
    cor_verde, 0, cor_verde, 0, cor_verde,
    0, cor_verde, cor_verde, cor_verde, 0,
    0, 0, cor_verde, 0, 0
};

uint32_t seta_esquerda[NUM_LEDS] = {
    0, 0, cor_verde, 0, 0,
    0, cor_verde, 0, 0, 0,
    cor_verde, cor_verde, cor_verde, cor_verde, cor_verde,
    0, cor_verde, 0, 0, 0,
    0, 0, cor_verde, 0, 0
};

uint32_t seta_direita[NUM_LEDS] = {
    0, 0, cor_verde, 0, 0,
    0, 0, 0, cor_verde, 0,
    cor_verde, cor_verde, cor_verde, cor_verde, cor_verde,
    0, 0, 0, cor_verde, 0,
    0, 0, cor_verde, 0, 0
};

// Protótipos de funções
void update_leds(PIO pio, int sm);
void mostrar_seta(direcao_t direcao, PIO pio, int sm);
int movimento_suave(int posicao_atual, int posicao_alvo);
int converter_posicao_display(int valor_joystick, int tamanho_tela);
bool verificar_direcao(direcao_t direcao);
void tocar_som(uint16_t frequencia, uint16_t duracao_ms);
void som_acerto();
void som_erro();
void som_inicio();
void desenhar_menu();
void desenhar_tela_jogo();
void desenhar_game_over();
direcao_t gerar_direcao_aleatoria();
void iniciar_rodada(PIO pio, int sm);
void verificar_resultado(PIO pio, int sm);
static void buttons_callback(uint gpio, uint32_t events);

// Função para atualizar os LEDs
void update_leds(PIO pio, int sm) {
    for (int i = 0; i < NUM_LEDS; i++) {
        pio_sm_put_blocking(pio, sm, leds[i] << 8u);
    }
}

// Função para mostrar a seta na matriz de LEDs
void mostrar_seta(direcao_t direcao, PIO pio, int sm) {
    uint32_t *padrao_seta;
    
    switch (direcao) {
        case CIMA:
            padrao_seta = seta_cima;
            break;
        case BAIXO:
            padrao_seta = seta_baixo;
            break;
        case ESQUERDA:
            padrao_seta = seta_esquerda;
            break;
        case DIREITA:
            padrao_seta = seta_direita;
            break;
        default:
            // Apaga todos os LEDs
            for (int i = 0; i < NUM_LEDS; i++) {
                leds[i] = 0;
            }
            update_leds(pio, sm);
            return;
    }
    
    // Copia o padrão para o buffer de LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = padrao_seta[i];
    }
    
    // Atualiza a matriz de LEDs
    update_leds(pio, sm);
}

// Função para suavizar o movimento do quadrado
int movimento_suave(int posicao_atual, int posicao_alvo) {
    if (posicao_atual == posicao_alvo) {
        return posicao_atual;
    }
    return posicao_atual + ((posicao_alvo - posicao_atual) / 3); // Move o quadrado de 30 em 30% para a posição alvo
}

// Função que converte a posição do joystick para a posição na tela
int converter_posicao_display(int valor_joystick, int tamanho_tela) {
    // Inversão do eixo Y
    if (tamanho_tela == HEIGHT) {
        valor_joystick = 4095 - valor_joystick;
    }
    
    int posicao_centro = tamanho_tela / 2 - TAMANHO_QUADRADO / 2;
    int alcance = (tamanho_tela - TAMANHO_QUADRADO) / 2;
    
    int posicao = posicao_centro + ((long)(valor_joystick - CENTRO_JOYSTICK) * alcance) / CENTRO_JOYSTICK;
    
    // Garante que o quadrado só se mova na tela
    if (posicao < 0) 
        return 0;
    if (posicao > tamanho_tela - TAMANHO_QUADRADO) 
        return tamanho_tela - TAMANHO_QUADRADO;
    return posicao;
}

// Verifica se o quadrado está na direção correta
bool verificar_direcao(direcao_t direcao) {
    const int MARGEM = 10;
    
    switch (direcao) {
        case CIMA:
            return posicao_y_atual < MARGEM;
        case BAIXO:
            return posicao_y_atual > HEIGHT - TAMANHO_QUADRADO - MARGEM;
        case ESQUERDA:
            return posicao_x_atual < MARGEM;
        case DIREITA:
            return posicao_x_atual > WIDTH - TAMANHO_QUADRADO - MARGEM;
        default:
            return false;
    }
}

// Funções para controle do buzzer
void tocar_som(uint16_t frequencia, uint16_t duracao_ms) {
    gpio_set_function(BUZZER_PIN, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(BUZZER_PIN);
    pwm_set_wrap(slice, 125000000 / frequencia - 1); // Calcula wrap para a frequência
    pwm_set_chan_level(slice, PWM_CHAN_A, 50); // Volume médio
    pwm_set_enabled(slice, true);
    sleep_ms(duracao_ms);
    pwm_set_enabled(slice, false); // Desliga
}

void som_acerto() {
    tocar_som(1200, 120); // Tom agudo e curto
}

void som_erro() {
    tocar_som(300, 300); // Tom grave e longo
}

void som_inicio() {
    tocar_som(700, 100);
    sleep_ms(50);
    tocar_som(900, 100);
}

// Funções para desenhar as telas do jogo
void desenhar_menu() {
    ssd1306_fill(&display, false);
    ssd1306_draw_string(&display, "BitDog Challenge", 10, 5);
    ssd1306_draw_string(&display, "Mova para a", 20, 25);
    ssd1306_draw_string(&display, "direcao da seta!", 15, 35);
    ssd1306_draw_string(&display, "Pressione o botao", 5, 50);
    ssd1306_draw_string(&display, "para iniciar", 20, 58);
    ssd1306_send_data(&display);
}

void desenhar_tela_jogo() {
    ssd1306_fill(&display, false);
    
    // Desenha o quadrado na posição atual
    ssd1306_rect(&display, posicao_y_atual, posicao_x_atual, TAMANHO_QUADRADO, TAMANHO_QUADRADO, true, true);
    
    // Exibe a pontuação
    char str_pontuacao[20];
    sprintf(str_pontuacao, "Pontos: %d", pontuacao);
    ssd1306_draw_string(&display, str_pontuacao, 5, 5);
    
    // Exibe o nível
    char str_nivel[15];
    sprintf(str_nivel, "Nivel: %d", nivel_atual);
    ssd1306_draw_string(&display, str_nivel, 80, 5);
    
    // Se estiver aguardando resposta, exibe o tempo restante
    if (direcao_atual != NENHUMA) {
        uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
        uint32_t tempo_passado = tempo_atual - tempo_inicio_rodada;
        uint32_t tempo_restante = (tempo_passado < TEMPO_MAXIMO) ? (TEMPO_MAXIMO - tempo_passado) / 1000 : 0;
        
        char str_tempo[15];
        sprintf(str_tempo, "Tempo: %d", (int)tempo_restante);
        ssd1306_draw_string(&display, str_tempo, 40, 58);
    }
    
    ssd1306_send_data(&display);
}

void desenhar_game_over() {
    ssd1306_fill(&display, false);
    ssd1306_draw_string(&display, "GAME OVER", 30, 10);
    
    char str_pontuacao[25];
    sprintf(str_pontuacao, "Pontuacao: %d", pontuacao);
    ssd1306_draw_string(&display, str_pontuacao, 20, 30);
    
    char str_melhor[25];
    sprintf(str_melhor, "Recorde: %d", melhor_pontuacao);
    ssd1306_draw_string(&display, str_melhor, 20, 40);
    
    ssd1306_draw_string(&display, "Pressione o botao", 5, 55);
    ssd1306_send_data(&display);
}

// Função para gerar uma direção aleatória
direcao_t gerar_direcao_aleatoria() {
    return (direcao_t)(rand() % 4); // Retorna um valor entre 0 e 3 (CIMA, BAIXO, DIREITA, ESQUERDA)
}

// Função para iniciar uma nova rodada
void iniciar_rodada(PIO pio, int sm) {
    direcao_atual = gerar_direcao_aleatoria();
    mostrar_seta(direcao_atual, pio, sm);
    tempo_inicio_rodada = to_ms_since_boot(get_absolute_time());
    acertou_direcao = false;
    
    // Centraliza o quadrado
    posicao_x_alvo = WIDTH / 2 - TAMANHO_QUADRADO / 2;
    posicao_y_alvo = HEIGHT / 2 - TAMANHO_QUADRADO / 2;
    
    // Envia mensagem de debug via UART
    printf("Nova rodada iniciada. Direcao: %d\n", direcao_atual);
}

// Função para verificar o resultado da rodada
void verificar_resultado(PIO pio, int sm) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
    uint32_t tempo_passado = tempo_atual - tempo_inicio_rodada;
    
    if (verificar_direcao(direcao_atual)) {
        // Acertou a direção
        acertou_direcao = true;
        
        // Calcula pontos baseado no tempo de resposta
        uint16_t pontos = PONTOS_BASE;
        if (tempo_passado < TEMPO_MAXIMO) {
            pontos += (TEMPO_MAXIMO - tempo_passado) * 20 / 1000; // 20 pontos extras por segundo restante
        }
        
        pontuacao += pontos;
        
        // Acende o LED verde
        gpio_put(LED_PIN_RED, 0);
        gpio_put(LED_PIN_GREEN, 1);
        gpio_put(LED_PIN_BLUE, 0);
        
        // Toca som de acerto
        som_acerto();
        
        // Envia mensagem de debug via UART
        printf("Acertou! Pontos: %d, Tempo: %d ms\n", pontos, tempo_passado);
        
        // Inicia nova rodada após um breve delay
        sleep_ms(500);
        gpio_put(LED_PIN_GREEN, 0);
        
        // Aumenta o nível a cada 5 acertos
        if (pontuacao / PONTOS_BASE % 5 == 0) {
            nivel_atual++;
            printf("Novo nivel: %d\n", nivel_atual);
        }
        
        iniciar_rodada(pio, sm);
    } else if (tempo_passado >= TEMPO_MAXIMO) {
        // Tempo esgotado
        
        // Acende o LED vermelho
        gpio_put(LED_PIN_RED, 1);
        gpio_put(LED_PIN_GREEN, 0);
        gpio_put(LED_PIN_BLUE, 0);
        
        // Toca som de erro
        som_erro();
        
        // Envia mensagem de debug via UART
        printf("Tempo esgotado! Game Over. Pontuacao final: %d\n", pontuacao);
        
        // Atualiza melhor pontuação se necessário
        if (pontuacao > melhor_pontuacao) {
            melhor_pontuacao = pontuacao;
            printf("Novo recorde: %d\n", melhor_pontuacao);
        }
        
        // Muda para o estado de game over
        estado_atual = END;
        sleep_ms(500);
        gpio_put(LED_PIN_RED, 0);
    }
}

// Função de interrupção dos botões
static void buttons_callback(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Callback do botão A (iniciar/reiniciar)
    if (gpio == BUTTON_PIN_A && current_time - last_interrupt_time_A > 200000) {
        last_interrupt_time_A = current_time;
        
        printf("Botao A pressionado\n");
        
        switch (estado_atual) {
            case START:
                estado_atual = DURING;
                pontuacao = 0;
                nivel_atual = 1;
                som_inicio();
                iniciar_rodada(pio0, 0); // Usando pio0, sm 0
                break;
                
            case END:
                estado_atual = START;
                break;
                
            default:
                break;
        }
    }
    // Callback do botão B (parar/verificar)
    else if (gpio == BUTTON_PIN_B && current_time - last_interrupt_time_B > 200000) {
        last_interrupt_time_B = current_time;
        
        printf("Botao B pressionado\n");
        
        if (estado_atual == DURING) {
            verificar_resultado(pio0, 0); // Usando pio0, sm 0
        }
    }
    // Callback do botão do joystick
    else if (gpio == JOYSTICK_PIN_BTN && current_time - last_interrupt_time_joystick > 200000) {
        last_interrupt_time_joystick = current_time;
        printf("Botao do joystick pressionado\n");
        
        // Ação específica para o botão do joystick (pode ser personalizada)
        if (estado_atual == DURING) {
            som_ativado = !som_ativado;
            printf("Som %s\n", som_ativado ? "ativado" : "desativado");
        }
    }
}

// Função de configuração
void setup() {
    stdio_init_all();

    // Inicializa I2C 
    i2c_init(i2c1, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    // Inicializa ADC 
    adc_init();
    adc_gpio_init(JOYSTICK_PIN_X);
    adc_gpio_init(JOYSTICK_PIN_Y);

    // Inicializa os LEDs
    gpio_init(LED_PIN_GREEN);
    gpio_set_dir(LED_PIN_GREEN, GPIO_OUT);
    gpio_put(LED_PIN_GREEN, 0);
    gpio_init(LED_PIN_BLUE);
    gpio_set_dir(LED_PIN_BLUE, GPIO_OUT);
    gpio_put(LED_PIN_BLUE, 0);
    gpio_init(LED_PIN_RED);
    gpio_set_dir(LED_PIN_RED, GPIO_OUT);
    gpio_put(LED_PIN_RED, 0);

    // Inicializa os botões
    gpio_init(JOYSTICK_PIN_BTN);
    gpio_init(BUTTON_PIN_A);
    gpio_init(BUTTON_PIN_B);
    gpio_set_dir(JOYSTICK_PIN_BTN, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_A, GPIO_IN);
    gpio_set_dir(BUTTON_PIN_B, GPIO_IN);
    gpio_pull_up(JOYSTICK_PIN_BTN);
    gpio_pull_up(BUTTON_PIN_A);
    gpio_pull_up(BUTTON_PIN_B);

    // Configurações PIO
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    printf("Loaded program at %d\n", offset);
 
    ws2812_program_init(pio, sm, offset, LED_MATRIX_PIN, 800000, IS_RGBW);
 
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = 0; 
    }
    update_leds(pio, sm);

    // Configura as interrupções 
    gpio_set_irq_enabled_with_callback(BUTTON_PIN_A, GPIO_IRQ_EDGE_FALL, true, buttons_callback);
    gpio_set_irq_enabled(BUTTON_PIN_B, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(JOYSTICK_PIN_BTN, GPIO_IRQ_EDGE_FALL, true);
    
    // Inicializa o display
    ssd1306_init(&display, WIDTH, HEIGHT, false, ENDERECO_DISPLAY, i2c1);
    ssd1306_config(&display);
// Define a posição inicial do quadrado no centro da tela
posicao_x_atual = posicao_x_alvo = WIDTH / 2 - TAMANHO_QUADRADO / 2;
posicao_y_atual = posicao_y_alvo = HEIGHT / 2 - TAMANHO_QUADRADO / 2;
}

// Função para mostrar uma seta na matriz de LEDs
void mostrar_seta(direcao_t direcao, PIO pio, int sm) {
    // Define a cor verde para os LEDs
    uint32_t cor_verde = 0x00FF00;
    
    // Limpa todos os LEDs primeiro
    for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = 0;
    }
    
    // Seleciona o padrão de seta baseado na direção
    switch (direcao) {
        case CIMA:
            // Padrão de seta para cima (5x5 matriz)
            leds[2] = cor_verde;        // Centro superior
            leds[6] = cor_verde; leds[7] = cor_verde; leds[8] = cor_verde;  // Linha 2
            leds[10] = cor_verde; leds[12] = cor_verde; leds[14] = cor_verde; // Linha 3
            leds[17] = cor_verde;       // Centro linha 4
            leds[22] = cor_verde;       // Centro linha 5
            break;
            
        case BAIXO:
            // Padrão de seta para baixo (5x5 matriz)
            leds[2] = cor_verde;        // Centro superior
            leds[7] = cor_verde;        // Centro linha 2
            leds[10] = cor_verde; leds[12] = cor_verde; leds[14] = cor_verde; // Linha 3
            leds[16] = cor_verde; leds[17] = cor_verde; leds[18] = cor_verde; // Linha 4
            leds[22] = cor_verde;       // Centro linha 5
            break;
            
        case ESQUERDA:
            // Padrão de seta para esquerda (5x5 matriz)
            leds[2] = cor_verde;        // Centro superior
            leds[6] = cor_verde;        // Esquerda linha 2
            leds[10] = cor_verde; leds[11] = cor_verde; leds[12] = cor_verde; leds[13] = cor_verde; leds[14] = cor_verde; // Linha completa
            leds[16] = cor_verde;       // Esquerda linha 4
            leds[22] = cor_verde;       // Centro linha 5
            break;
            
        case DIREITA:
            // Padrão de seta para direita (5x5 matriz)
            leds[2] = cor_verde;        // Centro superior
            leds[8] = cor_verde;        // Direita linha 2
            leds[10] = cor_verde; leds[11] = cor_verde; leds[12] = cor_verde; leds[13] = cor_verde; leds[14] = cor_verde; // Linha completa
            leds[18] = cor_verde;       // Direita linha 4
            leds[22] = cor_verde;       // Centro linha 5
            break;
            
        default:
            // Não mostra nada para NENHUMA direção
            break;
    }
    
    // Atualiza a matriz de LEDs
    update_leds(pio, sm);
}

int main() {
    setup();
    sleep_ms(100); // Pequena pausa para estabilização
    
    // Inicializa o estado do jogo
    estado_atual = START;
    desenhar_menu();
    
    // Inicializa o gerador de números aleatórios
    srand(to_ms_since_boot(get_absolute_time()));
    
    // Loop principal
    while (true) {
        switch (estado_atual) {
            case START:
                // Mostra a tela de menu
                desenhar_menu();
                break;
                
            case DURING:
                // Lê os valores do joystick
                adc_select_input(0); // Canal X
                uint16_t valor_x = adc_read();
                adc_select_input(1); // Canal Y
                uint16_t valor_y = adc_read();
                
                // Atualiza as posições alvo do quadrado
                if (abs(valor_x - CENTRO_JOYSTICK) < ZONA_MORTA) {
                    posicao_x_alvo = WIDTH / 2 - TAMANHO_QUADRADO / 2; // Centraliza se joystick parado
                } else {
                    posicao_x_alvo = converter_posicao_display(valor_x, WIDTH);
                }
                
                if (abs(valor_y - CENTRO_JOYSTICK) < ZONA_MORTA) {
                    posicao_y_alvo = HEIGHT / 2 - TAMANHO_QUADRADO / 2; // Centraliza se joystick parado
                } else {
                    posicao_y_alvo = converter_posicao_display(valor_y, HEIGHT);
                }
                
                // Move quadrado suavemente
                posicao_x_atual = movimento_suave(posicao_x_atual, posicao_x_alvo);
                posicao_y_atual = movimento_suave(posicao_y_atual, posicao_y_alvo);
                
                // Desenha a tela do jogo
                desenhar_tela_jogo();
                
                // Verifica se o tempo acabou
                if (direcao_atual != NENHUMA) {
                    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());
                    if (tempo_atual - tempo_inicio_rodada >= TEMPO_MAXIMO) {
                        verificar_resultado(pio0, 0); // Usando pio0, sm 0
                    }
                    
                    // Verifica se o jogador acertou a direção
                    if (verificar_direcao(direcao_atual) && !acertou_direcao) {
                        verificar_resultado(pio0, 0); // Usando pio0, sm 0
                    }
                }
                break;
                
            case END:
                // Mostra a tela de game over
                desenhar_game_over();
                break;
        }
        
        // Pequena pausa para estabilização
        sleep_ms(10);
    }
    
    return 0;
}
