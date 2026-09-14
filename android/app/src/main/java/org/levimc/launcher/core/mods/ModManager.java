package org.levimc.launcher.core.mods;

import org.levimc.launcher.CoreJni;

/**
 * Bridge used by the preloader runtime (org/levimc/launcher/core/mods/
 * ModManager). The native methods are implemented in libpreloader.so and
 * drive the C++ mod lifecycle (PLGetModRegistration).
 */
public final class ModManager {

    private ModManager() {}

    /**
     * Load a preload-native mod by absolute library path.
     *
     * @param libPath absolute path to the mod's .so
     * @param mod     Java Mod metadata (null ok)
     * @return true if loaded
     */
    public static native boolean nativeLoadMod(String libPath, Mod mod);

    /**
     * Load a preload-native mod from a mod root directory.
     */
    public static native boolean nativeLoadMod(String libPath, String modRootPath, Mod mod);

    /** Enable all loaded mods (invokes the C++ enable lifecycle). */
    public static native void nativeEnableLoadedMods();

    /** Disable and unload all loaded mods. */
    public static native void nativeDisableAndUnloadLoadedMods();

    static {
        CoreJni.ensureLoaded();
    }
}