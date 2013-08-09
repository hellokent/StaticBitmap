package com.renren.mobile.android.staticbitmap;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Pair;
import org.apache.http.HttpHost;
import org.apache.http.HttpResponse;
import org.apache.http.client.HttpClient;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.conn.params.ConnRouteParams;
import org.apache.http.impl.client.DefaultHttpClient;

import java.io.*;
import java.nio.channels.FileChannel;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import static java.util.concurrent.TimeUnit.SECONDS;
import static org.apache.http.params.CoreConnectionPNames.CONNECTION_TIMEOUT;
import static org.apache.http.params.CoreConnectionPNames.SO_TIMEOUT;

/**
 * Created by demor on 13-7-22.
 */
public final class Utils {
    public static HttpHost getProxyHost(final Context context){
        final ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
        final NetworkInfo ani = cm.getActiveNetworkInfo();
        HttpHost proxy = null;
        if (ani == null || !ani.isConnected()) {
            S.v("no network");
        } else {
            String apnName = ani.getExtraInfo();

            proxy = null;
            if (apnName != null) {
                S.v("apn: %s", apnName);
                String apn = apnName.toLowerCase();
                if (apn.equals("cmwap") || apn.equals("uniwap") || apn.equals("3gwap")) {
                    proxy = new HttpHost("10.0.0.172", 80, "http");
                } else if (apn.equals("ctwap")) {
                    proxy = new HttpHost("10.0.0.200", 80, "http");
                }
            }
            S.v("proxy: %s", proxy);
        }
        return proxy;
    }

    public static byte[] downloadData(final String url, final Context context){
        try {
            HttpClient client = new DefaultHttpClient();
            HttpHost proxy = getProxyHost(context);
            if (proxy != null) {
                client.getParams().setParameter(ConnRouteParams.DEFAULT_PROXY, proxy);
                S.v("use proxy:%s", proxy);
            }
            client.getParams().setIntParameter(CONNECTION_TIMEOUT, (int) SECONDS.toMillis(30));
            client.getParams().setIntParameter(SO_TIMEOUT, (int) SECONDS.toMillis(300));

            long start = System.currentTimeMillis();
            HttpGet get = new HttpGet(url);

            HttpResponse response = client.execute(get);
            long med = System.currentTimeMillis();

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            response.getEntity().writeTo(bos);
            byte[] data = bos.toByteArray();

            long end = System.currentTimeMillis();
            S.v("GET %d bytes consuming %d ms, (%d, %d), url:%s",
                    data.length, end - start, med - start, end - med, url);
            S.v("finish download");
            return data;
        } catch (Exception e) {
            return null;
        }
    }

    public static String toMD5(String s) {
        if (s == null) {
            return null;
        }
        try {
            byte[] bs = s.getBytes("UTF-8");
            MessageDigest md5 = MessageDigest.getInstance("MD5");
            md5.update(bs);
            byte[] result = md5.digest();
            return new String(result);
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return null;
    }

    public static boolean writeBytes2File(final byte[] bytes, final File file){
        if (bytes == null){
            return false;
        }
        FileOutputStream fos = null;
        try {
            if (file.exists()){
                file.delete();
                file.createNewFile();
            }
            fos = new FileOutputStream(file);
            fos.write(bytes);
            return true;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            quietClose(fos);
        }
        return false;
    }

    public static boolean copyFile(final File src, final File dst){
        if (src == null || !src.exists()){
            return false;
        }
        dst.mkdirs();
        dst.delete();
        FileInputStream fis = null;
        FileChannel inputChannel = null;
        FileOutputStream fos = null;
        FileChannel outputChannel = null;

        try {
            dst.createNewFile();
            fos = new FileOutputStream(dst);
            outputChannel = fos.getChannel();

            fis = new FileInputStream(src);
            inputChannel = fis.getChannel();

            inputChannel.transferTo(0, src.length(), outputChannel);

            return true;
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            quietClose(fis);
            quietClose(inputChannel);
            quietClose(fos);
            quietClose(outputChannel);
        }
        return false;
    }

    public static void quietClose(Closeable closeable){
        if (closeable == null){
            return;
        }
        try {
            closeable.close();
        } catch (IOException ignored) {
        }
    }

    static final BitmapFactory.Options GET_BITMAP_SIZE_OPTIONS = new BitmapFactory.Options(){
        {
            inJustDecodeBounds = true;
        }
    };

    public static Pair<Integer, Integer> getBitmapSize(String filePath){
        BitmapFactory.decodeFile(filePath, GET_BITMAP_SIZE_OPTIONS);
        return Pair.create(GET_BITMAP_SIZE_OPTIONS.outWidth, GET_BITMAP_SIZE_OPTIONS.outHeight);
    }
}
