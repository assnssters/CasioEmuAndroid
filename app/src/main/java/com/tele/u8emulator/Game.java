package com.tele.u8emulator;

import android.content.Context;
import android.os.Vibrator;
import android.content.Intent;
import android.app.Activity;
import android.net.Uri;
import android.database.Cursor;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;

import org.libsdl.app.SDLActivity;

public class Game extends SDLActivity {
    private static native void onFileSelected(String path);
    private static native void onFileSaved(String path);
    private static native void onFolderSelected(String path);
    private static native void onFolderSaved(String path);

    public void vibrate(long milliseconds) {
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator != null && vibrator.hasVibrator()) {
            vibrator.vibrate(milliseconds);
        }
    }

    public static void nativeVibrate(long milliseconds) {
        ((Game) SDLActivity.mSingleton).vibrate(milliseconds);
    }

    private String getPathFromUri(Uri uri) {
        String path = uri.toString();
        if (DocumentsContract.isDocumentUri(this, uri)) {
            try (Cursor cursor = getContentResolver().query(
                    uri,
                    new String[]{OpenableColumns.DISPLAY_NAME},
                    null, null, null)) {
                if (cursor != null && cursor.moveToFirst()) {
                    String displayName = cursor.getString(
                            cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
                    path = displayName;
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return path;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (resultCode == Activity.RESULT_OK && data != null && data.getData() != null) {
            String path = getPathFromUri(data.getData());
            
            switch (requestCode) {
                case 1: // Open File
                    onFileSelected(path);
                    break;
                case 2: // Save File
                    onFileSaved(path);
                    break;
                case 3: // Open Folder
                    onFolderSelected(path);
                    break;
                case 4: // Save Folder
                    onFolderSaved(path);
                    break;
            }
        }
    }
}