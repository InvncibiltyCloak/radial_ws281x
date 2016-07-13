#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <turbojpeg.h>

#include "bilinear.h"

#define EPRINT(args...) fprintf (stderr, args)
#define DPRINT(args...) printf (args)

int main(int argc, char* argv[]);
void loadImage(Image *image, char* filename);
void cropToSquare(Image *image);
