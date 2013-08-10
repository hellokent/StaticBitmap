#include "decode.h"
#include <memory.h>

int try_jpeg(FILE *sfp, char const *cache, Image *image);
int try_png(FILE *sfp, Image *image);

int decode2RGBA (FILE* srcFILE, char const* cache_dir, Image* image){
	// methods

  	// check FILE open status
  	if(!srcFILE) {
  		LOGD("srcFILE is null");
  		return 0;
  	}

  	if(try_jpeg(srcFILE, cache_dir, image)){
  		LOGD("jpeg success");
  		return 1;
  	}

  	if (try_png(srcFILE, image)){
  		LOGD("png success");
  		return 1;
  	}

  	return 0;
}

int saveRGBA (Image *image, FILE* file){
  	if (!image || !file){
  	  	return 0;
  	}
  	fwrite(&(image->width), 4, 1, file);
  	LOGD("write width:%d", image->width);
  	fwrite(&(image->height), 4, 1, file);
  	LOGD("write height:%d", image->height);
  	fwrite(&(image->stride), 4, 1, file);
  	LOGD("write stride:%d", image->stride);
  	fwrite(&(image->pixel_type), 4, 1, file);
  	LOGD("write pixel_type:%d", image->pixel_type);
  	fwrite(image->base, 1, image->stride * image->height, file);
  	fflush(file);
  	LOGD("save finish");
  	return 1;
}

int readRGBA (Image *image, FILE* file){
  	if (!image || !file){
        LOGD("illegal ptr");
  	  	return 0;
  	}
  	fread(&(image->width), 4, 1, file);
  	LOGD("read width:%d", image->width);
  	fread(&(image->height), 4, 1, file);
  	LOGD("read height:%d", image->height);
  	fread(&(image->stride), 4, 1, file);
  	LOGD("read stride:%d", image->stride);
  	fread(&(image->pixel_type), 4, 1, file);
  	LOGD("read pixel_type:%d", image->pixel_type);
  	image->base = (uint8_t *)malloc(image->stride * image->height);
  	fread(image->base, 1, image->stride * image->height, file);
  	LOGD("read base");
  	LOGD("read finish");
  	return 1;
}

void resizeRegion (Image* image, int tl, int tr, int bl, int br){
  	int height = bl - tl;
  	int width = br - tl;
  	int offset = 0;

  	LOGI("resize:%u, %u, %u, %u", tl, tr, bl, br);
  	uint8_t* base = (uint8_t *)malloc(height * width * 4);
  	if(!base){
        LOGE("malloc base failed int resizeRegion");
  	  	return;
  	}

  	for(int i = tl; i < bl; ++i){
  	  	memcpy(base + offset, image->base + i * image->stride, width * 4);
  	  	offset += width * 4;
  	}
  	LOGD("copy finished");
  	free(image->base);
  	image->base = base;
  	image->stride = width * 4;
  	image->width = width;
  	image->height = height;
  	LOGD("free and update stride finished");
}

typedef union {
	struct {
		struct jpeg_error_mgr pub;    /* "public" fields */
		jmp_buf setjmp_buffer;    /* for return to caller */
	}jpeg;
	struct {
	}png;
}my_error_t;

// callbacks
void jpeg_error_exit (j_common_ptr cinfo)
{
	/* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
	my_error_t *myerr = (my_error_t*) cinfo->err;

	/* print message */
	char buf[JMSG_LENGTH_MAX];
	cinfo->err->format_message(cinfo, buf);
	LOGV("jpeg error: %s", buf);

	/* Return control to the setjmp point */
	longjmp(myerr->jpeg.setjmp_buffer, 1);
}

