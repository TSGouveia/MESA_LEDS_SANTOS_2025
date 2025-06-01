#include <FastLED.h>
#include <LEDMatrix.h>

// --- Configurações da Matriz ---
#define LED_PIN        10         // Pino de dados onde a matriz está ligada
#define COLOR_ORDER    RGB        // Ordem das cores (RGB, GRB, BRG, etc.)
#define CHIPSET        WS2812B    // Tipo de LED (WS2811, WS2812B, APA102, etc.)

#define MATRIX_WIDTH   32         // Largura da matriz (DEVE CORRESPONDER AO SCRIPT PYTHON)
#define MATRIX_HEIGHT  18         // Altura da matriz (DEVE CORRESPONDER AO SCRIPT PYTHON)
#define NUM_PIXELS     (MATRIX_WIDTH * MATRIX_HEIGHT) // Total de LEDs

// Tipo de layout da matriz - VERIFIQUE SE ESTÁ CORRETO PARA A SUA FIAÇÃO!
// Exemplos: HORIZONTAL_ZIGZAG_MATRIX, HORIZONTAL_MATRIX, VERTICAL_ZIGZAG_MATRIX, etc.
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX

// --- Configurações de Exibição ---
#define BRIGHTNESS     96         // Brilho (0-255) - Ajuste conforme necessário

// --- Configurações Específicas das Animações ---
// Estas são as durações que pretende para cada tipo de animação.
// Os nomes (_anim, _ANIM_NUM_FRAMES) dos arrays e defines virão do image_data.h

// Para a sequência "NEEC" (assumindo que tem uma pasta chamada "NEEC")
#define NEEC_SEQUENCE_FRAME_DURATION_MS  900 // Duração de cada frame da sequência NEEC
#define NEEC_SEQUENCE_LOOP_COUNT         4   // Quantas vezes a sequência NEEC (todos os seus frames) repete

// Para outras sequências como "Bifana", "Batata", "Menu"
// (assumindo que as suas pastas se chamam "Bifana", "Batata", "Menu" ou similar)
#define OTHER_SEQUENCES_FRAME_DURATION_MS 100   // Duração de cada frame (exceto o último)
#define OTHER_SEQUENCES_LAST_FRAME_HOLD_MS 2000 // Duração que o último frame da sequência fica visível

// Inclui os dados da imagem gerados pelo script Python.
// Este ficheiro DEVE estar na mesma pasta que o seu .ino.
// Ele define os arrays NOME_DA_PASTA_anim e as defines NOME_DA_PASTA_ANIM_NUM_FRAMES,
// bem como NUM_PIXELS_GENERATED.
#include "image_data.h"

// --- Objeto da Matriz ---
// Usa o cLEDMatrix para mapear coordenadas (x,y) para o índice linear do LED.
cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

// --- Funções Auxiliares ---

/**
 * @brief Exibe um único frame na matriz de LEDs.
 * O frame é lido da memória PROGMEM.
 * @param frame_data_progmem Ponteiro para o array de píxeis do frame em PROGMEM.
 */
void displayProgmemFrame(const CRGB frame_data_progmem[NUM_PIXELS_GENERATED]) {
  CRGB pixel_color; // Variável temporária para armazenar a cor do píxel lida da PROGMEM

  // Itera por todas as coordenadas (x,y) da matriz
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      // Calcula o índice linear do píxel no array de dados do frame.
      // O script Python gera os píxeis por ordem de varrimento (linha por linha, da esquerda para a direita).
      int pixel_index = (y * MATRIX_WIDTH) + x;

      // Copia o píxel da PROGMEM para a variável temporária pixel_color.
      // memcpy_P é usado para ler da memória de programa (Flash).
      memcpy_P(&pixel_color, &frame_data_progmem[pixel_index], sizeof(CRGB));

      // Define a cor do LED na coordenada (x,y) usando o objeto cLEDMatrix.
      // cLEDMatrix trata do mapeamento correto para a fiação física da sua matriz.
      leds(x, y) = pixel_color;
    }
  }
  FastLED.show(); // Envia os dados atualizados para todos os LEDs da matriz
}

/**
 * @brief Reproduz uma sequência de animação completa.
 * @param animation_array_progmem O array 2D da animação em PROGMEM (ex: NEEC_anim).
 * @param num_frames_in_sequence O número total de frames na sequência (ex: NEEC_ANIM_NUM_FRAMES).
 * @param frame_duration_ms Duração de cada frame em milissegundos.
 * @param last_frame_hold_ms Duração extra para o último frame da sequência (0 se não desejar).
 * @param repeat_count Quantas vezes repetir toda a sequência (1 para reproduzir uma vez).
 */
