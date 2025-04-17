#ifndef DISPLAY_H
#define DISPLAY_H

typedef enum {
    BORDA_NENHUMA,
    BORDA_ESQUERDA,
    BORDA_DIREITA,
    BORDA_CIMA,
    BORDA_BAIXO
} TipoBorda;

TipoBorda ultima_borda = BORDA_NENHUMA;

void verificar_borda(int x, int y);
int movimento_suave(int posicao_atual, int posicao_alvo);
void desenhar_borda(bool estilo_pontilhado);
int converter_posicao_display(int valor_joystick, int tamanho_tela);

// Variáveis da posição do quadrado
extern int posicao_x_atual;     
extern int posicao_y_atual;     
extern int posicao_x_alvo;      
extern int posicao_y_alvo;  
extern estilo_borda = false; 
ssd1306_t display;             

#endif