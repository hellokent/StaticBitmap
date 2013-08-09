ROOT_PATH := $(call my-dir)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := static_bitmap
LOCAL_CFLAGS := -DHAVE_CONFIG_H \
	-DDEBUG \
	-DCLAZZ=\"com.renren.mobile.android.staticbitmap.StaticBitmapJni\"
LOCAL_SRC_FILES := decode.cpp jni_interface.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/jpeg $(LOCAL_PATH)/libpng

LOCAL_STATIC_LIBRARIES := jpeg png

LOCAL_LDLIBS := -llog -ljnigraphics -lz

include $(BUILD_SHARED_LIBRARY)

include $(ROOT_PATH)/libpng/Android.mk
include $(ROOT_PATH)/jpeg/Android.mk

