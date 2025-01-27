package com.tele.u8emulator;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.os.Vibrator;
import android.widget.Toast;
import android.database.Cursor;
import android.provider.DocumentsContract;

import org.libsdl.app.SDLActivity;

public class Game extends SDLActivity {
    private static String lastSelectedPath = "";
    
    public void vibrate(long milliseconds) {
        Vibrator vibrator = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        if (vibrator != null && vibrator.hasVibrator()) {
            vibrator.vibrate(milliseconds);
        }
    }

    public static void nativeVibrate(long milliseconds) {
        ((Game) SDLActivity.mSingleton).vibrate(milliseconds);
    }

    private String getRealPathFromURI(Uri uri) {
        String filePath = "";
        if (DocumentsContract.isDocumentUri(this, uri)) {
            String documentId = DocumentsContract.getDocumentId(uri);
            if ("com.android.externalstorage.documents".equals(uri.getAuthority())) {
                String[] split = documentId.split(":");
                if (split.length >= 2) {
                    String type = split[0];
                    String relativePath = split[1];
                    if ("primary".equalsIgnoreCase(type)) {
                        return Environment.getExternalStorageDirectory() + "/" + relativePath;
                    }
                }
            }
        }
        
        String[] proj = { android.provider.MediaStore.Images.Media.DATA };
        Cursor cursor = getContentResolver().query(uri, proj, null, null, null);
        if (cursor != null) {
            int column_index = cursor.getColumnIndexOrThrow(android.provider.MediaStore.Images.Media.DATA);
            cursor.moveToFirst();
            filePath = cursor.getString(column_index);
            cursor.close();
        }
        return filePath;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions, int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if (requestCode == 1234) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }
            if (!allGranted) {
                Toast.makeText(this, "Storage permissions are required", Toast.LENGTH_LONG).show();
            }
        }
    }
    
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            if (uri != null) {
                lastSelectedPath = getRealPathFromURI(uri);
            }
        }

        if (requestCode == 1234) {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
                if (!Environment.isExternalStorageManager()) {
                    Toast.makeText(this, "Storage permissions are required", Toast.LENGTH_LONG).show();
                }
            }
        }
    }

    public static String getLastSelectedPath() {
        return lastSelectedPath;
    }
}