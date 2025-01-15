package com.tele.u8emulator;

import android.content.Context;
import android.os.Vibrator;

import org.libsdl.app.SDLActivity;

public class Game extends SDLActivity {
    public void vibrate(long milliseconds) {
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator != null && vibrator.hasVibrator()) {
            vibrator.vibrate(milliseconds);
        }
    }

    // 这是JNI将调用的静态方法
    public static void nativeVibrate(long milliseconds) {
        // 获取当前Activity的实例并调用vibrate方法
        ((Game) SDLActivity.mSingleton).vibrate(milliseconds);
    }
}
