package com.tele.u8emulator;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity; 
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Build;
import android.os.Environment;
import android.widget.Toast;
import android.database.Cursor;
import android.provider.DocumentsContract;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;

public class SystemDialogs {
    private static final int PERMISSION_REQUEST_CODE = 1234;
    private static final String[] PERMISSIONS = {
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE
    };

    private static boolean checkPermissions(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            return Environment.isExternalStorageManager();
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            for (String permission : PERMISSIONS) {
                if (ContextCompat.checkSelfPermission(activity, permission) 
                    != PackageManager.PERMISSION_GRANTED) {
                    return false;
                }
            }
        }
        return true;
    }

    private static void requestPermissions(Activity activity) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            try {
                Intent intent = new Intent(android.provider.Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                intent.addCategory("android.intent.category.DEFAULT");
                intent.setData(Uri.parse(String.format("package:%s", activity.getPackageName())));
                activity.startActivityForResult(intent, PERMISSION_REQUEST_CODE);
            } catch (Exception e) {
                Intent intent = new Intent();
                intent.setAction(android.provider.Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                activity.startActivityForResult(intent, PERMISSION_REQUEST_CODE);
            }
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            ActivityCompat.requestPermissions(activity, PERMISSIONS, PERMISSION_REQUEST_CODE);
        }
    }

    // Helper method to get real path from URI
    private static String getRealPathFromURI(Activity activity, Uri uri) {
        String filePath = "";
        if (DocumentsContract.isDocumentUri(activity, uri)) {
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
        Cursor cursor = activity.getContentResolver().query(uri, proj, null, null, null);
        if (cursor != null) {
            int column_index = cursor.getColumnIndexOrThrow(android.provider.MediaStore.Images.Media.DATA);
            cursor.moveToFirst();
            filePath = cursor.getString(column_index);
            cursor.close();
        }
        return filePath;
    }

    public static String openFileDialog(Activity activity) {
        if (!checkPermissions(activity)) {
            requestPermissions(activity);
            return "";
        }

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        activity.startActivityForResult(intent, 1);
        return "";
    }

    public static String saveFileDialog(Activity activity, String preferredName) {
        if (!checkPermissions(activity)) {
            requestPermissions(activity);
            return "";
        }

        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType("*/*");
        intent.addCategory(Intent.CATEGORY_OPENABLE);
        intent.putExtra(Intent.EXTRA_TITLE, preferredName);
        activity.startActivityForResult(intent, 2);
        return "";
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static String openFolderDialog(Activity activity) {
        if (!checkPermissions(activity)) {
            requestPermissions(activity);
            return "";
        }

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        activity.startActivityForResult(intent, 3);
        return "";
    }

    public static String saveFolderDialog(Activity activity) {
        if (!checkPermissions(activity)) {
            requestPermissions(activity);
            return "";
        }

        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        activity.startActivityForResult(intent, 4);
        return "";
    }
}