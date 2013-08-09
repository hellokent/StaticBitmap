package com.renren.mobile.android.staticbitmap;

import com.example.StaticBitmap.BuildConfig;

import static android.util.Log.*;
import static java.lang.String.format;

public class S {
    static final String TAG = "cylog_bitmap";

    private S() {
    }

    public static void e(String msg, Object... args) {
        if (!BuildConfig.DEBUG){
            return;
        }
        log(ERROR, format(msg, args));
    }

    public static void w(String msg, Object... args) {
        if (!BuildConfig.DEBUG){
            return;
        }
        log(WARN, format(msg, args));
    }

    public static void i(String msg, Object... args) {
        if (!BuildConfig.DEBUG){
            return;
        }
        log(INFO, format(msg, args));
    }

    public static void d(String msg, Object... args) {
        if (!BuildConfig.DEBUG){
            return;
        }
        log(DEBUG, format(msg, args));
    }

    public static void v(String msg, Object... args) {
        if (!BuildConfig.DEBUG){
            return;
        }
        log(VERBOSE, format(msg, args));
    }

    static void log(int level, String msg) {
        if (!BuildConfig.DEBUG){
            return;
        }
        println(level, TAG, msg);
    }
}
