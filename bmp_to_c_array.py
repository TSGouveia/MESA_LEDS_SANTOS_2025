from PIL import Image
import os
import sys
import re  # Para sanitizar nomes

# Dimensões fixas da matriz
MATRIX_WIDTH = 32
MATRIX_HEIGHT = 18
NUM_PIXELS_TOTAL = MATRIX_WIDTH * MATRIX_HEIGHT


def sanitize_name(name):
    """Remove caracteres não alfanuméricos para nomes de variáveis C."""
    name = re.sub(r'[^a-zA-Z0-9_]', '', name)
    if not name or name[0].isdigit():  # Garante que não comece com número e não seja vazio
        name = "_" + name
    return name


def natural_sort_key(s):
    """Chave para ordenação natural (ex: item1, item2, item10)."""
    return [int(text) if text.isdigit() else text.lower() for text in re.split('([0-9]+)', s)]


def create_c_array_for_sequence(sequence_folder_path, base_c_name_for_sequence):
    """
    Cria o código C para uma única sequência de animação (uma subpasta).
    Retorna a string do código C, o nome do array gerado e o nome da define de contagem de frames.
    """
    if not os.path.isdir(sequence_folder_path):
        print(f"  Aviso: Subpasta não encontrada ou não é um diretório: {sequence_folder_path} (ignorado)")
        return None, None, None

    try:
        bmp_files_unsorted = [f for f in os.listdir(sequence_folder_path) if f.lower().endswith(".bmp")]
        if not bmp_files_unsorted:
            print(f"  Nenhum arquivo .bmp encontrado em {sequence_folder_path} (ignorado)")
            return None, None, None

        bmp_files = sorted(bmp_files_unsorted, key=natural_sort_key)
        print(f"    Processando sequência '{os.path.basename(sequence_folder_path)}' com arquivos: {bmp_files}")

    except Exception as e:
        print(f"  Erro ao listar arquivos em {sequence_folder_path}: {e} (ignorado)")
        return None, None, None

    num_frames = len(bmp_files)
    if num_frames == 0:
        return None, None, None

    array_name_c = f"{base_c_name_for_sequence}_anim"
    num_frames_define_name = f"{base_c_name_for_sequence.upper()}_ANIM_NUM_FRAMES"

    c_code_for_sequence = f"// Animação da sequência: {os.path.basename(sequence_folder_path)}\n"
    c_code_for_sequence += f"#define {num_frames_define_name} {num_frames}\n"
    c_code_for_sequence += f"const PROGMEM CRGB {array_name_c}[{num_frames_define_name}][{NUM_PIXELS_TOTAL}] = {{\n"

    for i, bmp_filename in enumerate(bmp_files):
        image_path = os.path.join(sequence_folder_path, bmp_filename)
        try:
            img = Image.open(image_path)
        except Exception as e:
            print(f"      Erro ao abrir imagem {image_path}: {e} (frame será PRETO)")
            img = Image.new('RGB', (MATRIX_WIDTH, MATRIX_HEIGHT), (0, 0, 0))  # Frame preto

        if img.width != MATRIX_WIDTH or img.height != MATRIX_HEIGHT:
            print(
                f"      Aviso: Redimensionando {bmp_filename} de {img.width}x{img.height} para {MATRIX_WIDTH}x{MATRIX_HEIGHT}")
            try:
                img = img.resize((MATRIX_WIDTH, MATRIX_HEIGHT), Image.Resampling.LANCZOS)
            except Exception as e:
                print(f"      Falha ao redimensionar {bmp_filename}: {e} (frame será PRETO)")
                img = Image.new('RGB', (MATRIX_WIDTH, MATRIX_HEIGHT), (0, 0, 0))

        try:
            img = img.convert("RGB")  # Garante que está em formato RGB
        except Exception as e:
            print(f"      Falha ao converter {bmp_filename} para RGB: {e} (frame será PRETO)")
            img = Image.new('RGB', (MATRIX_WIDTH, MATRIX_HEIGHT), (0, 0, 0))

        c_code_for_sequence += f"  {{ // Frame {i} ({bmp_filename})\n    "
        pixel_count_in_frame = 0
        for y_idx in range(MATRIX_HEIGHT):
            for x_idx in range(MATRIX_WIDTH):
                try:
                    r, g, b = img.getpixel((x_idx, y_idx))
                except Exception as e:
                    # Em caso de erro ao ler pixel (improvável após conversão para RGB e resize)
                    # print(f"        Erro ao ler pixel ({x_idx},{y_idx}) de {bmp_filename}: {e}")
                    r, g, b = 0, 0, 0  # Pixel preto

                c_code_for_sequence += f"CRGB({r}, {g}, {b}), "
                pixel_count_in_frame += 1
                if pixel_count_in_frame % 8 == 0 and (pixel_count_in_frame != NUM_PIXELS_TOTAL):
                    c_code_for_sequence += "\n    "
        c_code_for_sequence = c_code_for_sequence.strip().rstrip(',') + "\n  },\n"

    c_code_for_sequence = c_code_for_sequence.strip().rstrip(',') + "\n};"
    return c_code_for_sequence, array_name_c, num_frames_define_name