// try jpeg
int try_jpeg(FILE *sfp, const char *cache, Image* image) {
	int success = 0;
	int width;
	int height;
	int stride;
	my_error_t err;
	int data_offset = 0;

	// jpg var
	struct jpeg_decompress_struct jpeg_dec;
	JSAMPARRAY jpeg_line = 0;

	rewind(sfp);
	jpeg_dec.client_data = (void*)cache;
	jpeg_dec.err = jpeg_std_error(&err.jpeg.pub);
	err.jpeg.pub.error_exit = jpeg_error_exit;

	if (setjmp(err.jpeg.setjmp_buffer)) {
		LOGD("set jmp in try_jpeg");
		jpeg_dec.err = NULL;
		jpeg_destroy_decompress(&jpeg_dec);
		LOGD("after set jmp return, in try_jpeg");
		return 0;
	}

	jpeg_create_decompress(&jpeg_dec);
	LOGV("create decompress");

	jpeg_stdio_src(&jpeg_dec, sfp);
	LOGV("stdio src");

	if(jpeg_read_header(&jpeg_dec, FALSE) != JPEG_HEADER_OK) {
		LOGD("is not jpeg");
		goto fail;
	}
	LOGV("got jpeg");

	(void) jpeg_start_decompress(&jpeg_dec);
	LOGV("start decompress");

	width = jpeg_dec.output_width;
	height = jpeg_dec.output_height;
	stride = jpeg_dec.output_width * jpeg_dec.output_components;

	image->width = width;
	image->height = height;
	image->stride = width * 4;
	image->pixel_type = PIXEL_RGBA8888;

	LOGV("jpeg: [width:%d, height:%d, stride:%d]", width, height, image->stride);
	jpeg_line = (jpeg_dec.mem->alloc_sarray)((j_common_ptr)&jpeg_dec, JPOOL_IMAGE, stride, 1);
	LOGV("stride %d, alloc line %p -> %p", stride, jpeg_line, *jpeg_line);

	image->base = (uint8_t *) malloc(image->stride * height);
	for(JSAMPROW row = *jpeg_line;jpeg_dec.output_scanline < jpeg_dec.output_height;) {
		jpeg_read_scanlines(&jpeg_dec, jpeg_line, 1);
		for(int x=0, p=0, rp=0; x<width; x++) {
			image->base[data_offset + rp++] = row[p++];
			image->base[data_offset + rp++] = row[p++];
			image->base[data_offset + rp++] = row[p++];
			image->base[data_offset + rp++] = 0xff;
		}
		data_offset += width * 4;
	}
	LOGV("jpeg convert finished");

success:
	success = 1;

fail:
	(void) jpeg_finish_decompress(&jpeg_dec);
	jpeg_destroy_decompress(&jpeg_dec);
	return success;
}

