package com.renren.mobile.android.staticbitmap;

import android.graphics.Bitmap;
import android.graphics.Rect;
import android.os.Handler;
import android.os.Looper;

public abstract class UIStaticBitmapRequest extends StaticBitmapRequest {
    public final Handler kUiHandler = new Handler(Looper.getMainLooper());

    public UIStaticBitmapRequest(String url, int loadType, Rect rect) {
        super(url, loadType, rect);
    }

    public abstract void onUIPreload();
    public abstract void onUILoadFinshed(final Bitmap bitmap);
    public abstract void onUILoadFailed();

    @Override
    public final void onPreload() {
        kUiHandler.post(new Runnable() {
            @Override
            public void run() {
                onUIPreload();
            }
        });
    }

    @Override
    public final void onLoadFinshed(final Bitmap bitmap) {
        kUiHandler.post(new Runnable() {
            @Override
            public void run() {
                onUILoadFinshed(bitmap);
            }
        });
    }

    @Override
    public final void onLoadFailed() {
        kUiHandler.post(new Runnable() {
            @Override
            public void run() {
                onUILoadFailed();
            }
        });
    }
}
