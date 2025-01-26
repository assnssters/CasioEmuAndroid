package com.tele.u8emulator;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.widget.Toast;

public class SystemDialogs {
    public static String openFileDialog(Activity activity) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
        intent.setType("*/*");
        activity.startActivityForResult(intent, 1);
        return "";
    }

    public static String saveFileDialog(Activity activity, String preferredName) {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType("*/*");
        intent.putExtra(Intent.EXTRA_TITLE, preferredName);
        activity.startActivityForResult(intent, 2);
        return "";
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP)
    public static String openFolderDialog(Activity activity) {
        Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT_TREE);
        activity.startActivityForResult(intent, 3);
        return "";
    }

    public static String saveFolderDialog(Activity activity) {
        Intent intent = new Intent(Intent.ACTION_CREATE_DOCUMENT);
        intent.setType("vnd.android.document/directory");
        activity.startActivityForResult(intent, 4);
        return "";
    }
}