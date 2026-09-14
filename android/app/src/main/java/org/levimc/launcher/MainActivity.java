package org.levimc.launcher;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Toast;

import org.levimc.launcher.core.minecraft.MinecraftRuntimePreparer;
import org.levimc.launcher.core.mods.Mod;
import org.levimc.launcher.core.mods.ModManager;
import org.levimc.launcher.preloader.PreloaderInput;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Minimal LeviLaunchroid-compatible host. Bundles the ApexAntLamina
 * preload-native mod (manifest.json + preload-levilamina.so) in the app's
 * native lib dir, copies the mod folder into filesDir and drives the
 * lifecycle through the preloader's Java bridge:
 *
 *   JavaVM -> libpreloader JNI_OnLoad
 *   PreloaderInput.nativeSetActivity(this)
 *   MinecraftRuntimePreparer.nativeSetupRuntime(modsRoot)
 *   ModManager.nativeLoadMod(libPath, modRoot, mod)
 *   ModManager.nativeEnableLoadedMods()
 */
public final class MainActivity extends Activity {

    private static final String TAG = "ApexAntLamina";
    private static final String LIB_NAME = "libpreload-levilamina.so";
    private static final String MOD_DIR = "mods";

    private boolean mLoaded;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        View root = new View(this);
        root.setBackgroundColor(0xFF000000);
        setContentView(root);

        try {
            CoreJni.ensureLoaded();
            bootstrap();
            showBootScreen();
        } catch (Throwable t) {
            Log.e(TAG, "ApexAntLamina failed to start", t);
            Toast.makeText(this, "ApexAntLamina failed: " + t.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private void bootstrap() throws IOException {
        File modsRoot = new File(getFilesDir(), MOD_DIR);
        File modRoot = new File(modsRoot, "ApexAntLamina");
        File libDir = new File(getApplicationInfo().nativeLibraryDir);

        // Extract manifest.json + the preload .so next to each other, the
        // layout the preloader's ModManifest scanner expects.
        File manifest = new File(modRoot, "manifest.json");
        File lib = new File(modRoot, LIB_NAME);
        copyAssetOrThrow("manifest.json", manifest, false);
        File nativeLib = new File(libDir, LIB_NAME);
        copyFile(nativeLib, lib);

        if (!manifest.isFile() || !lib.isFile()) {
            throw new IOException("mod bundle incomplete: " + modRoot);
        }

        if (!lib.canRead()) {
            Log.w(TAG, "libpreload-levilamina.so not executable from filesDir; using fallback");
        }

        // Publish the mods path to the native runtime (LeviLauncher glue).
        MinecraftRuntimePreparer.nativeSetupRuntime(modsRoot.getAbsolutePath());
        PreloaderInput.nativeSetActivity(this);

        // Load + enable. Loading uses a 2-arg overload: libPath + modRoot.
        Mod mod = new Mod(
                "ApexAntLamina",
                "ApexAntLamina",
                "0.1.0",
                "ChimeraAnt-DEV",
                lib.getAbsolutePath(),
                modRoot.getAbsolutePath(),
                true);
        boolean loaded = ModManager.nativeLoadMod(lib.getAbsolutePath(), modRoot.getAbsolutePath(), mod);
        if (!loaded) {
            Log.e(TAG, "nativeLoadMod returned false");
            throw new IllegalStateException("nativeLoadMod failed for " + lib);
        }
        mLoaded = true;
        ModManager.nativeEnableLoadedMods();
    }

    private void showBootScreen() {
        Toast.makeText(this, mLoaded ? "ApexAntLamina loaded" : "ApexAntLamina not loaded", Toast.LENGTH_LONG).show();
    }

    // -------- InputBridge activity hooks (called natively) --------

    /** Public method invoked from native code via reflection when a mod requests the keyboard. */
    public void showSoftKeyboard() {
        Log.i(TAG, "showSoftKeyboard requested");
    }

    /** Public method invoked from native code via reflection when a mod hides the keyboard. */
    public void hideSoftKeyboard() {
        Log.i(TAG, "hideSoftKeyboard requested");
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mLoaded) {
            try {
                ModManager.nativeDisableAndUnloadLoadedMods();
            } catch (Throwable ignored) {
            }
        }
        try {
            PreloaderInput.nativeClearActivity();
        } catch (Throwable ignored) {
        }
    }

    // -------- helpers --------

    private void copyAssetOrThrow(String asset, File out, boolean overwrite) throws IOException {
        if (out.isFile() && !overwrite) {
            return;
        }
        try (InputStream in = getAssets().open(asset)) {
            // noinspection ResultOfMethodCallIgnored
            out.getParentFile().mkdirs();
            try (OutputStream os = new FileOutputStream(out)) {
                byte[] buf = new byte[8192];
                int n;
                while ((n = in.read(buf)) > 0) {
                    os.write(buf, 0, n);
                }
            }
        }
    }

    private static void copyFile(File src, File dst) throws IOException {
        // noinspection ResultOfMethodCallIgnored
        dst.getParentFile().mkdirs();
        try (InputStream in = new java.io.FileInputStream(src);
             OutputStream os = new FileOutputStream(dst)) {
            byte[] buf = new byte[8192];
            int n;
            while ((n = in.read(buf)) > 0) {
                os.write(buf, 0, n);
            }
        }
    }
}