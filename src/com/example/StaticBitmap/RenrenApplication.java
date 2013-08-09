package com.example.StaticBitmap;

import android.app.Application;

/**
 * Created by demor on 13-7-22.
 */
public class RenrenApplication extends Application {
    static Application sApp;

    @Override
    public void onCreate() {
        super.onCreate();
        sApp = this;
    }

    public static Application getGlobalContext(){
        return sApp;
    }
}
