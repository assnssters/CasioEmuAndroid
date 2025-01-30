package com.tele.u8emulator;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.os.Build;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import java.util.ArrayList;
import java.util.List;

public class PermissionManager {
    public static final int PERMISSION_REQUEST_CODE = 123;

    private static final String[] REQUIRED_PERMISSIONS = new String[] {
        Manifest.permission.READ_EXTERNAL_STORAGE,
        Manifest.permission.WRITE_EXTERNAL_STORAGE,
        Build.VERSION.SDK_INT >= Build.VERSION_CODES.R ? 
            Manifest.permission.MANAGE_EXTERNAL_STORAGE : null
    };

    public static boolean checkAndRequestPermissions(Activity activity) {
        List<String> permissionsNeeded = new ArrayList<>();
        
        for (String permission : REQUIRED_PERMISSIONS) {
            if (permission != null && ContextCompat.checkSelfPermission(activity, permission) 
                != PackageManager.PERMISSION_GRANTED) {
                permissionsNeeded.add(permission);
            }
        }

        if (!permissionsNeeded.isEmpty()) {
            ActivityCompat.requestPermissions(activity,
                permissionsNeeded.toArray(new String[0]),
                PERMISSION_REQUEST_CODE);
            return false;
        }
        
        return true;
    }

    public static boolean hasAllPermissions(Activity activity) {
        for (String permission : REQUIRED_PERMISSIONS) {
            if (permission != null && ContextCompat.checkSelfPermission(activity, permission) 
                != PackageManager.PERMISSION_GRANTED) {
                return false;
            }
        }
        return true;
    }
}