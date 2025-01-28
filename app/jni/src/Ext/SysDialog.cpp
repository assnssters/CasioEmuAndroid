#include "SysDialog.h"

#ifdef _WIN32
#include <windows.h>
#include <shobjidl.h>
#include <filesystem>
#include <functional>

void SystemDialogs::OpenFileDialog(std::function<void(std::filesystem::path)> callback) {
    IFileDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_FORCEFILESYSTEM);
        }

        COMDLG_FILTERSPEC rgSpec[] = {
            {L"All Files", L"*.*"}};
        pfd->SetFileTypes(1, rgSpec);

        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR path;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    callback(std::filesystem::path(path));
                    CoTaskMemFree(path);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}

void SystemDialogs::SaveFileDialog(std::string preferred_name, std::function<void(std::filesystem::path)> callback) {
    IFileSaveDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_FORCEFILESYSTEM);
        }

        if (!preferred_name.empty()) {
            int needed = MultiByteToWideChar(CP_UTF8, 0, preferred_name.c_str(), -1, NULL, 0);
            if (needed > 0) {
                std::wstring wname(needed - 1, 0);
                MultiByteToWideChar(CP_UTF8, 0, preferred_name.c_str(), -1, wname.data(), needed);
                pfd->SetFileName(wname.c_str());
            }
        }

        COMDLG_FILTERSPEC rgSpec[] = {
            {L"All Files", L"*.*"}};
        pfd->SetFileTypes(1, rgSpec);

        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR path;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    callback(std::filesystem::path(path));
                    CoTaskMemFree(path);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}

void SystemDialogs::OpenFolderDialog(std::function<void(std::filesystem::path)> callback) {
    IFileDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        }

        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR path;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    callback(std::filesystem::path(path));
                    CoTaskMemFree(path);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}

void SystemDialogs::SaveFolderDialog(std::function<void(std::filesystem::path)> callback) {
    IFileDialog* pfd;
    HRESULT hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr)) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        }

        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem* psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR path;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                    callback(std::filesystem::path(path));
                    CoTaskMemFree(path);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
}
#endif

#ifdef __ANDROID__
#include <jni.h>
#include <android/log.h>
#include <SDL.h>
#include <SDL_system.h>

// Initialize static members
std::function<void(std::filesystem::path)> SystemDialogs::fileOpenCallback;
std::function<void(std::filesystem::path)> SystemDialogs::fileSaveCallback;
std::function<void(std::filesystem::path)> SystemDialogs::folderOpenCallback;
std::function<void(std::filesystem::path)> SystemDialogs::folderSaveCallback;

static bool GetJNIEnv(JNIEnv **env) {
    *env = (JNIEnv*)SDL_AndroidGetJNIEnv();
    return (*env != NULL);
}

void SystemDialogs::OpenFileDialog(std::function<void(std::filesystem::path)> callback) {
    fileOpenCallback = callback;
    
    JNIEnv *env;
    if (!GetJNIEnv(&env)) {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        return;
    }

    jclass systemDialogsClass = env->FindClass("com/tele/u8emulator/SystemDialogs");
    if (!systemDialogsClass) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID openFileMethod = env->GetStaticMethodID(systemDialogsClass, "openFileDialog", 
        "(Landroid/app/Activity;)V");
    if (!openFileMethod) {
        env->DeleteLocalRef(systemDialogsClass);
        env->DeleteLocalRef(activity);
        return;
    }

    env->CallStaticVoidMethod(systemDialogsClass, openFileMethod, activity);

    env->DeleteLocalRef(systemDialogsClass);
    env->DeleteLocalRef(activity);
}

void SystemDialogs::SaveFileDialog(std::string preferred_name, std::function<void(std::filesystem::path)> callback) {
    fileSaveCallback = callback;

    JNIEnv *env;
    if (!GetJNIEnv(&env)) {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        return;
    }

    jclass systemDialogsClass = env->FindClass("com/tele/u8emulator/SystemDialogs");
    if (!systemDialogsClass) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID saveFileMethod = env->GetStaticMethodID(systemDialogsClass, "saveFileDialog", 
        "(Landroid/app/Activity;Ljava/lang/String;)V");
    if (!saveFileMethod) {
        env->DeleteLocalRef(systemDialogsClass);
        env->DeleteLocalRef(activity);
        return;
    }

    jstring jPreferredName = env->NewStringUTF(preferred_name.c_str());
    env->CallStaticVoidMethod(systemDialogsClass, saveFileMethod, activity, jPreferredName);

    env->DeleteLocalRef(jPreferredName);
    env->DeleteLocalRef(systemDialogsClass);
    env->DeleteLocalRef(activity);
}

