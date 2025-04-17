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
    // Adicionar log detalhado para depuração
    printf("Posição do quadrado: x=%d, y=%d, WIDTH=%d, HEIGHT=%d, TAMANHO=%d\n", 
               x, y, WIDTH, HEIGHT, TAMANHO_QUADRADO);
        
    // Correção das condições de borda
    if (x <= 1 && ultima_borda != BORDA_ESQUERDA) {  // Ajustado para 1 em vez de 0
        printf("DETECTADO: Tocou na borda esquerda\n");
         ultima_borda = BORDA_ESQUERDA;
        matriz_exibir_padrao(PADRAO_RAIO);
        som_borda_esquerda();
    } else if (x >= WIDTH - TAMANHO_QUADRADO - 1 && ultima_borda != BORDA_DIREITA) {  // Ajustado -1
        printf("DETECTADO: Tocou na borda direita\n");
        ultima_borda = BORDA_DIREITA;
        matriz_exibir_padrao(PADRAO_RAIO);
        som_borda_direita();
    } else if (y <= 1 && ultima_borda != BORDA_CIMA) {  // Ajustado para 1 em vez de 0
        printf("DETECTADO: Tocou na borda superior\n");
         ultima_borda = BORDA_CIMA;
        matriz_exibir_padrao(PADRAO_RAIO);
         som_borda_cima();
    } else if (y >= HEIGHT - TAMANHO_QUADRADO - 1 && ultima_borda != BORDA_BAIXO) {  // Ajustado -1
        printf("DETECTADO: Tocou na borda inferior\n");
        ultima_borda = BORDA_BAIXO;
        matriz_exibir_padrao(PADRAO_RAIO);
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
    return posicao_atual + ((posicao_alvo - posicao_atual) / 5); //Move o quadrado de 30 em 30% para a posição alvo
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