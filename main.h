#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "bilinear.h"
#include "ws2811.h"
#include "turbojpeg.h"

#define PIXEL_FORMAT              TJPF_XRGB

#define TARGET_FREQ               WS2811_TARGET_FREQ
#define GPIO_PIN                  18
#define DMA                       5
#define STRIP_TYPE                WS2811_STRIP_RGB
#define WIDTH                     144
#define HEIGHT                    1
#define LED_COUNT                 (WIDTH * HEIGHT)

#define EPRINT(args...) fprintf (stderr, args)
#define DPRINT(args...) printf (args)

int main(int argc, char* argv[]);
void loadImage(Image *image, char* filename);
void cropToSquare(Image *image);
ws2811_led_t **allocateLookup(int numSlices);
void generateLookup(ws2811_led_t **lookup, int numSlices);

struct coord {
  double x;
  double y;
};
