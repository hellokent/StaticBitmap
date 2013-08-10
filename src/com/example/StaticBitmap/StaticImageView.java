package com.example.StaticBitmap;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.text.TextUtils;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import com.renren.mobile.android.staticbitmap.S;
import com.renren.mobile.android.staticbitmap.StaticBitmapJni;

/**
 * Created by demor on 13-8-10.
 */
public class StaticImageView extends View {

	Bitmap mBitmap = null;
	int mPtr;
	int mWidth, mHeight;
	String mFilePath;
	StaticBitmapJni mJni = new StaticBitmapJni();
	Paint mPaint;


	public StaticImageView(Context context) {
		super(context);
		mPaint = new Paint();
	}

	public StaticImageView(Context context, AttributeSet attrs) {
		super(context, attrs);
	}

	public StaticImageView(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		mPaint = new Paint();
	}

	@Override
	protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
		super.onMeasure(widthMeasureSpec, heightMeasureSpec);
		mWidth = MeasureSpec.getSize(widthMeasureSpec);
		mHeight = MeasureSpec.getSize(heightMeasureSpec);
		S.i("width:%d, height:%d", mWidth, mHeight);
		if (!isBitmapValid(mBitmap)){
			S.i("begin create bitmap");
			mBitmap = Bitmap.createBitmap(mWidth, mHeight, Bitmap.Config.ARGB_8888);
			S.i("end create bitmap");
			if (!TextUtils.isEmpty(mFilePath)){
				mPtr = mJni.map(mFilePath);
				mJni.flushData(mPtr, mBitmap, mBitmapOffsetX, mBitmapOffsetY);
			}
		}
	}

	public void setFilePath(String filePath){
		if (!isBitmapValid(mBitmap)){
			mFilePath = filePath;
			return;
		}
		mPtr = mJni.map(filePath);
		mJni.flushData(mPtr, mBitmap, mBitmapOffsetX, mBitmapOffsetY);
	}

	public static boolean isBitmapValid(Bitmap bitmap){
		return bitmap != null && !bitmap.isRecycled();
	}

	int mTouchX, mTouchY;
	int mBitmapOffsetX, mBitmapOffsetY;
	int mBitmapBaseOffsetX, mBitmapBaseOffsetY;

	{
		mBitmapOffsetX = mBitmapOffsetY = 0;
		mBitmapBaseOffsetX = mBitmapBaseOffsetY = 0;
	}

	@Override
	public boolean dispatchTouchEvent(MotionEvent event) {
		final int x = (int)event.getX();
		final int y = (int)event.getY();

		switch (event.getAction()){
			case MotionEvent.ACTION_DOWN:
				mTouchX = x;
				mTouchY = y;
				break;
			case MotionEvent.ACTION_MOVE:
				final int touchOffsetX = mTouchX - x;
				final int touchOffsetY = mTouchY - y;
				mBitmapOffsetX = touchOffsetX;
				mBitmapOffsetY = touchOffsetY;
				invalidate();
				break;
			case MotionEvent.ACTION_CANCEL:
			case MotionEvent.ACTION_UP:
				mBitmapBaseOffsetX = mBitmapOffsetX;
				mBitmapBaseOffsetY = mBitmapOffsetY;
				break;
		}
		return true;
	}

	@Override
	protected void onDraw(Canvas canvas) {
		super.onDraw(canvas);
		S.w("offset:(%d, %d)", mBitmapOffsetX, mBitmapOffsetY);
		long startTime = System.currentTimeMillis();
		mJni.flushData(mPtr, mBitmap, mBitmapOffsetX + mBitmapBaseOffsetX, mBitmapOffsetY + mBitmapBaseOffsetY);
		canvas.drawBitmap(mBitmap, 0, 0, mPaint);
		S.e("after draw time:%d", System.currentTimeMillis() - startTime);
	}
}
