#include "main.h"

#define ROTATION_RATE 100 //RPM
#define REFRESH_RATE  165 //Hz

static void setup_handlers(void);

Image image;
static uint8_t running = 1;

ws2811_t ledstring = {
    .freq = TARGET_FREQ,
    .dmanum = DMA,
    .channel = {
        [0] = {
            .gpionum = GPIO_PIN,
            .count = LED_COUNT,
            .invert = 0,
            .brightness = 255,
            .strip_type = STRIP_TYPE,
        },
        [1] = {
            .gpionum = 0,
            .count = 0,
            .invert = 0,
            .brightness = 0,
        },
    },
};

ws2811_led_t **lookup;

int main(int argc, char* argv[]){
  if(argc < 2){
    printf("Not enough input arguments!\n");
    printf("Usage: %s filename\n", argv[0]);
    return -1;
  }

  loadImage(&image, argv[1]);

  cropToSquare(&image);

  uint16_t numSlices = (REFRESH_RATE * 60) / ROTATION_RATE;

  ws2811_init(&ledstring);
  // Undo the built-in allocation, since we are going to allocate a lot more
  free(ledstring.channel[0].leds);

  lookup = allocateLookup(numSlices);
  generateLookup(lookup, numSlices);

  setup_handlers();

  unsigned int updatesCompleted = 0;

  #ifdef PERFCOUNT
  struct timespec start, end;

  printf("Update rate:\n");
  if(clock_gettime(CLOCK_REALTIME, &start)) {
    EPRINT("Error: Couldnt get clock time!");
  }
  #endif

  //Main loop
  while(running) {
    ledstring.channel[0].leds = lookup[updatesCompleted % numSlices];
    if (ws2811_render(&ledstring)) {
      EPRINT("Error: Rendering string didnt return 0!\n");
      break;
    }
    updatesCompleted += 1;

    #ifdef PERFCOUNT
    if(!(updatesCompleted % 300)) {
      if(clock_gettime(CLOCK_REALTIME, &end)) {
        EPRINT("Error: Couldnt get clock time!");
      }
      double freq = 300000.0/((end.tv_sec - start.tv_sec)*1000 + (end.tv_nsec - start.tv_nsec)/1000000);
      printf("\t%.2f Hz\r", freq);
      fflush(stdout);
      start = end;
    }
    #endif
  }

  for(int i = 0; i < LED_COUNT; i++) {
    ledstring.channel[0].leds[i] = 0;
  }
  ws2811_render(&ledstring);
  ws2811_fini(&ledstring);
}

// Given a filename and an image object, it loads the file with that name into
// the object.
void loadImage(Image *image, char* filename) {
  FILE* jpegfile = fopen(filename, "rb");
  if(jpegfile == NULL) {
    EPRINT("Error: Unable to open file!\n");
    exit(-1);
  }

  struct stat stbuf;
  if ((fstat(fileno(jpegfile), &stbuf) != 0) || (!S_ISREG(stbuf.st_mode))) {
    EPRINT("Error: Unable to determine file size!\n");
    exit(-1);
  }

  off_t  filesize = stbuf.st_size;

  unsigned char* buffer = malloc(filesize);
  if (buffer == NULL) {
    EPRINT("Error: Unable to allocate memory for jpeg!\n");
    exit(-1);
  }

  if(fread(buffer, 1, filesize, jpegfile) != filesize) {
    EPRINT("Error: Unable to read file!\n");
    exit(-1);
  }

  fclose(jpegfile);

  tjhandle decomp;
  if(!(decomp = tjInitDecompress())) {
    EPRINT("Error: Unable to initialize TurboJPEG decompressor!\n");
    EPRINT("%s\n", tjGetErrorStr());
    exit(-1);
  }

  int width, height, jpegSubsamp, jpegColorspace;

  if(tjDecompressHeader3(decomp, buffer, filesize, &width, &height, &jpegSubsamp, &jpegColorspace)) {
    EPRINT("Error: Unable to read JPEG header!\n");
    EPRINT("%s\n", tjGetErrorStr());
    exit(-1);
  }

  image->width = width;
  image->height = height;

  unsigned long decompressed_size;
  decompressed_size = width*height*tjPixelSize[PIXEL_FORMAT];
  unsigned char* buffer2 = malloc(decompressed_size);

  if(tjDecompress2(decomp, buffer, filesize, buffer2, width, width * tjPixelSize[PIXEL_FORMAT], height, PIXEL_FORMAT, TJFLAG_NOREALLOC)) {
    EPRINT("Error: Unable to decompress JPEG image!\n");
    EPRINT("%s\n", tjGetErrorStr());
    exit(-1);
  }

  // Free up some memory since we are done with image decoding
  tjDestroy(decomp);
  free(buffer);

  assert(tjPixelSize[PIXEL_FORMAT] == sizeof(Pixel));
  image->data = (Pixel *) buffer2;

  return;
}

// Given an image object, this function crops it to be the largest square from
// the original
void cropToSquare(Image *image) {
  if(image->width < image->height) {
    memmove(image->data,
            image->data + ((image->height - image->width)/2*image->width),
            ((uint32_t)image->width)^2*sizeof(Pixel));
    image->data = realloc(image->data, ((uint32_t)image->width)^2*sizeof(Pixel));
    image->height = image->width;
  } else {
    uint16_t start_offset = (image->width - image->height)/2;
    for(uint16_t row = 0; row < image->height; row++){
      memmove(image->data + (row*image->height),
              image->data + (image->width*row + start_offset),
              image->height*sizeof(Pixel));
    }
    image->data = realloc(image->data, ((uint32_t)image->height)^2*sizeof(Pixel));
    image->width = image->height;
  }
  if(!image->data) {
    EPRINT("Error: Unable to realloc image data after crop!\n");
    exit(-1);
  }
}

// Allocates the memory needed for a lookup table of a given number of slices
ws2811_led_t **allocateLookup(int numSlices) {
  ws2811_led_t **lookup;
  if(!(lookup = malloc(numSlices * sizeof *lookup))) {
    EPRINT("Error: Unable to allocate space for lookup table top level!\n");
    exit(-1);
  }

  for(int i = 0; i < numSlices; i++) {
    if(!(lookup[i] = malloc(LED_COUNT * sizeof *(lookup[i])))) {
      EPRINT("Error: Unable to allocate space for lookup table data!\n");
      exit(-1);
    }
  }
  return lookup;
}

// Runs the bilinear image interpolation to generate the values of the lookup table
void generateLookup(ws2811_led_t **lookup, int numSlices) {
  assert(sizeof(ws2811_led_t) == sizeof(Pixel));
  for(int ang_stop = 0; ang_stop < numSlices; ang_stop++) {
    for(int i = 0; i < LED_COUNT; i++) {
      double r = (i+1.0)/LED_COUNT;
      double th = ang_stop * 2 * M_PI / numSlices;
      lookup[ang_stop][i] = *((ws2811_led_t *)interpolate(&image, r*cos(th), r*sin(th)));
    }
  }
}

static void ctrl_c_handler(int signum) {
  running = 0;
}

static void setup_handlers(void) {
  struct sigaction sa =
  {
    .sa_handler = ctrl_c_handler,
  };

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}