void playAnimationSequence(
    const CRGB animation_array_progmem[][NUM_PIXELS_GENERATED], // Array 2D de frames
    int num_frames_in_sequence,
    unsigned long frame_duration_ms,
    unsigned long last_frame_hold_ms,
    int repeat_count) {

  if (num_frames_in_sequence == 0) return; // Não faz nada se não houver frames

  for (int r = 0; r < repeat_count; r++) { // Loop para repetições da sequência inteira
    for (int frame_idx = 0; frame_idx < num_frames_in_sequence; frame_idx++) { // Loop por cada frame
      // animation_array_progmem[frame_idx] é um ponteiro para o início do array de píxeis do frame atual
      displayProgmemFrame(animation_array_progmem[frame_idx]);

      // Aplica o atraso apropriado
      if (frame_idx == num_frames_in_sequence - 1 && last_frame_hold_ms > 0) {
        // Se for o último frame da sequência E um "hold" foi especificado
        delay(last_frame_hold_ms);
      } else {
        delay(frame_duration_ms);
      }
    }
  }
}

// --- Setup e Loop ---
void setup() {
  delay(1000); // Pequena pausa para estabilização ao iniciar

  // Inicializa FastLED
  // leds[0] é usado com cLEDMatrix para obter o ponteiro para o início do buffer de LEDs
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], NUM_PIXELS)
         .setCorrection(TypicalLEDStrip); // Opcional: correção de cor
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.clear(true); // Começa com a matriz apagada (preto) e atualiza os LEDs

  // Serial.begin(115200); // Descomente para mensagens de debug via Monitor Série
}

void loop() {
  // --- Reproduzir Animação NEEC ---
  // Verifique os nomes exatos (NEEC_anim, NEEC_ANIM_NUM_FRAMES) no seu image_data.h!
  // Se a pasta se chamou "NEEC", o script gerou NEEC_anim e NEEC_ANIM_NUM_FRAMES.
  #ifdef NEEC_ANIM_NUM_FRAMES // Verifica se a animação NEEC foi definida no image_data.h
    playAnimationSequence(
        NEEC_anim,                          // O array de frames da animação NEEC
        NEEC_ANIM_NUM_FRAMES,               // Número de frames na animação NEEC (do image_data.h)
        NEEC_SEQUENCE_FRAME_DURATION_MS,    // Duração de cada frame NEEC
        0,                                  // Sem "hold" extra no último frame DENTRO do loop de repetição NEEC
        NEEC_SEQUENCE_LOOP_COUNT            // Quantas vezes repetir a sequência NEEC completa
    );
  #endif

  // --- Reproduzir Animação Bifana ---
  // Se a sua pasta se chamou "Bifana", usar Bifana_anim e BIFANA_ANIM_NUM_FRAMES
  #ifdef BIFANA_ANIM_NUM_FRAMES
    playAnimationSequence(
        Bifana_anim,                        // Nome do array conforme gerado pelo script
        BIFANA_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS, // "Hold" no último frame da sequência
        1                                   // Reproduzir a sequência uma vez
    );
  #endif

  // --- Reproduzir Animação Batata ---
  // Se a sua pasta se chamou "Batata", usar Batata_anim e BATATA_ANIM_NUM_FRAMES
  #ifdef BATATA_ANIM_NUM_FRAMES
    playAnimationSequence(
        Batata_anim,
        BATATA_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS,
        1
    );
  #else
    // Se BATATA_ANIM_NUM_FRAMES não estiver definido (ex: pasta "Batata" não encontrada pelo script Python),
    // esta secção não será compilada, evitando erros.
    // Serial.println("Aviso: Animação Batata não definida no image_data.h");
  #endif

  // --- Reproduzir Animação Menu ---
  // Se a sua pasta se chamou "Menu", usar Menu_anim e MENU_ANIM_NUM_FRAMES
  #ifdef MENU_ANIM_NUM_FRAMES
    playAnimationSequence(
        Menu_anim,
        MENU_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS,
        1
    );
  #endif

  // O loop() recomeçará, reproduzindo todas as sequências novamente.
  // delay(1000); // Pausa opcional entre os ciclos completos de todas as animações
}