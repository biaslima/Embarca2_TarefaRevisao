#include "display.h"

void verificar_borda(int x, int y) {
    if (x == 0 && ultima_borda != BORDA_ESQUERDA) {
        ultima_borda = BORDA_ESQUERDA;
        matriz_exibir_raio();
        som_borda_esquerda();
    } else if (x == WIDTH - TAMANHO_QUADRADO && ultima_borda != BORDA_DIREITA) {
        ultima_borda = BORDA_DIREITA;
        matriz_exibir_raio();
        som_borda_direita();
    } else if (y == 0 && ultima_borda != BORDA_CIMA) {
        ultima_borda = BORDA_CIMA;
        matriz_exibir_raio();
        som_borda_cima();
    } else if (y == HEIGHT - TAMANHO_QUADRADO && ultima_borda != BORDA_BAIXO) {
        ultima_borda = BORDA_BAIXO;
        matriz_exibir_raio();
        som_borda_baixo();
    } else if (
        x > 0 && x < WIDTH - TAMANHO_QUADRADO &&
        y > 0 && y < HEIGHT - TAMANHO_QUADRADO &&
        ultima_borda != BORDA_NENHUMA
    ) {
        ultima_borda = BORDA_NENHUMA;
        matriz_exibir_coracao();
    }
}

// Função pra suavisar o movimento do quadradp
int movimento_suave(int posicao_atual, int posicao_alvo) {
    if (posicao_atual == posicao_alvo) {
        return posicao_atual;
    }
    return posicao_atual + ((posicao_alvo - posicao_atual) / 3); //Move o quadrado de 30 em 30% para a posição alvo
}

// Função pra desenhar borda
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
    //Inversão do eixo Y
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

