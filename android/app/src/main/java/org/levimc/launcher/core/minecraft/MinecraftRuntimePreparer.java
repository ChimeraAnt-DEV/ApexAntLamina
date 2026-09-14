package org.levimc.launcher.core.minecraft;

import org.levimc.launcher.CoreJni;

/**
 * Runtime preparer expected by libpreloader.so. On LeviLauncher this points
 * the native runtime at the mod directory; ApexAntLamina installs its mod
 * folder under filesDir/mods, so this is where the mod root is published.
 */
public final class MinecraftRuntimePreparer {

    private MinecraftRuntimePreparer() {}

    public static native void nativeSetupRuntime(String modsPath);

    static {
        CoreJni.ensureLoaded();
    }
}