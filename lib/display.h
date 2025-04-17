#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssd1306.h"

typedef enum {
    BORDA_NENHUMA,
    BORDA_ESQUERDA,
    BORDA_DIREITA,
    BORDA_CIMA,
    BORDA_BAIXO
} TipoBorda;

// Variáveis globais 
extern TipoBorda ultima_borda;
extern int posicao_x_atual;     
extern int posicao_y_atual;     
extern int posicao_x_alvo;      
extern int posicao_y_alvo;  
extern bool estilo_borda; 
extern ssd1306_t display;    
extern bool leds_ligados;
extern uint8_t coracao_cor;  

//CONSTANTES IMPORTANTES
#define TAMANHO_QUADRADO 8      
#define ENDERECO_DISPLAY 0x3C   
#define CENTRO_JOYSTICK 2048     
#define ZONA_MORTA 100 

// Protótipos de funções
void verificar_borda(int x, int y);
int movimento_suave(int posicao_atual, int posicao_alvo);
void desenhar_borda(bool estilo_pontilhado);
int converter_posicao_display(int valor_joystick, int tamanho_tela);

#endif