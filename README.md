# Projeto Mesa de LEDs com Animações

Este projeto exibe sequências de animações numa mesa de LEDs (matriz 32x18) controlada por um microcontrolador (por exemplo, Arduino, ESP32) utilizando a biblioteca FastLED.

## Estrutura do Projeto

O projeto está organizado da seguinte forma:

*   **`Animations/`**: Esta pasta contém as fontes das animações.
    *   Cada subpasta dentro de `Animations/` representa uma sequência de animação individual (ex: `NEEC/`, `Bifana/`, `Batata/`, `Menu/`).
    *   As animações originais foram criadas e exportadas usando o **Aseprite**. Cada frame da animação deve ser exportado como um ficheiro `.bmp` individual (ex: `frame_01.bmp`, `frame_02.bmp`, ..., `frame_XX.bmp`) dentro da respetiva subpasta da sequência.
        *   **Dimensões:** Todos os frames BMP devem ter **32x18 píxeis**.
        *   **Nomenclatura:** Os frames devem ser nomeados em ordem alfanumérica para que o script de conversão os processe na sequência correta.

*   **`bmp_to_c_array.py`**: (Localizado na raiz do projeto) Um script Python que converte as sequências de imagens BMP da pasta `Animations/` para um formato de array C (`image_data.h`) que pode ser incluído no código do microcontrolador.

*   **`Arduino/`**:
    *   **`santos_loop/`**: Pasta que contém o sketch Arduino.
        *   **`santos_loop.ino`**: O código Arduino/C++ principal que corre no microcontrolador. Ele lê os dados do `image_data.h` e exibe as animações na mesa de LEDs.
        *   **`image_data.h`**: Este ficheiro é **gerado automaticamente** pelo script Python e **deve ser movido para esta pasta (`Arduino/santos_loop/`)** após a sua geração. Contém os dados de píxeis das animações em arrays `PROGMEM`. **Não edite este ficheiro manualmente**; regenere-o sempre que alterar as animações.
    *   Bibliotecas necessárias (a serem instaladas no Arduino IDE):
        *   `FastLED`
        *   `LEDMatrix` (opcional, mas usada neste exemplo para facilitar o mapeamento de coordenadas)

## Como Usar

1.  **Preparar Animações:**
    *   Crie as suas animações no Aseprite (ou outro software de sua preferência).
    *   Exporte cada frame de cada sequência como um ficheiro `.bmp` individual com dimensões de 32x18 píxeis.
    *   Organize os frames exportados em subpastas dentro de `Animations/` (ex: `Animations/NEEC/frame01.bmp`, `Animations/Bifana/bif_01.bmp`, etc.).

2.  **Converter Animações para Código C:**
    *   Certifique-se de que tem Python instalado e a biblioteca Pillow (`pip install Pillow`).
    *   Execute o script `bmp_to_c_array.py` (que está na raiz do projeto) a partir da linha de comandos, estando na raiz do projeto:
        ```bash
        python bmp_to_c_array.py
        ```
    *   O script irá solicitar o caminho para a pasta principal das animações (deverá introduzir: `Animations` ou `./Animations`).
    *   Ele também irá solicitar o nome do ficheiro de saída. Introduza `image_data.h`. Este ficheiro será gerado na raiz do projeto.
    *   **Mova o `image_data.h` gerado para dentro da pasta `Arduino/santos_loop/`.**

3.  **Carregar Código para o Microcontrolador:**
    *   Abra o sketch `Arduino/santos_loop/santos_loop.ino` no Arduino IDE.
    *   **Verifique as configurações no topo do sketch**: `LED_PIN`, `COLOR_ORDER`, `CHIPSET`, `MATRIX_WIDTH`, `MATRIX_HEIGHT`, `MATRIX_TYPE` devem corresponder à sua mesa de LEDs.
    *   **Verifique os nomes dos arrays de animação no `loop()`**: Devem corresponder aos nomes gerados no `image_data.h` (baseados nos nomes das suas subpastas de animação, ex: `NEEC_anim`, `Bifana_anim`).
    *   Compile e carregue o sketch para o seu microcontrolador.

## Configurações Importantes no Código Arduino

Dentro do ficheiro `santos_loop.ino`, as seguintes definições são cruciais:

*   `#define MATRIX_WIDTH 32`
*   `#define MATRIX_HEIGHT 18`
*   `#define LED_PIN ...`
*   `#define COLOR_ORDER ...`
*   `#define CHIPSET ...`
*   `#define MATRIX_TYPE ...` (Ex: `HORIZONTAL_ZIGZAG_MATRIX`)

Ajuste as constantes de duração das animações (`NEEC_SEQUENCE_FRAME_DURATION_MS`, `OTHER_SEQUENCES_FRAME_DURATION_MS`, etc.) conforme desejado.

## Dependências

*   **Python 3.x**
*   **Pillow** (biblioteca Python para processamento de imagem): `pip install Pillow`
*   **Arduino IDE**
*   **Biblioteca FastLED** (para Arduino)
*   **Biblioteca LEDMatrix** (para Arduino)

---
