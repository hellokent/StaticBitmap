// standard c&c++
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// java wrapper
#include <jni.h>

// android
#include <android/log.h>
#include <android/bitmap.h>
#include "decode.h"

//linux
#include <sys/mman.h>

#define I(NAME) Java_##com_renren_mobile_android_staticbitmap_StaticBitmapJni##_##NAME
extern "C"{
/**
int drawRegion2Bitmap(Bitmap bitmap, String filePath, String cacheFilePath, boolean needResize,
                                        int topx, int topy, int btmx, int btmy);
int map(String filePath, String cacheFilePath, boolean needResize,
                                int topx, int topy, int btmx, int btmy);
int getWidth(int ptr);
int getHeight(int ptr);
int flushData(int ptr, Bitmap bitmap, int topx, int topy);
int unmap(int ptr);
*/
JNIEXPORT jint JNICALL I(drawRegion2Bitmap)(JNIEnv *, jobject, jobject,
    jstring, jstring, jboolean, jint, jint, jint, jint);

JNIEXPORT jint JNICALL I(map)(JNIEnv *, jobject
    jstring, jstring, jboolean, jint, jint, jint, jint);

JNIEXPORT jint JNICALL I(getWidth)(JNIEnv *, jobject, jint);

JNIEXPORT jint JNICALL I(getHeight)(JNIEnv *, jobject, jint);

JNIEXPORT jint JNICALL I(flushData)(JNIEnv *, jobject, jint, jobject, jint, jint);

JNIEXPORT jint JNICALL I(unmap)(JNIEnv *, jobject, jint);
}

inline void copy4(uint8_t *dst, uint8_t *src) {
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

jint JNICALL I(drawRegion2Bitmap)(JNIEnv * env, jobject object, jobject bitmap,
                                    jstring jpath, jstring jcachePath, jboolean need_resize,
                                    jint top_x, jint top_y, jint btm_x, jint btm_y){
    uint8_t *ptr = 0;
	Image src_image;
    char const *path = env->GetStringUTFChars(jpath, 0);
	char const *cache_path = env->GetStringUTFChars(jcachePath, 0);

    //1. decode
	LOGI("jpath=%s, jcachepath=%s", path, cache_path);
	FILE* fp = fopen(path, "rb");
	if (!fp){
		LOGE("open file %s error", path);
		LOGERRNO();
		return (jint)0;
	}
	int result = decode2RGBA(fp, cache_path, &src_image);
	env->ReleaseStringUTFChars(jpath, path);
    env->ReleaseStringUTFChars(jcachePath, cache_path);
	LOGD("decode2RGBA.result:%d", result);
	if (result){
		LOGD("decode success, width=%d, height=%d", src_image.width, src_image.height);
	}else{
		LOGD("decode2RGBA failed");
	}
	LOGI("release string");
	fclose(fp);

	//2. get bitmap info
    AndroidBitmapInfo info = {0,0,0,0,0};
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, (void**)&ptr);
	if(!ptr) {
		LOGE("lock bitmap failed");
		return (jint) 0;
	}

	//3. get region pixels
	if(need_resize == JNI_TRUE){
        resizeRegion(&src_image, top_x, top_y, btm_x, btm_y);
	}

	//4. draw to bitmap
    draw2Bitmap(ptr, &src_image, &info);
	LOGD("draw2Bitmap finished");
	AndroidBitmap_unlockPixels(env, bitmap);
	LOGD("unlock Pixels finshed");

	//5. release resources
	destoryImage(&src_image);
	LOGD("destory Image");
	return (jint)1;
}

// struct
struct map_t {
	size_t size;
	int width;
	int height;
	char *ptr;
};

JNIEXPORT jint JNICALL I(map)(JNIEnv * env, jobject object,
                                jstring jpath, jstring jcachePath, jboolean need_resize,
                                jint top_x, jint top_y, jint btm_x, jint btm_y){
    uint8_t *ptr = 0;
    Image src_image;
    map_t *mpt;
    char const *path = env->GetStringUTFChars(jpath, 0);
    char const *cache_path = env->GetStringUTFChars(jcachePath, 0);

    //1. decode
    LOGI("jpath=%s, jcachepath=%s", path, cache_path);
    FILE* fp = fopen(path, "rb");
    if (!fp){
        LOGE("open file %s error", path);
        LOGERRNO();
        return (jint)0;
    }
    int result = decode2RGBA(fp, cache_path, &src_image);
    env->ReleaseStringUTFChars(jpath, path);
    env->ReleaseStringUTFChars(jcachePath, cache_path);
    LOGD("decode2RGBA.result:%d", result);
    if (result){
        LOGD("decode success, width=%d, height=%d", src_image.width, src_image.height);
    }else{
        LOGD("decode2RGBA failed");
    }
    LOGI("release string");
    fclose(fp);

//    //2. get bitmap info
//    AndroidBitmapInfo info = {0,0,0,0,0};
//    AndroidBitmap_getInfo(env, bitmap, &info);
//    AndroidBitmap_lockPixels(env, bitmap, (void**)&ptr);
//    if(!ptr) {
//        LOGE("lock bitmap failed");
//        return (jint) 0;
//    }

    //3. get region pixels
    if(need_resize == JNI_TRUE){
        resizeRegion(&src_image, top_x, top_y, btm_x, btm_y);
    }

    //Now, we set image data into src_image
    mpt = new map_t;
    mpt->width = src_image->width;
    mpt->height = src_image->height;
    //ashmem
    mpt->ptr = mmap(NULL, src_image->stride * src_image->height, PROT_READ, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (!mpt->ptr){
        LOGE("mmap failed");
        LOGERRNO();
        return (jint)-1;
    }
    LOGI("mmap success!!!, ptr:%p", mpt->ptr);
    memcpy()
    return (jint)mpt;
}

JNIEXPORT jint JNICALL I(getWidth)(JNIEnv *env, jobject object, jint ptr){
	map_t *mpt = (map_t*)ptr;
	if(!mpt) {
		LOGE("get width of null pointer");
		return -1;
	}
	return (jint)mpt->width;
}

JNIEXPORT jint JNICALL I(getHeight)(JNIEnv *env, jobject object, jint ptr){
	map_t *mpt = (map_t*)ptr;
	if(!mpt) {
		LOGE("get height of null pointer");
		return -1;
	}
	return (jint)mpt->height;
}

JNIEXPORT jint JNICALL I(flushData)(JNIEnv *env, jobject, jint, jobject, jint, jint);

JNIEXPORT jint JNICALL I(unmap)(JNIEnv *, jobject, jint);