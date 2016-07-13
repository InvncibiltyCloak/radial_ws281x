#include "main.h"

#define ROTATION_RATE 100 //RPM
#define REFRESH_RATE  200 //Hz


Image image;

int main(int argc, char* argv[]){
  if(argc < 1){
    printf("Not enough input arguments!\n");
    printf("Usage: %s filename\n", argv[0]);
    return -1;
  }

  loadImage(&image, argv[1]);

  cropToSquare(&image);


}

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

  unsigned char* buffer = (unsigned char*)malloc(filesize);
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
  decompressed_size = tjBufSize(width, height, jpegSubsamp);
  unsigned char* buffer2 = (unsigned char*) malloc(decompressed_size);

  if(tjDecompress2(decomp, buffer, filesize, buffer2, width, width * tjPixelSize[TJPF_RGB], height, TJPF_RGB, TJFLAG_NOREALLOC)) {
    EPRINT("Error: Unable to decompress JPEG image!\n");
    EPRINT("%s\n", tjGetErrorStr());
    exit(-1);
  }

  // Free up some memory since we are done with image decoding
  tjDestroy(decomp);
  free(buffer);

  assert(tjPixelSize[TJPF_RGB] == sizeof(Pixel));
  image->data = (Pixel *) buffer2;

  return;
}

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
