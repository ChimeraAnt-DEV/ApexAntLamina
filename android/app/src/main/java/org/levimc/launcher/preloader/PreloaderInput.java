package org.levimc.launcher.preloader;

import org.levimc.launcher.CoreJni;

/**
 * Input bridge expected by libpreloader.so. The launcher forwards touch/key/
 * text/mouse events to the preloader so mods can install input callbacks.
 */
public final class PreloaderInput {

    private PreloaderInput() {}

    public static native boolean nativeOnTouch(int action, int pointerId, float x, float y);

    public static native boolean nativeOnKeyEvent(int keyCode, int unicodeChar, boolean isKeyDown);

    public static native boolean nativeOnTextInput(String text);

    public static native boolean nativeOnMouse(int button, boolean isDown);

    public static native void nativeSetActivity(android.app.Activity activity);

    public static native void nativeClearActivity();

    public static native boolean nativeIsPauseMenuOpen();

    public static native boolean nativeIsHudScreenOpen();

    public static native boolean nativeIsShowingMenu();

    public static native boolean nativeShouldForceGlobalModMenu();

    public static native void nativeConfigureSignatureRules(String rulesPath, String minecraftVersion);

    static {
        CoreJni.ensureLoaded();
    }
}