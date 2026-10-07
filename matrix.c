#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define FLAGS ":i:o:"
#define EXIT_FLAG_FAILURE 10
#define MATRIX_NUM 3
#define MAX_VAL 255
#define MOD_POS 251
#define MOD_NEG 241

typedef struct {
  uint32_t H;
  uint32_t W;
  uint8_t* data;
} MatrixABC;

typedef struct {
  uint16_t DH;
  uint16_t DW;
  int8_t* data;
} MatrixD;

void cleanup_resources(MatrixABC* matrix_abc, MatrixD* matrix_d,
                       MatrixABC* result_abc) {
  if (matrix_abc->data) free(matrix_abc->data);
  if (matrix_d->data) free(matrix_d->data);
  if (result_abc->data) free(result_abc->data);
}

int parse_args(int argc, char* argv[], const char** in_path,
               const char** out_path) {
  int opt;
  opterr = 0;
  while ((opt = getopt(argc, argv, FLAGS)) != -1) {
    if (opt == 'i')
      *in_path = optarg;
    else if (opt == 'o')
      *out_path = optarg;
    else if (opt == ':')
      return EXIT_FLAG_FAILURE;
    else if (opt == '?')
      return EXIT_FAILURE;
  }
  if (*in_path == NULL || *out_path == NULL) {
    return EXIT_FLAG_FAILURE;
  }
  return 0;
}

int read_input_file(const char* path, MatrixABC* matrix_abc,
                    MatrixD* matrix_d) {
  FILE* read_file = fopen(path, "rb");
  if (read_file == NULL) return EXIT_FLAG_FAILURE;
  if (fread(&matrix_abc->H, sizeof(uint32_t), 1, read_file) != 1 ||
      fread(&matrix_abc->W, sizeof(uint32_t), 1, read_file) != 1) {
    fclose(read_file);
    return EXIT_FAILURE;
  }
  if (matrix_abc->H != 0 &&
      matrix_abc->W > SIZE_MAX / MATRIX_NUM / matrix_abc->H) {
    fclose(read_file);
    return EXIT_FAILURE;
  }
  size_t abc_size = matrix_abc->H * matrix_abc->W * MATRIX_NUM;
  matrix_abc->data = malloc(abc_size * sizeof(uint8_t));
  if (matrix_abc->data == NULL || fread(matrix_abc->data, sizeof(uint8_t),
                                        abc_size, read_file) != abc_size) {
    fclose(read_file);
    return EXIT_FAILURE;
  }
  if (fread(&matrix_d->DH, sizeof(uint16_t), 1, read_file) != 1 ||
      fread(&matrix_d->DW, sizeof(uint16_t), 1, read_file) != 1) {
    fclose(read_file);
    return EXIT_FAILURE;
  }
  size_t d_size = matrix_d->DH * matrix_d->DW;
  matrix_d->data = malloc(d_size * sizeof(int8_t));
  if (matrix_d->data == NULL ||
      fread(matrix_d->data, sizeof(int8_t), d_size, read_file) != d_size) {
    fclose(read_file);
    return EXIT_FAILURE;
  }
  fclose(read_file);
  return 0;
}

int64_t calculate_pixel(const MatrixABC* matrix_abc, const MatrixD* matrix_d,
                        uint32_t y, uint32_t x, int c, int cy, int cx) {
  int64_t sum = 0;
  for (uint16_t ky = 0; ky < matrix_d->DH; ky++) {
    for (uint16_t kx = 0; kx < matrix_d->DW; kx++) {
      int target_y = y + ky - cy;
      int target_x = x + kx - cx;
      if (target_y >= 0 && (uint32_t)target_y < matrix_abc->H &&
          target_x >= 0 && (uint32_t)target_x < matrix_abc->W) {
        uint32_t index_abc =
            ((target_y * matrix_abc->W) + target_x) * MATRIX_NUM + c;
        uint32_t index_d = ky * matrix_d->DW + kx;
        sum += matrix_abc->data[index_abc] * matrix_d->data[index_d];
      }
    }
  }
  return sum;
}

int calculate_result(const MatrixABC* matrix_abc, const MatrixD* matrix_d,
                     MatrixABC* result_abc) {
  result_abc->H = matrix_abc->H;
  result_abc->W = matrix_abc->W;
  size_t abc_size = result_abc->H * result_abc->W * MATRIX_NUM;
  result_abc->data = malloc(abc_size * sizeof(uint8_t));
  if (result_abc->data == NULL) return EXIT_FAILURE;
  int cy = matrix_d->DH / 2;
  int cx = matrix_d->DW / 2;
  for (uint32_t y = 0; y < matrix_abc->H; y++) {
    for (uint32_t x = 0; x < matrix_abc->W; x++) {
      for (int c = 0; c < MATRIX_NUM; c++) {
        int64_t sum = calculate_pixel(matrix_abc, matrix_d, y, x, c, cy, cx);
        if (sum > MAX_VAL) {
          sum = sum % MOD_POS;
        } else if (sum < 0) {
          sum = -sum;
          sum = sum % MOD_NEG;
        }
        uint32_t result_index = ((y * matrix_abc->W) + x) * MATRIX_NUM + c;
        result_abc->data[result_index] = (uint8_t)sum;
      }
    }
  }
  return 0;
}

int write_output_file(const char* path, const MatrixABC* result_abc) {
  FILE* write_file = fopen(path, "wb");
  if (write_file == NULL) return EXIT_FLAG_FAILURE;
  size_t abc_size = result_abc->H * result_abc->W * MATRIX_NUM;
  if (fwrite(&result_abc->H, sizeof(uint32_t), 1, write_file) != 1 ||
      fwrite(&result_abc->W, sizeof(uint32_t), 1, write_file) != 1 ||
      fwrite(result_abc->data, sizeof(uint8_t), abc_size, write_file) !=
          abc_size) {
    fclose(write_file);
    return EXIT_FAILURE;
  }
  fclose(write_file);
  return 0;
}

int main(int argc, char* argv[]) {
  const char* in_path = NULL;
  const char* out_path = NULL;
  MatrixABC matrix_abc = {0};
  MatrixD matrix_d = {0};
  MatrixABC result_abc = {0};
  int status = parse_args(argc, argv, &in_path, &out_path);
  if (status != 0) return status;
  status = read_input_file(in_path, &matrix_abc, &matrix_d);
  if (status != 0) {
    cleanup_resources(&matrix_abc, &matrix_d, &result_abc);
    return status;
  }
  status = calculate_result(&matrix_abc, &matrix_d, &result_abc);
  if (status != 0) {
    cleanup_resources(&matrix_abc, &matrix_d, &result_abc);
    return status;
  }
  status = write_output_file(out_path, &result_abc);
  cleanup_resources(&matrix_abc, &matrix_d, &result_abc);
  return status;
}