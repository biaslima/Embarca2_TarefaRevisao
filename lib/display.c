#include "display.h"
#include "buzzer.h"
#include "matriz_leds.h"

TipoBorda ultima_borda = BORDA_NENHUMA;
bool estilo_borda = false; 
int posicao_x_atual = 0;     
int posicao_y_atual = 0;     
int posicao_x_alvo = 0;      
int posicao_y_alvo = 0;
ssd1306_t display;
static uint32_t ultima_deteccao_borda = 0;

void verificar_borda(int x, int y) {
    uint32_t tempo_atual = to_us_since_boot(get_absolute_time());
    if (tempo_atual - ultima_deteccao_borda < 200000) {
        return;
    }
    // Usar condições exatas para as bordas
    if (x == 0 && ultima_borda != BORDA_ESQUERDA) {
        estilo_borda = !estilo_borda;
        printf("DETECTADO: Tocou na borda esquerda\n");
        ultima_borda = BORDA_ESQUERDA;
        matriz_exibir_padrao(PADRAO_RAIO_1);
        som_borda_esquerda();
    } else if (x == WIDTH - TAMANHO_QUADRADO && ultima_borda != BORDA_DIREITA) {
        estilo_borda = !estilo_borda;
        printf("DETECTADO: Tocou na borda direita\n");
        ultima_borda = BORDA_DIREITA;
        matriz_exibir_padrao(PADRAO_RAIO_2);
        som_borda_direita();
    } else if (y == 0 && ultima_borda != BORDA_CIMA) {
        estilo_borda = !estilo_borda;
        printf("DETECTADO: Tocou na borda superior\n");
        ultima_borda = BORDA_CIMA;
        matriz_exibir_padrao(PADRAO_RAIO_3);
        som_borda_cima();
    } else if (y == HEIGHT - TAMANHO_QUADRADO && ultima_borda != BORDA_BAIXO) {
        estilo_borda = !estilo_borda;
        printf("DETECTADO: Tocou na borda inferior\n");
        ultima_borda = BORDA_BAIXO;
        matriz_exibir_padrao(PADRAO_RAIO_4);
        som_borda_baixo();
    } else if (x > 5 && x < WIDTH - TAMANHO_QUADRADO - 5 && y > 5 && y < HEIGHT - TAMANHO_QUADRADO - 5 && ultima_borda != BORDA_NENHUMA) {
        // Adicionei uma margem maior para retornar ao estado normal
        printf("DETECTADO: Saiu de todas as bordas\n");
        ultima_borda = BORDA_NENHUMA;
        matriz_exibir_padrao(PADRAO_CORACAO);
    }
    if (ultima_borda != BORDA_NENHUMA) {
        ultima_deteccao_borda = tempo_atual;
    }
}


// Função para suavisar o movimento do quadrado
int movimento_suave(int posicao_atual, int posicao_alvo) {
    if (posicao_atual == posicao_alvo) {
        return posicao_atual;
    }
    
    // Se estiver muito próximo do alvo ou se o alvo for uma borda, mova diretamente para ele
    if (abs(posicao_alvo - posicao_atual) <= 2 || 
        posicao_alvo == 0 || 
        posicao_alvo == WIDTH - TAMANHO_QUADRADO || 
        posicao_alvo == HEIGHT - TAMANHO_QUADRADO) {
        return posicao_alvo;
    }
    
    return posicao_atual + ((posicao_alvo - posicao_atual) / 4); // 25% de movimento para cada frame
}

// Função para desenhar borda
void desenhar_borda(bool estilo_pontilhado) {
    ssd1306_fill(&display, false); 
    
    // Desenha as linhas horizontais (em cima e embaixo)
    for (int i = 0; i < WIDTH; i += (estilo_pontilhado ? 4 : 1)) {
        ssd1306_pixel(&display, i, 0, true);          
        ssd1306_pixel(&display, i, HEIGHT - 1, true); 
    }
    
    // Desenha as linhas verticais (esquerda e direita)
    for (int i = 0; i < HEIGHT; i += (estilo_pontilhado ? 4 : 1)) {
        ssd1306_pixel(&display, 0, i, true);         
        ssd1306_pixel(&display, WIDTH - 1, i, true); 
    }
}

// Função que converte a posição do joystick para a posição na tela
int converter_posicao_display(int valor_joystick, int tamanho_tela) {
     // Inversão do eixo Y para corresponder ao comportamento esperado do joystick
     if (tamanho_tela == HEIGHT) {
        valor_joystick = 4095 - valor_joystick;
    }
    
    // Garantir que o valor esteja dentro dos limites do ADC
    if (valor_joystick < 0) valor_joystick = 0;
    if (valor_joystick > 4095) valor_joystick = 4095;
    
    // Ajustar para garantir que valores extremos alcancem as bordas
    if (valor_joystick <= 50) return 0;
    if (valor_joystick >= 4045) return tamanho_tela - TAMANHO_QUADRADO;
    
    // Reduzir um pouco a zona morta para melhorar a resposta
    long alcance_entrada = 4095;
    long alcance_saida = tamanho_tela - TAMANHO_QUADRADO;
    
    // Calcular a posição com escala completa
    int posicao = (valor_joystick * alcance_saida) / alcance_entrada;
    
    return posicao;
}