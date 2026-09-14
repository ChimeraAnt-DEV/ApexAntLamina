package org.levimc.launcher.core.mods.inbuilt;

import org.levimc.launcher.CoreJni;

/**
 * Inbuilt mod menu bridge expected by libpreloader.so. The launcher exposes
 * an in-game mod menu through these native queries; the numbers here are
 * stubs so that the preloader does not crash when it queries the bridge.
 */
public final class ExternalModBridge {

    private ExternalModBridge() {}

    public static native int nativeGetExternalModCount();

    public static native String nativeGetExternalModInfo(int index);

    public static native void nativeToggleExternalMod(String id);

    public static native void nativeSetExternalModConfig(String id, String key, String value);

    public static native int nativeGetExternalButtonCount();

    public static native String nativeGetExternalButtonInfo(int index);

    public static native byte[] nativeGetExternalButtonIconBytes(int index);

    public static native void nativeDispatchExternalButtonEvent(int index);

    public static native Object[] nativeGetDrawCommands();

    public static native byte[] nativeGetRegisteredFontBytes();

    public static native Object[] nativeGetRegisteredImage();

    static {
        CoreJni.ensureLoaded();
    }
}