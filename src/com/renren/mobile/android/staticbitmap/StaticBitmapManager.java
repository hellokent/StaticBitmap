package com.renren.mobile.android.staticbitmap;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.text.TextUtils;
import com.example.StaticBitmap.RenrenApplication;

import java.io.File;

import static com.renren.mobile.android.staticbitmap.Utils.downloadData;
import static com.renren.mobile.android.staticbitmap.Utils.writeBytes2File;

public enum StaticBitmapManager {
    PROFILE(100, 100),
    CHAT_BACKGROUND(200, 200);

    public static final int NET_URL = 1;
    public static final int LOCAL_SD_PATH = 2;
    public static final int ASSET = 3;

    static final int HANDLER_PRELOAD = 1;
    static final int HANDLER_LOAD = 2;

    final HandlerThread kStaticBitmapThread = new HandlerThread("static_bitmap_thread");
    final Handler kStaticBitmapHandler = new Handler(kStaticBitmapThread.getLooper()){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            StaticBitmapRequest request = (StaticBitmapRequest) msg.obj;
            switch (msg.what){
                case HANDLER_PRELOAD:
                    request.onPreload();
                    final String cacheFile = preload(request);
                    if (TextUtils.isEmpty(cacheFile)){
                        request.onLoadFailed();
                        return;
                    }
                    request.mCacheFilePath = cacheFile;

                    Message message = kStaticBitmapHandler.obtainMessage();
                    message.what = HANDLER_LOAD;
                    message.obj = request;
                    message.sendToTarget();
                    break;
                case HANDLER_LOAD:

                    break;
                default:
            }
        }
    };

    final String kOriginImageFolderName;
    final String kCacheFolderName;
    final StaticBitmapJni kJni;
    final int kWidth;
    final int kHeight;


    public File getOriginFile(String url, Context context){
        return new File(context.getCacheDir().getAbsolutePath() + "/" + kOriginImageFolderName, Utils.toMD5(url));
    }

    public File getCacheFile(String url, Context context){
        return new File(context.getCacheDir().getAbsolutePath() + "/" + kCacheFolderName, Utils.toMD5(url));
    }

    public String preload(final StaticBitmapRequest request){
        if (request== null){
            return null;
        }
        final String url = request.getUrl();
        final int type = request.getLoadType();

        final Context context = RenrenApplication.getGlobalContext();
        final File originFile = getOriginFile(url, context);
        if (originFile.exists()){
            return originFile.getAbsolutePath();
        }
        switch (type){
            case NET_URL:
                //download image
                if (!writeBytes2File(downloadData(url, context), originFile)){
                    return null;
                }
                break;
            case LOCAL_SD_PATH:
                if (!Utils.copyFile(new File(url), originFile)){
                    return null;
                }
                break;
            case ASSET:
                if (!Utils.copyFile(new File("file:///android_assets/" + url), originFile)){
                    return null;
                }
                break;
            default:
                return null;
        }
        final String cacheFile = getCacheFile(url, context).getAbsolutePath();
        // TODO
//        kJni.convert(context, );
        return cacheFile;
    }

    StaticBitmapManager(int width, int height){
        kOriginImageFolderName = this.name().toLowerCase();
        new File(RenrenApplication.getGlobalContext().getCacheDir(), kOriginImageFolderName).mkdirs();
        kCacheFolderName = kOriginImageFolderName + "/cache";
        final File cacheFile = new File(RenrenApplication.getGlobalContext().getCacheDir(), kCacheFolderName);
        cacheFile.mkdirs();
        kWidth = width;
        kHeight = height;
        kJni = new StaticBitmapJni();
    }

    public void loadBitmap(final StaticBitmapRequest callback){
        assert callback != null;
        Message msg = kStaticBitmapHandler.obtainMessage();
        msg.what = HANDLER_PRELOAD;
        msg.arg1 = callback.getLoadType();
        msg.obj = callback;
        msg.sendToTarget();
    }
}
