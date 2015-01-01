#include <pebble.h>
#include "drawing.h"

// Thanks to https://github.com/Jnmattern/Minimalist/blob/master/src/bitmap.h for bmp drawing
void bmpPutPixel(GBitmap *bmp, int x, int y, GColor c) {
  if (x >= bmp->bounds.size.w || y >= bmp->bounds.size.h || x < 0 || y < 0) return;
  int byteoffset = y*bmp->row_size_bytes + x/8;
  ((uint8_t *)bmp->addr)[byteoffset] &= ~(1<<(x%8));
  if (c == GColorWhite) ((uint8_t *)bmp->addr)[byteoffset] |= (1<<(x%8));
}

void bmpRect(GBitmap *bmp, GRect rect, GColor c, bool fill) {
  int i, j;
  int xe = rect.origin.x + rect.size.w;
  int ye = rect.origin.y + rect.size.h;
  for (j=rect.origin.y ; j<ye; j++) {
    for (i=rect.origin.x ; i<xe; i++) {
      if (fill || (!(j > rect.origin.y && j < (ye - 1)) || (i == rect.origin.x || i == (xe - 1)))) {
        bmpPutPixel(bmp, i, j, c);
      }
    }
  }
}