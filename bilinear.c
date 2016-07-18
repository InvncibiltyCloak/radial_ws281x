#include "bilinear.h"

static void weighted_average_pixel(Pixel *first, Pixel *second, Pixel *result, double weight);
static Pixel* get_at(Image *img, Coord coord);

Pixel* interpolate(Image *img, double x, double y) {
  assert(x >= 0 && y >= 0); // This is normally hidden by the cast to uint
  Coord tl_coord = {
    .x = (uint16_t) x,
    .y = (uint16_t) y
  };
  Pixel *tl_pix = get_at(img, tl_coord);

  Coord br_coord = {
    .x = (uint16_t) (x+1),
    .y = (uint16_t) (y+1)
  };
  Pixel *br_pix = get_at(img, br_coord);

  Coord tr_coord = {
    .x = br_coord.x,
    .y = tl_coord.y
  };
  Pixel *tr_pix = get_at(img, tr_coord);

  Coord bl_coord = {
    .x = tl_coord.x,
    .y = br_coord.y
  };
  Pixel *bl_pix = get_at(img, bl_coord);

  double resid_x = x - tl_coord.x;
  double resid_y = y - tl_coord.y;

  Pixel *result = malloc(sizeof *result);
  if(!result) {
    EPRINT("Error: Unable to allocate memory for a pixel!\n");
    exit(-1);
  }
  Pixel temp;
  weighted_average_pixel(br_pix, bl_pix, result, resid_x);
  weighted_average_pixel(tr_pix, tl_pix, &temp, resid_x);
  weighted_average_pixel(result, &temp, result, resid_y);
  return result;
}

static Pixel* get_at(Image *img, Coord coord) {
  assert(coord.x < img->width && coord.y < img->height);
    // Dont need to check lower bound because it is underflow.
  return img->data + (coord.y * img->width + coord.x);
}

/* Averages the RGB values of two pixels based on a weighting. The resulting
 * value is stored in the result pixel, so make sure it can be clobbered.
 */
static void weighted_average_pixel(Pixel *first, Pixel *second, Pixel *result, double weight) {
  result->red = (uint8_t)(first->red*weight + second->red*(1-weight));
  result->green = (uint8_t)(first->green*weight + second->green*(1-weight));
  result->blue = (uint8_t)(first->blue*weight + second->blue*(1-weight));
}
