#include "SysDialog.h"

#ifdef _WIN32
#include <ShlObj.h>
#include <SysDialog.h>
#include <Windows.h>
#include <commdlg.h>

std::filesystem::path SystemDialogs::OpenFileDialog() {
    auto cwd = std::filesystem::current_path();
    OPENFILENAME ofn;
    TCHAR szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("All\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn) == TRUE) {
        std::filesystem::current_path(cwd);
        return std::filesystem::path(ofn.lpstrFile);
    }
    std::filesystem::current_path(cwd);
    return {};
}

std::filesystem::path SystemDialogs::SaveFileDialog(std::string prefered_name) {
    auto cwd = std::filesystem::current_path();
    OPENFILENAME ofn;
    TCHAR szFile[260] = {0};
    MultiByteToWideChar(65001, 0, prefered_name.c_str(), prefered_name.size(), szFile, 260);
    // strcpy_s(szFile, prefered_name.c_str());
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = TEXT("All\0*.*\0");
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if (GetSaveFileName(&ofn) == TRUE) {
        std::filesystem::current_path(cwd);
        return std::filesystem::path(ofn.lpstrFile);
    }
    std::filesystem::current_path(cwd);
    return {};
}

std::filesystem::path SystemDialogs::OpenFolderDialog() {
    auto cwd = std::filesystem::current_path();
    BROWSEINFO bi = {0};
    bi.lpszTitle = TEXT("Select Folder");
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    std::filesystem::current_path(cwd);
    if (pidl != 0) {
        TCHAR path[MAX_PATH];
        if (SHGetPathFromIDList(pidl, path)) {
            return std::filesystem::path(path);
        }
    }
    return {};
}

std::filesystem::path SystemDialogs::SaveFolderDialog() {
    return OpenFolderDialog(); // In Windows, folder dialogs are usually used for both opening and saving.
}
#elif defined(__ANDROID__)

#include <jni.h>
#include <SDL.h>
#include <filesystem>
#include <thread>
#include <chrono>

std::filesystem::path WaitForResult(JNIEnv* env, jobject activity) {
    // Wait for result with timeout
    for(int i = 0; i < 100; i++) { // Wait up to 10 seconds
        jclass gameCls = env->FindClass("com/tele/u8emulator/Game");
        jmethodID getLastPath = env->GetStaticMethodID(gameCls, "getLastSelectedPath", "()Ljava/lang/String;");
        jstring result = (jstring)env->CallStaticObjectMethod(gameCls, getLastPath);
        
        if(result != nullptr) {
            const char *path = env->GetStringUTFChars(result, nullptr);
            std::string pathStr = path;
            env->ReleaseStringUTFChars(result, path);
            
            if(!pathStr.empty()) {
                return std::filesystem::path(pathStr);
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return {};
}

std::filesystem::path SystemDialogs::OpenFileDialog() {
    auto env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass cls = env->FindClass("com/tele/u8emulator/SystemDialogs");
    jmethodID openFileDialogMethod = env->GetStaticMethodID(cls, "openFileDialog",
                                                      "(Landroid/app/Activity;)Ljava/lang/String;");
    
    jobject activity = (jobject)SDL_AndroidGetActivity();
    env->CallStaticObjectMethod(cls, openFileDialogMethod, activity);
    
    return WaitForResult(env, activity);
}

std::filesystem::path SystemDialogs::SaveFileDialog(std::string prefered_name) {
    auto env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass cls = env->FindClass("com/tele/u8emulator/SystemDialogs");
    jmethodID saveFileDialogMethod = env->GetStaticMethodID(cls, "saveFileDialog",
                                                      "(Landroid/app/Activity;Ljava/lang/String;)Ljava/lang/String;");
    
    jobject activity = (jobject)SDL_AndroidGetActivity();
    jstring jPreferedName = env->NewStringUTF(prefered_name.c_str());
    env->CallStaticObjectMethod(cls, saveFileDialogMethod, activity, jPreferedName);
    env->DeleteLocalRef(jPreferedName);
    
    return WaitForResult(env, activity);
}

std::filesystem::path SystemDialogs::OpenFolderDialog() {
    auto env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass cls = env->FindClass("com/tele/u8emulator/SystemDialogs");
    jmethodID openFolderDialogMethod = env->GetStaticMethodID(cls, "openFolderDialog",
                                                        "(Landroid/app/Activity;)Ljava/lang/String;");
    
    jobject activity = (jobject)SDL_AndroidGetActivity();
    env->CallStaticObjectMethod(cls, openFolderDialogMethod, activity);
    
    return WaitForResult(env, activity);
}

std::filesystem::path SystemDialogs::SaveFolderDialog() {
    auto env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    jclass cls = env->FindClass("com/tele/u8emulator/SystemDialogs");
    jmethodID saveFolderDialogMethod = env->GetStaticMethodID(cls, "saveFolderDialog",
                                                        "(Landroid/app/Activity;)Ljava/lang/String;");
    
    jobject activity = (jobject)SDL_AndroidGetActivity();
    env->CallStaticObjectMethod(cls, saveFolderDialogMethod, activity);
    
    return WaitForResult(env, activity);
}

#elif defined(__linux__)
#include <cstdlib>
#include <iostream>

std::filesystem::path RunKDialog(const std::string& args) {
    std::string command = "kdialog " + args;
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe)
        return std::filesystem::path();

    char buffer[128];
    std::string result = "";
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    pclose(pipe);
    if (!result.empty() && result.back() == '\n') {
        result.pop_back();
    }
    return std::filesystem::path(result);
}

std::filesystem::path SystemDialogs::OpenFileDialog() {
    return RunKDialog("--getopenfilename");
}

std::filesystem::path SystemDialogs::SaveFileDialog(std::string prefered_name) {
    return RunKDialog("--getsavefilename");
}

std::filesystem::path SystemDialogs::OpenFolderDialog() {
    return RunKDialog("--getexistingdirectory");
}

std::filesystem::path SystemDialogs::SaveFolderDialog() {
    return OpenFolderDialog();
}

#endif
