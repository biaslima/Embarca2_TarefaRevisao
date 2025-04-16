// Padrões de setas para a matriz de LEDs (8x8)
// Cada byte representa uma linha, com bits definindo quais LEDs acendem
const uint8_t seta_cima[8] = {
    0b00011000,
    0b00111100,
    0b01111110,
    0b11011011,
    0b10011001,
    0b00011000,
    0b00011000,
    0b00000000
};

const uint8_t seta_baixo[8] = {
    0b00000000,
    0b00011000,
    0b00011000,
    0b10011001,
    0b11011011,
    0b01111110,
    0b00111100,
    0b00011000
};

const uint8_t seta_esquerda[8] = {
    0b00010000,
    0b00110000,
    0b01110000,
    0b11111111,
    0b11111111,
    0b01110000,
    0b00110000,
    0b00010000
};

const uint8_t seta_direita[8] = {
    0b00001000,
    0b00001100,
    0b00001110,
    0b11111111,
    0b11111111,
    0b00001110,
    0b00001100,
    0b00001000
};
