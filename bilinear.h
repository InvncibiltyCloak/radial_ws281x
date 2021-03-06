#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

typedef struct {
  uint8_t blue;
  uint8_t green;
  uint8_t red;
  uint8_t pad;
} Pixel;

typedef struct {
  uint16_t width;
  uint16_t height;
  Pixel *data;
} Image;

typedef struct {
  uint16_t x;
  uint16_t y;
} Coord;



Pixel* interpolate(Image *img, double x, double y);

#define EPRINT(args...) fprintf (stderr, args)