int try_png(FILE *sfp, Image *image) {
	int success = 0;
	png_uint_32 width;
	png_uint_32 height;
	png_uint_32 stride;
	int bit_depth, color_type, interlace_type;
	my_error_t err;
	char* rgba_line;
	int offset;

	// jpg var
	struct jpeg_decompress_struct jpeg_dec;
	JSAMPARRAY jpeg_line = 0;

	// png vars
	png_structp png_ptr = 0;
	png_infop png_info = 0;
	png_byte png_magic[8];
	png_bytep png_line = 0;

	rewind(sfp);
	fread(png_magic, sizeof(png_magic), 1, sfp);
	if(png_sig_cmp(png_magic, 0, 8)) {
		LOGD("is not png");
		goto fail;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
	if(!png_ptr) {
		LOGD("create read struct failed");
		goto fail;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		LOGD("png jmp point!");
		png_destroy_read_struct(&png_ptr, &png_info, 0);
		if(png_line)
			free(png_line);
		return 0;
	}

	png_info = png_create_info_struct(png_ptr);
	if(!png_info) {
		LOGD("create info struct failed");
		goto fail;
	}

	png_init_io(png_ptr, sfp);
	png_set_sig_bytes(png_ptr, sizeof(png_magic));
	png_read_info(png_ptr, png_info);
	png_set_expand(png_ptr);
	if (png_get_valid(png_ptr, png_info, PNG_INFO_tRNS))
		png_set_tRNS_to_alpha(png_ptr);
	png_read_update_info(png_ptr, png_info);
	png_get_IHDR(png_ptr, png_info, &width, &height, &bit_depth, &color_type, &interlace_type, int_p_NULL, int_p_NULL);
	image->width = width;
	image->height = height;
	LOGV("png [%d, %d], bit_depth %d", (int)width, (int)height, (int)bit_depth);

	stride = png_get_rowbytes(png_ptr, png_info);
	png_line = (png_bytep)malloc(stride);
	LOGV("stride %d, alloc line %p", (int)stride, png_line);
    if(!png_line){
        LOGD("failed in malloc image->base");
        goto fail;
    }

	rgba_line = (char *) malloc(width << 2);

	image->stride = 4 * image->width;
	image->base = (uint8_t *)malloc(width * 4 * height);
    if(!image->base){
        LOGD("failed in malloc image->base");
        goto fail;
    }

    offset = width * 4;

	switch(stride / width) {
	case 1:
		for(int y=0; y<height; y++) {
			png_read_row(png_ptr, png_line, png_bytep_NULL);
			for(int x=0, p=0, rp=0; x<width; x++) {
				rgba_line[rp++] = png_line[p];   // g
				rgba_line[rp++] = png_line[p];   // g
				rgba_line[rp++] = png_line[p++]; // g
				rgba_line[rp++] = 0xff;          // 0xff
			}
			memcpy(image->base + y * offset, rgba_line, offset);
		}
		break;
	case 2:
		for(int y=0; y<height; y++) {
			png_read_row(png_ptr, png_line, png_bytep_NULL);
			for(int x=0, p=0, rp=0; x<width; x++) {
				rgba_line[rp++] = png_line[p];   // g
				rgba_line[rp++] = png_line[p];   // g
				rgba_line[rp++] = png_line[p++]; // g
				rgba_line[rp++] = png_line[p++]; // a
			}
			memcpy(image->base + y * offset, rgba_line, offset);
		}
		break;
	case 3:
		for(int y=0; y<height; ++y) {
			png_read_row(png_ptr, png_line, png_bytep_NULL);
			for(int x=0, p=0, rp=0; x<width; ++x) {
				rgba_line[rp++] = png_line[p++]; // r
				rgba_line[rp++] = png_line[p++]; // g
				rgba_line[rp++] = png_line[p++]; // b
				rgba_line[rp++] = 0xff;          // 0xff
			}
			memcpy(image->base + y * offset, rgba_line, offset);
		}
		break;
	case 4:
		for(int y=0; y<height; y++) {
			png_read_row(png_ptr, png_line, png_bytep_NULL);
			for(int x=0, p=0, rp=0; x<width; x++) {
				rgba_line[rp++] = png_line[p++]; // r
				rgba_line[rp++] = png_line[p++]; // g
				rgba_line[rp++] = png_line[p++]; // b
				rgba_line[rp++] = png_line[p++]; // a
			}
			memcpy(image->base + y * offset, rgba_line, offset);
		}
		break;
	default:
		LOGD("unknown png bit depth %d", bit_depth);
		break;
	}
	LOGW("finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1");
success:
	success = 1;

fail:
	png_destroy_read_struct(&png_ptr, &png_info, 0);
	if(png_line)
		free(png_line);
	return success;
}
/*
enum AndroidBitmapFormat {
    ANDROID_BITMAP_FORMAT_NONE      = 0,
    ANDROID_BITMAP_FORMAT_RGBA_8888 = 1,
    ANDROID_BITMAP_FORMAT_RGB_565   = 4,
    ANDROID_BITMAP_FORMAT_RGBA_4444 = 7,
    ANDROID_BITMAP_FORMAT_A_8       = 8,
};
*/

const int PIXEL_RGBA8888 = 1;
const int PIXEL_RGB565 = 4;
const int PIXEL_RGBA4444 = 7;
const int RESIZE_NEAREST_NEIGHBOUR_INTERPOLATION = 1;
const int RESIZE_BILINEAR_INTERPOLATION = 2;

struct Pixel {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

class PixelIO
{
public:
    virtual int size() const = 0;
    virtual Pixel read(uint8_t*) const = 0;
    virtual void write(uint8_t*, Pixel) const = 0;
    static const PixelIO* get(int pixelType);
};

#define Copy2(dst, src) { \
    *(uint8_t*)dst = *(uint8_t*)src; \
    *(((uint8_t*)dst) + 1) =  *(((uint8_t*)src) + 1); \
}

#define Copy4(dst, src) { \
    *(uint8_t*)dst = *(uint8_t*)src; \
    *(((uint8_t*)dst) + 1) =  *(((uint8_t*)src) + 1); \
    *(((uint8_t*)dst) + 2) =  *(((uint8_t*)src) + 2); \
    *(((uint8_t*)dst) + 3) =  *(((uint8_t*)src) + 3); \
}

