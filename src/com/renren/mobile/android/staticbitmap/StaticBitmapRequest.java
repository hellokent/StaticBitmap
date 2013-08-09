package com.renren.mobile.android.staticbitmap;

import android.graphics.Bitmap;
import android.graphics.Rect;

/**
 * Created by demor on 13-7-24.
 */
public abstract class StaticBitmapRequest {
    final String kUrl;
    final int kLoadType;
    String mCacheFilePath = null;
    final Rect kRect;

    public StaticBitmapRequest(String url, int loadType, final Rect rect){
        kUrl = url;
        kLoadType = loadType;
        kRect = rect;
    }

    public String getUrl() {
        return kUrl;
    }

    public int getLoadType() {
        return kLoadType;
    }

    public abstract void onPreload();
    public abstract void onLoadFinshed(final Bitmap bitmap);
    public abstract void onLoadFailed();
}
