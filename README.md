# Não toque nas Bordas! – Projeto BitDogLab

Projeto de revisão do módulo de capacitação em Sistemas Embarcados utilizando a placa BitDogLab e o microcontrolador RP2040.

## 🎯 Objetivo
Criar um sistema interativo com joystick, display, matriz de LEDs e buzzer, onde o movimento de um quadrado no display gera efeitos visuais e sonoros.

## 📦 Funcionalidades
- Quadrado inicial centralizado e movimentado com joystick.
- Pressão do botão A exibe um coração vermelho na matriz.
- Pressão do botão B troca a cor do coração.
- Ao encostar o quadrado em qualquer borda, a matriz exibe um novo desenho com cor correspondente, e o buzzer emite um som diferente.
- UART envia logs com os eventos ocorridos.

## 🧩 Periféricos Utilizados
- ADC: Leitura analógica do joystick.
- Botões com interrupção e debounce.
- Display OLED (SSD1306).
- Matriz de LEDs WS2812 (via PIO).
- LED RGB via PWM.
- Buzzer com diferentes tons.
- UART para depuração.

## 🛠️ Requisitos
- VS Code com extensão do Pico SDK
- Pico SDK configurado
- Arquivo `ws2812.pio` incluído no projeto

## 📹 Demonstração
Assista ao vídeo de funcionamento aqui: [YouTube/Drive](https://drive.google.com/file/d/SEU_VIDEO_ID/view)

## 📁 Organização
```
├── CMakeLists.txt
├── pio/
│   └── ws2812.pio               ← Código PIO para a matriz WS2812
├── lib/
│   ├── display.c
│   ├── display.h                ← Controle do display OLED
│   ├── matriz_leds.c
│   ├── matriz_leds.h            ← Funções para desenhos e cores na matriz de LEDs
│   ├── buzzer.c
│   ├── buzzer.h                 ← Funções para os sons personalizados
├── src/
│   └── main.c                   ← Lógica principal (movimento, bordas, interação)
├── README.md
```

## 👩‍💻 Autora
**Anna Beatriz Silva Lima**  
Feira de Santana – EmbarcaTech 2025