void draw2Bitmap_offset(uint8_t *bitmapPtr, Image* image, AndroidBitmapInfo* info, int offset_x, int offset_y){
    const PixelIO *dstIO = PixelIO::get(info->format);
    const PixelIO *srcIO = PixelIO::get(PIXEL_RGBA8888);
    const int dstSize = dstIO->size();
    const int srcSize = srcIO->size();
    int start_y_offset = 0, start_x_offset = 0;
    int pixels, dst_offset, src_offset;
    int pixel_width, pixel_height;

    if(image->height < info->height){
        start_y_offset = (info->height - image->height) / 2;
    }

    if(image->width < info->width){
        start_x_offset = (info->width - image->width) / 2;
    }

    offset_x = offset_x + info->width > image->width ? image->width - info->width : offset_x;
    offset_y = offset_y + info->height > image->height ? image->height - info->height : offset_y;

    offset_x = offset_x < 0 ? 0 : offset_x;
    offset_y = offset_y < 0 ? 0 : offset_y;

    pixel_width = (image->width - offset_x) <= info->width ? (image->width - offset_x) : info->width;
    pixel_height = (image->height - offset_y) <= info->height ? (image->height - offset_y) : info->height;

    LOGD("info->format:%d, dstSize:%d, srcSize:%d, info(%d, %d)", info->format, dstSize, srcSize, info->width, info->height);
    LOGI("base:%p, width:%d, height:%d, stride:%d", image->base, image->width, image->height, image->stride);
    LOGI("offset1(%d, %d)", start_x_offset, start_y_offset);
    LOGI("offset2(%d, %d)", offset_x, offset_y);
    LOGI("pixel(%d, %d)", (image->width - offset_x), (image->height - offset_y));
    LOGI("draw pixel(%d, %d)", pixel_width, pixel_height);

    for (int i = offset_y; i < pixel_height + offset_y; ++i){
        for (int j = offset_x; j < pixel_width + offset_x; ++j){
            dst_offset = (j - offset_x + (start_y_offset + i - offset_y) * info->height + start_x_offset) * dstSize;
            src_offset = (i * image->width + j) * srcSize;
            dstIO->write(bitmapPtr + dst_offset, srcIO->read(image->base + src_offset));
        }
    }
    LOGI("draw2Bitmap finished");
}

void draw2Bitmap(uint8_t *bitmapPtr, Image* image, AndroidBitmapInfo* info){
    draw2Bitmap_offset(bitmapPtr, image, info, 0, 0);
}


namespace {
class RGBA8888_IO : public PixelIO {
public:
    int size() const {
        return 4;
    }

    Pixel read(uint8_t *buf) const {
        Pixel v;
        Copy4(&v, buf);
        return v;
    }

    void write(uint8_t *buf, Pixel data) const {
        Copy4(buf, &data);
    }
};

class RGBA4444_IO : public PixelIO {
public:
    int size() const {
        return 2;
    }

    Pixel read(uint8_t *buf) const {
        struct {
            uint16_t a : 4;
            uint16_t g : 4;
            uint16_t b : 4;
            uint16_t r : 4;
        } data;
        Copy2(&data, buf);
        Pixel p;
        p.r = data.r << 4;
        p.g = data.g << 4;
        p.b = data.b << 4;
        p.a = data.a << 4;
        return p;
    }

    void write(uint8_t *buf, Pixel pix) const {
        struct {
            uint16_t a : 4;
            uint16_t g : 4;
            uint16_t b : 4;
            uint16_t r : 4;
        } data;
        data.r = pix.r >> 4;
        data.g = pix.g >> 4;
        data.b = pix.b >> 4;
        data.a = pix.a >> 4;
        Copy2(buf, &data);
    }
};

class RGB565_IO : public PixelIO {
public:
    int size() const {
        return 2;
    }

