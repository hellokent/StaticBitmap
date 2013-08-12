#ifndef DECODE_H
#define DECODE_H


#include <stdio.h>
#include <stdlib.h>
#include <android/bitmap.h>

#include "log.h"

extern "C" {

#include "libpng/png.h"
#include "jpeg/jpeglib.h"

typedef struct {
    uint8_t *base;
    int stride;
    int pixel_type;
    int width;
    int height;
    AndroidBitmapInfo *info;
}Image;


void destoryImage(Image* image);

// if success return 1, else return 0.
int decode2RGBA (FILE* srcFile, char const* cacheDir, Image* image);

void resizeRegion (Image* image, int tl, int tr, int bl, int br);

int saveRGBA (Image *image, FILE* file);

int readRGBA (Image *image, FILE* file);

void resize(Image dst, Image src, int core);

void draw2Bitmap(uint8_t *bitmap, Image* , AndroidBitmapInfo* );

void draw2Bitmap_offset(uint8_t *bitmap, Image* , AndroidBitmapInfo*, int , int );
}


extern const int PIXEL_RGBA8888;
extern const int PIXEL_RGBA4444;
extern const int PIXEL_RGB565;

extern const int RESIZE_NEAREST_NEIGHBOUR_INTERPOLATION;
extern const int RESIZE_BILINEAR_INTERPOLATION;

#endif
