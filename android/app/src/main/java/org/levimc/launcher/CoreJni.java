package org.levimc.launcher;

/**
 * Loads libpreloader.so (the ApexAntLamina runtime) once for the whole
 * process. All JNI bridge classes funnel through here so the library is
 * never loaded twice and the JNI_OnLoad JavaVM is installed exactly once.
 */
public final class CoreJni {

    private static volatile boolean sLoaded = false;

    private CoreJni() {}

    public static synchronized void ensureLoaded() {
        if (sLoaded) {
            return;
        }
        // libpreloader.so must be loaded with RTLD_GLOBAL so libpreloader's
        // exported pl:: symbols resolve when the preload-native mod .so is
        // dlopen'ed afterwards.
        try {
            System.loadLibrary("preloader");
            sLoaded = true;
        } catch (UnsatisfiedLinkError e) {
            android.util.Log.e("ApexAntLamina", "Failed to load libpreloader.so", e);
            throw e;
        }
    }
}