if __name__ == "__main__":
    print("--- Conversor de Animações BMP para FastLED (32x18) ---")

    # Solicitar pasta principal de animações
    while True:
        main_animations_folder_input = input(
            "Digite o caminho para a pasta principal contendo as subpastas de animação: ").strip()
        if os.path.isdir(main_animations_folder_input):
            break
        else:
            print(
                f"Erro: Pasta '{main_animations_folder_input}' não encontrada ou não é um diretório. Tente novamente.")

    # Solicitar nome do arquivo de saída
    output_file_input = input(f"Digite o nome do arquivo .h de saída (padrão: image_data.h): ").strip()
    if not output_file_input:
        output_file_input = "image_data.h"
    elif not output_file_input.lower().endswith(".h"):
        output_file_input += ".h"  # Garante a extensão .h

    all_c_code_blocks = []
    processed_sequences_info = []

    print(f"\nProcessando pasta principal: {main_animations_folder_input}")
    print(f"Dimensões da matriz fixas em: {MATRIX_WIDTH}x{MATRIX_HEIGHT}\n")

    # Lista subpastas (que serão as sequências de animação)
    try:
        subfolders = sorted([
            f.path for f in os.scandir(main_animations_folder_input) if f.is_dir()
        ], key=lambda p: natural_sort_key(os.path.basename(p)))
    except Exception as e:
        print(f"Erro ao listar subpastas em '{main_animations_folder_input}': {e}")
        sys.exit(1)

    if not subfolders:
        print(f"Nenhuma subpasta encontrada em '{main_animations_folder_input}'. Nada a fazer.")
        sys.exit(0)

    for subfolder_path in subfolders:
        sequence_name_raw = os.path.basename(subfolder_path)
        base_c_name = sanitize_name(sequence_name_raw)

        print(f"Processando subpasta: '{sequence_name_raw}' (será '{base_c_name}' no código C)")
        c_code_block, array_name, define_name = create_c_array_for_sequence(subfolder_path, base_c_name)

        if c_code_block:
            all_c_code_blocks.append(c_code_block)
            processed_sequences_info.append({'name': base_c_name, 'array': array_name, 'define': define_name})
        else:
            print(f"  Subpasta '{sequence_name_raw}' não gerou código (sem BMPs válidos ou erro).\n")

    if not all_c_code_blocks:
        print("Nenhuma animação processada com sucesso. Arquivo de saída não será gerado.")
        sys.exit(0)

    # Monta o arquivo .h final
    final_h_content = "#ifndef IMAGE_DATA_H\n"
    final_h_content += "#define IMAGE_DATA_H\n\n"
    final_h_content += "#include <FastLED.h>    // Para CRGB\n"
    final_h_content += "#include <avr/pgmspace.h> // Para PROGMEM\n\n"
    final_h_content += f"// Dimensões da Matriz (usadas na geração destes dados)\n"
    final_h_content += f"// Certifique-se que seu código Arduino usa MATRIX_WIDTH {MATRIX_WIDTH} e MATRIX_HEIGHT {MATRIX_HEIGHT}\n"
    final_h_content += f"// #define MATRIX_WIDTH   {MATRIX_WIDTH}\n"
    final_h_content += f"// #define MATRIX_HEIGHT  {MATRIX_HEIGHT}\n"
    final_h_content += f"#define NUM_PIXELS_GENERATED ({MATRIX_WIDTH} * {MATRIX_HEIGHT}) // {NUM_PIXELS_TOTAL}\n\n"

    for code_block in all_c_code_blocks:
        final_h_content += code_block + "\n\n"

    final_h_content += "#endif // IMAGE_DATA_H\n"

    try:
        with open(output_file_input, "w") as f:
            f.write(final_h_content)
        print(f"\nArquivo '{output_file_input}' gerado com sucesso contendo as seguintes animações:")
        for info in processed_sequences_info:
            print(f"  - Sequência: {info['name']} -> Array: {info['array']}, Define: {info['define']}")
        print(f"\nLembre-se de colocar '{output_file_input}' na mesma pasta do seu sketch Arduino.")
    except IOError as e:
        print(f"Erro ao escrever o arquivo de saída '{output_file_input}': {e}")