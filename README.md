# Projeto Joystick com LEDs RGB e Display SSD1306

Este projeto utiliza um joystick para controlar a intensidade de LEDs RGB e exibir a posição do joystick em um display SSD1306. O sistema também inclui funcionalidades como alternar o estado dos LEDs e a borda do display por meio de botões.

# Funcionalidades

1. **Controle de LEDs RGB:**

    - O LED Vermelho é controlado pelo eixo X do joystick.

    - O LED Azul é controlado pelo eixo Y do joystick.

    - O LED Verde é alternado pelo botão do joystick.

2. **Display SSD1306:**

    - Um quadrado de 8x8 pixels é exibido no display, movendo-se conforme a posição do joystick.

    - A borda do display alterna entre dois estilos ao pressionar o botão do joystick.

3. **Botões:**

    - Botão A: Liga/desliga todos os LEDs.

    - Botão do Joystick: Alterna o estado do LED Verde e muda o estilo da borda do display.

# Como utilizar 

1. **Clone o repositório:**
```
git clone <https://github.com/Amanda-Fajardo/EmbarcaTech-ProjetoU4C8>
```

2. Tenha o VSCode instalado e configurado.
   
3. Tenha a extensão do Raspberry Pi Pico instalada e configurada.
   
4. Compile o programa utilizando o CMake:

```
mkdir build
cd build
cmake ..
make
```
5. Carregue o firmware no seu microcontrolador Raspberry Pi Pico para ver o funcionamento do programa.