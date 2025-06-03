#include <FastLED.h>
#include <LEDMatrix.h>

// --- Configurações da Matriz ---
#define LED_PIN        10
#define COLOR_ORDER    RGB // Mantido conforme o seu código base
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  18
#define NUM_PIXELS     (MATRIX_WIDTH * MATRIX_HEIGHT)

// IMPORTANTE: Usar o MATRIX_TYPE do código que FUNCIONA para a correção
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX // <<< Alterado para corresponder ao seu código funcional

// --- Configurações de Exibição ---
#define BRIGHTNESS     255

// --- Configurações Específicas das Animações ---
#define NEEC_SEQUENCE_FRAME_DURATION_MS  900
#define NEEC_SEQUENCE_LOOP_COUNT         4

#define OTHER_SEQUENCES_FRAME_DURATION_MS 100
#define OTHER_SEQUENCES_LAST_FRAME_HOLD_MS 2000

#include "image_data.h" // Contém os arrays _anim e as defines _ANIM_NUM_FRAMES

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

// --- Funções Auxiliares ---

/**
 * @brief Exibe um único frame na matriz de LEDs.
 * O frame é lido da memória PROGMEM e a correção de cor R<->G é aplicada
 * aos 13 LEDs do canto (conforme a lógica de correção que funcionou).
 * @param frame_data_progmem Ponteiro para o array de píxeis do frame em PROGMEM.
 */
void displayProgmemFrame(const CRGB frame_data_progmem[NUM_PIXELS_GENERATED]) {
  CRGB pixel_color_temp;

  // 1. Carrega os dados do PROGMEM para o buffer lógico 'leds'
  for (int y_img = 0; y_img < MATRIX_HEIGHT; y_img++) {
    for (int x_img = 0; x_img < MATRIX_WIDTH; x_img++) {
      // O script Python gera os dados em ordem de varrimento (scanline)
      // (0,0) (1,0) ... (W-1,0)
      // (0,1) (1,1) ... (W-1,1)
      // ...
      // Portanto, y_img e x_img correspondem às coordenadas da imagem.
      int pixel_index_in_image_array = (y_img * MATRIX_WIDTH) + x_img;
      
      memcpy_P(&pixel_color_temp, &frame_data_progmem[pixel_index_in_image_array], sizeof(CRGB));
      
      // leds(x_visual, y_visual) = cor_da_imagem
      // Aqui, x_img e y_img são as coordenadas visuais se a imagem foi desenhada
      // da mesma forma que a matriz é percebida.
      leds(x_img, y_img) = pixel_color_temp;
    }
  }

  // 2. Aplica a SUA CORREÇÃO ESPECÍFICA (que funcionou no código da Unity)
  // Esta correção opera sobre os valores JÁ no buffer 'leds(x,y)'
  int correction_y_visual = 0; // Coordenada Y VISUAL dos LEDs a corrigir (linha do topo)
  int start_correction_x_visual = 0; // Coordenada X VISUAL inicial
  int wrong_led_count = 13;

  for (int x_visual_corrigir = start_correction_x_visual; x_visual_corrigir < wrong_led_count; x_visual_corrigir++) {
      
          CRGB& pixelParaCorrigir = leds(x_visual_corrigir, correction_y_visual); // Obtém REFERÊNCIA ao píxel no buffer
          
          byte r_atual_no_buffer = pixelParaCorrigir.r; // Este é R da imagem
          byte g_atual_no_buffer = pixelParaCorrigir.g; // Este é G da imagem
          // byte b_atual_no_buffer = pixelParaCorrigir.b; // B não é trocado

          // Aplica a troca para compensar a má interpretação da fita/LEDs
          pixelParaCorrigir.r = g_atual_no_buffer;
          pixelParaCorrigir.g = r_atual_no_buffer;
          // pixelParaCorrigir.b permanece o mesmo
      
  }
  // --- FIM DA CORREÇÃO ---

  // 3. Envia os dados (com a correção) para a matriz física
  FastLED.show();
}


/**
 * @brief Reproduz uma sequência de animação completa.
 * (Função sem alterações em relação à sua versão anterior para animações)
 */
void playAnimationSequence(
    const CRGB animation_array_progmem[][NUM_PIXELS_GENERATED],
    int num_frames_in_sequence,
    unsigned long frame_duration_ms,
    unsigned long last_frame_hold_ms,
    int repeat_count) {

  if (num_frames_in_sequence == 0) return;

  for (int r = 0; r < repeat_count; r++) {
    for (int frame_idx = 0; frame_idx < num_frames_in_sequence; frame_idx++) {
      displayProgmemFrame(animation_array_progmem[frame_idx]);

      if (frame_idx == num_frames_in_sequence - 1 && last_frame_hold_ms > 0) {
        delay(last_frame_hold_ms);
      } else {
        delay(frame_duration_ms);
      }
    }
  }
}

// --- Setup e Loop ---
void setup() {
  delay(1000);

  // MUITO IMPORTANTE: Use COLOR_ORDER RGB aqui, pois a correção é feita manualmente depois.
  FastLED.addLeds<CHIPSET, LED_PIN, RGB>(leds[0], NUM_PIXELS) 
         .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS); // Uso a define BRIGHTNESS do topo
  FastLED.clear(true);
}

void loop() {
  // --- Reproduzir Animação NEEC ---
  // Verifique os nomes exatos (NEEC_anim, NEEC_ANIM_NUM_FRAMES) no seu image_data.h!
  #ifdef NEEC_ANIM_NUM_FRAMES
    playAnimationSequence(
        NEEC_anim,
        NEEC_ANIM_NUM_FRAMES,
        NEEC_SEQUENCE_FRAME_DURATION_MS,
        0,
        NEEC_SEQUENCE_LOOP_COUNT
    );
  #endif

  // --- Reproduzir Animação Bifana ---
  #ifdef BIFANA_ANIM_NUM_FRAMES
    playAnimationSequence(
        Bifana_anim,
        BIFANA_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS,
        1
    );
  #endif

  // --- Reproduzir Animação Batata ---
  #ifdef BATATA_ANIM_NUM_FRAMES
    playAnimationSequence(
        Batata_anim,
        BATATA_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS,
        1
    );
  #endif

  // --- Reproduzir Animação Menu ---
  #ifdef MENU_ANIM_NUM_FRAMES
    playAnimationSequence(
        Menu_anim,
        MENU_ANIM_NUM_FRAMES,
        OTHER_SEQUENCES_FRAME_DURATION_MS,
        OTHER_SEQUENCES_LAST_FRAME_HOLD_MS,
        1
    );
  #endif
}