    Pixel read(uint8_t *buf) const {
        struct {
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        } data;
        Copy2(&data, buf);

        Pixel pix;
        pix.r = data.r << 3;
        pix.g = data.g << 2;
        pix.b = data.b << 3;
        return pix;
    }

    void write(uint8_t *buf, Pixel pix) const {
        struct {
            uint16_t b : 5;
            uint16_t g : 6;
            uint16_t r : 5;
        } data;
        data.r = pix.r >> 3;
        data.g = pix.g >> 2;
        data.b = pix.b >> 3;
        Copy2(buf, &data);
    }
};
}

const PixelIO* PixelIO::get(int type) {
    const static RGBA8888_IO _8888;
    const static RGBA4444_IO _4444;
    const static RGB565_IO _565;
    switch (type) {
        case PIXEL_RGBA8888:
            return &_8888;
        case PIXEL_RGBA4444:
            return &_4444;
        case PIXEL_RGB565:
            return &_565;
    }
}

inline void copy4(uint8_t *dst, uint8_t *src) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

namespace {
Pixel linear_core(int i, int j, float u, float v, Image &src, const PixelIO *io) {
    if(u >= 0.5f && src.width-i > 2)
        i++;
    if(v >= 0.5f && src.height-j > 2)
        j++;
    return io->read(src.base + src.stride * i + io->size() * j);
}

Pixel bilinear_core(int i, int j, float u, float v, Image &src, const PixelIO *io) {
    int size = io->size();
    if(v <= 0.01f && u < 0.01f) {
        return io->read(src.base + src.stride * i + io->size() * j);
    }

    uint8_t *up = src.base + i * src.stride + j * size;
    uint8_t *down = up + src.stride;
    Pixel a = io->read(up);
    Pixel b = io->read(up + size);
    Pixel c = io->read(down);
    Pixel d = io->read(down + size);

    float _u = 1-u;
    float _v = 1-v;
    float ap = _u * _v;
    float bp =  u * _v;
    float cp = _u *  v;
    float dp =  u *  v;
    /*
        j  u
      i a-----b
        |-----|
      v |--P--|
        c-----d
    */

    Pixel P;
    P.r = (uint8_t)(a.r * ap + b.r * bp + c.r * cp + d.r * dp);
    P.g = (uint8_t)(a.g * ap + b.g * bp + c.g * cp + d.g * dp);
    P.b = (uint8_t)(a.b * ap + b.b * bp + c.b * cp + d.b * dp);
    P.a = (uint8_t)(a.a * ap + b.a * bp + c.a * cp + d.a * dp);
    return P;
    }
}

void destoryImage(Image* image){
    free(image->base);
}

void resize(Image dst, Image src, int core) {
    float du = src.width / (float)dst.width;
    float dv = src.height/ (float)dst.height;

    const PixelIO *srcIO = PixelIO::get(src.pixel_type);
    const PixelIO *dstIO = PixelIO::get(dst.pixel_type);
    LOGD("[resize] dst(%d, %d, size:%d), src(%d, %d, size:%d)",
    dst.width, dst.height, dstIO->size(),
    src.width, src.height, srcIO->size());
    Pixel (*core_fp)(int, int, float, float, Image&, const PixelIO*);
    switch(core) {
        case RESIZE_NEAREST_NEIGHBOUR_INTERPOLATION:
            core_fp = linear_core;
            break;
        case RESIZE_BILINEAR_INTERPOLATION:
            core_fp = bilinear_core;
            break;
    }

    float v = 0;
    uint8_t *base = dst.base;
    int dstSize = dstIO->size();
    for(int i=0, row=0; row<dst.height; row++, v+=dv, base+=dst.stride) {
        while(v >= 1) {
            i ++;
            v -= 1;
        }
        float u = 0;
        uint8_t *cur = base;
        for(int j=0, col=0; col<dst.width; col++, u+=du, cur+=dstSize) {
            while(u >= 1) {
                j ++;
                u -= 1;
            }
            Pixel p = core_fp(i, j, u, v, src, srcIO);
            dstIO->write(cur, p);
        }
    }
}