void SystemDialogs::OpenFolderDialog(std::function<void(std::filesystem::path)> callback) {
    folderOpenCallback = callback;

    JNIEnv *env;
    if (!GetJNIEnv(&env)) {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        return;
    }

    jclass systemDialogsClass = env->FindClass("com/tele/u8emulator/SystemDialogs");
    if (!systemDialogsClass) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID openFolderMethod = env->GetStaticMethodID(systemDialogsClass, "openFolderDialog", 
        "(Landroid/app/Activity;)V");
    if (!openFolderMethod) {
        env->DeleteLocalRef(systemDialogsClass);
        env->DeleteLocalRef(activity);
        return;
    }

    env->CallStaticVoidMethod(systemDialogsClass, openFolderMethod, activity);

    env->DeleteLocalRef(systemDialogsClass);
    env->DeleteLocalRef(activity);
}

void SystemDialogs::SaveFolderDialog(std::function<void(std::filesystem::path)> callback) {
    folderSaveCallback = callback;

    JNIEnv *env;
    if (!GetJNIEnv(&env)) {
        return;
    }

    jobject activity = (jobject)SDL_AndroidGetActivity();
    if (!activity) {
        return;
    }

    jclass systemDialogsClass = env->FindClass("com/tele/u8emulator/SystemDialogs");
    if (!systemDialogsClass) {
        env->DeleteLocalRef(activity);
        return;
    }

    jmethodID saveFolderMethod = env->GetStaticMethodID(systemDialogsClass, "saveFolderDialog", 
        "(Landroid/app/Activity;)V");
    if (!saveFolderMethod) {
        env->DeleteLocalRef(systemDialogsClass);
        env->DeleteLocalRef(activity);
        return;
    }

    env->CallStaticVoidMethod(systemDialogsClass, saveFolderMethod, activity);

    env->DeleteLocalRef(systemDialogsClass);
    env->DeleteLocalRef(activity);
}

// JNI callbacks
extern "C" {
    JNIEXPORT void JNICALL Java_com_tele_u8emulator_Game_onFileSelected(JNIEnv* env, jclass clazz, jstring path) {
        if (SystemDialogs::fileOpenCallback) {
            const char* cPath = env->GetStringUTFChars(path, nullptr);
            SystemDialogs::fileOpenCallback(std::filesystem::path(cPath));
            env->ReleaseStringUTFChars(path, cPath);
        }
    }

    JNIEXPORT void JNICALL Java_com_tele_u8emulator_Game_onFileSaved(JNIEnv* env, jclass clazz, jstring path) {
        if (SystemDialogs::fileSaveCallback) {
            const char* cPath = env->GetStringUTFChars(path, nullptr);
            SystemDialogs::fileSaveCallback(std::filesystem::path(cPath));
            env->ReleaseStringUTFChars(path, cPath);
        }
    }

    JNIEXPORT void JNICALL Java_com_tele_u8emulator_Game_onFolderSelected(JNIEnv* env, jclass clazz, jstring path) {
        if (SystemDialogs::folderOpenCallback) {
            const char* cPath = env->GetStringUTFChars(path, nullptr);
            SystemDialogs::folderOpenCallback(std::filesystem::path(cPath));
            env->ReleaseStringUTFChars(path, cPath);
        }
    }

    JNIEXPORT void JNICALL Java_com_tele_u8emulator_Game_onFolderSaved(JNIEnv* env, jclass clazz, jstring path) {
        if (SystemDialogs::folderSaveCallback) {
            const char* cPath = env->GetStringUTFChars(path, nullptr);
            SystemDialogs::folderSaveCallback(std::filesystem::path(cPath));
            env->ReleaseStringUTFChars(path, cPath);
        }
    }
}
#endif