package com.example.StaticBitmap;

import android.app.Activity;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import com.renren.mobile.android.staticbitmap.StaticBitmapJni;

public class MainActivity extends Activity {
    StaticBitmapJni jni = new StaticBitmapJni();
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        ImageView imageView = (ImageView)findViewById(R.id.image1);
        long startTime = System.currentTimeMillis();
        Bitmap bitmap = Bitmap.createBitmap(200, 200, Bitmap.Config.RGB_565);
//        jni.drawFile2Bitmap(bitmap, "/sdcard/demo3.jpg");
//        Rect rect = new Rect();
//        rect.left = rect.top = 0;
//        rect.right = rect.bottom = 100;
//        jni.drawRegionFile2Bitmap(bitmap, "/sdcard/demo3.jpg", rect);
	    int ptr = jni.map("/sdcard/demo3.jpg");
	    jni.flushData(ptr, bitmap, 0, 0);
        Log.i("cylog_bitmap", "use time:" + (System.currentTimeMillis() - startTime));
        imageView.setImageBitmap(bitmap);
	    jni.unmap(ptr);

	    imageView = (ImageView)findViewById(R.id.image2);
	    startTime = System.currentTimeMillis();
	    Bitmap bitmap1 = Bitmap.createBitmap(200, 200, Bitmap.Config.RGB_565);
//        jni.drawFile2Bitmap(bitmap, "/sdcard/demo3.jpg");
//        Rect rect = new Rect();
//        rect.left = rect.top = 0;
//        rect.right = rect.bottom = 100;
//        jni.drawRegionFile2Bitmap(bitmap, "/sdcard/demo3.jpg", rect);
	    ptr = jni.map("/sdcard/demo3.jpg");
	    jni.flushData(ptr, bitmap1, 50, 50);
	    Log.i("cylog_bitmap", "use time:" + (System.currentTimeMillis() - startTime));
	    imageView.setImageBitmap(bitmap1);
	    jni.unmap(ptr);

	    imageView = (ImageView)findViewById(R.id.image3);
	    startTime = System.currentTimeMillis();
	    Bitmap bitmap2 = Bitmap.createBitmap(200, 200, Bitmap.Config.RGB_565);
//        jni.drawFile2Bitmap(bitmap, "/sdcard/demo3.jpg");
//        Rect rect = new Rect();
//        rect.left = rect.top = 0;
//        rect.right = rect.bottom = 100;
//        jni.drawRegionFile2Bitmap(bitmap, "/sdcard/demo3.jpg", rect);
	    ptr = jni.map("/sdcard/demo3.jpg");
	    jni.flushData(ptr, bitmap2, 100, 100);
	    Log.i("cylog_bitmap", "use time:" + (System.currentTimeMillis() - startTime));
	    imageView.setImageBitmap(bitmap2);
	    jni.unmap(ptr);
    }
}
