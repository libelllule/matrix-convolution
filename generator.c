#include <stdint.h>
#include <stdio.h>

int main() {
  FILE* f = fopen("my_test.in", "wb");
  if (!f) return 1;
  uint32_t H = 2, W = 2;
  fwrite(&H, sizeof(uint32_t), 1, f);
  fwrite(&W, sizeof(uint32_t), 1, f);
  uint8_t img[12] = {50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50};
  fwrite(img, sizeof(uint8_t), 12, f);
  uint16_t DH = 3, DW = 3;
  fwrite(&DH, sizeof(uint16_t), 1, f);
  fwrite(&DW, sizeof(uint16_t), 1, f);
  int8_t filter[9] = {0, 0, 0, 0, 1, 0, 0, 0, 0};
  fwrite(filter, sizeof(int8_t), 9, f);
  fclose(f);
  printf("Файл my_test.in успешно создан!\n");
  return 0;
}