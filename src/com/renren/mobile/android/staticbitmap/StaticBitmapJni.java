package com.renren.mobile.android.staticbitmap;

import android.graphics.Bitmap;
import android.graphics.Rect;
import com.example.StaticBitmap.RenrenApplication;

import java.io.File;

public class StaticBitmapJni {

    static final String CACHE_FILE;

    static {
        System.loadLibrary("static_bitmap");
        CACHE_FILE = RenrenApplication.getGlobalContext().getCacheDir().getAbsolutePath() + File.separatorChar + "img_cache";
        final File cache = new File(CACHE_FILE);
        cache.mkdirs();
    }

	//draw系方法
    private native int drawRegion2Bitmap(Bitmap bitmap, String filePath, String cacheFilePath, boolean needResize,
                                        int topx, int topy, int btmx, int btmy);

    public int drawFile2Bitmap(Bitmap bitmap, String filePath){
        return drawRegion2Bitmap(bitmap, filePath, CACHE_FILE, false, 0, 0, 0, 0);
    }

    /**
     * 下图是几个坐标的选取
     * *----------------------*
     * |                      |
     * |                      |
     * |     {topy}           |
     * |{topx} *------*       |
     * |       |      |       |
     * |       |      |       |
     * |       |      |       |
     * |       |      |       |
     * |       *------*{btmx} |
     * |            {btmy}    |
     * |                      |
     * |                      |
     * |                      |
     * *----------------------*
     */
    public int drawRegionFile2Bitmap(Bitmap bitmap, String filePath, Rect rect){
        return drawRegion2Bitmap(bitmap, filePath, CACHE_FILE, true, rect.left, rect.top, rect.right, rect.bottom);
    }

	public int map(String filePath){
		return map(filePath, CACHE_FILE, false, 0, 0, 0, 0);
	}

	public native int map(String filePath, String cacheFilePath, boolean needResize,
	                                int topx, int topy, int btmx, int btmy);

	public native int getWidth(int ptr);

	public native int getHeight(int ptr);

	public native int flushData(int ptr, Bitmap bitmap, int topx, int topy);

	public native int unmap(int ptr);
}
