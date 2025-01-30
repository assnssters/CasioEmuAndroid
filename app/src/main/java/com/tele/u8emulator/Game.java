package com.tele.u8emulator;

import android.content.Context;
import android.os.Vibrator;
import android.content.Intent;
import android.app.Activity;
import android.net.Uri;
import android.database.Cursor;
import android.provider.DocumentsContract;
import android.provider.OpenableColumns;
import android.content.pm.PackageManager;
import android.util.Log;
import org.libsdl.app.SDLActivity;

public class Game extends SDLActivity {
    private static final String TAG = "Game";
    private static Uri pendingUri = null;
    private static byte[] pendingData = null;
    private static int pendingRequestCode = -1;
    
    private static native void onFileSelected(String path, byte[] data);
    private static native void onFileSaved(String path);
    private static native void onFolderSelected(String path);
    private static native void onFolderSaved(String path);
    private static native void onExportFailed();
    private static native void onImportFailed();

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
                    int columnIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    if (columnIndex != -1) {
                        path = cursor.getString(columnIndex);
                    }
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return path;
    }

    public static void exportData(byte[] data, Uri uri) {
        Activity activity = SDLActivity.mSingleton;
        if (activity == null) return;

        Game game = (Game)activity;
        if (!PermissionManager.hasAllPermissions(game)) {
            pendingUri = uri;
            pendingData = data;
            PermissionManager.checkAndRequestPermissions(game);
            return;
        }

        boolean success = FileUtils.writeToUri(activity, uri, data);
        if (!success) {
            onExportFailed();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                         String[] permissions,
                                         int[] grantResults) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        
        if (requestCode == PermissionManager.PERMISSION_REQUEST_CODE) {
            boolean allGranted = true;
            for (int result : grantResults) {
                if (result != PackageManager.PERMISSION_GRANTED) {
                    allGranted = false;
                    break;
                }
            }

            if (allGranted) {
                if (pendingUri != null && pendingData != null) {
                    exportData(pendingData, pendingUri);
                } else if (pendingRequestCode != -1) {
                    handlePendingRequest();
                }
            } else {
                onExportFailed();
            }
            
            pendingUri = null;
            pendingData = null;
            pendingRequestCode = -1;
        }
    }

    private void handlePendingRequest() {
        if (pendingRequestCode != -1 && pendingUri != null) {
            String path = getPathFromUri(pendingUri);
            
            switch (pendingRequestCode) {
                case 1: // Open File
                    byte[] fileData = FileUtils.readFromUri(this, pendingUri);
                    if (fileData != null) {
                        onFileSelected(path, fileData);
                    } else {
                        onImportFailed();
                    }
                    break;
                case 2: // Save File
                    onFileSaved(pendingUri.toString());
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

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        
        if (resultCode == Activity.RESULT_OK && data != null && data.getData() != null) {
            Uri uri = data.getData();
            Log.d(TAG, "File URI: " + uri.toString());
            
            // Take persistable permission
            try {
                final int takeFlags = Intent.FLAG_GRANT_READ_URI_PERMISSION |
                                    Intent.FLAG_GRANT_WRITE_URI_PERMISSION;
                getContentResolver().takePersistableUriPermission(uri, takeFlags);
            } catch (Exception e) {
                Log.e(TAG, "Failed to take persistable permission: " + e.getMessage());
            }

            if (!PermissionManager.hasAllPermissions(this)) {
                pendingUri = uri;
                pendingRequestCode = requestCode;
                PermissionManager.checkAndRequestPermissions(this);
                return;
            }

            String path = getPathFromUri(uri);
            Log.d(TAG, "File path: " + path);
            
            switch (requestCode) {
                case 1: // Open File
                    byte[] fileData = FileUtils.readFromUri(this, uri);
                    if (fileData != null && fileData.length > 0) {
                        Log.d(TAG, "Read file data: " + fileData.length + " bytes");
                        onFileSelected(path, fileData);
                    } else {
                        Log.e(TAG, "Failed to read file data");
                        onImportFailed();
                    }
                    break;
                case 2: // Save File
                    onFileSaved(uri.toString());
                    break;
                case 3: // Open Folder
                    onFolderSelected(path);
                    break;
                case 4: // Save Folder
                    onFolderSaved(path);
                    break;
            }
        } else {
            Log.w(TAG, "Activity result failed or no data");
        }
    }
}