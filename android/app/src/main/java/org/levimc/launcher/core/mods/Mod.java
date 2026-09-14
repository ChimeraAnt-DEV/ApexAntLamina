package org.levimc.launcher.core.mods;

import android.os.Parcelable;

/**
 * Java side of a kPreloadNative preloader mod. Mirrors what the preloader
 * needs: a library path and an optional mod root directory.
 */
public final class Mod implements Parcelable {

    public final String id;
    public final String displayName;
    public final String version;
    public final String author;
    public final String libPath;
    public final String modRootPath;
    public final boolean enabled;

    public Mod(
            String id,
            String displayName,
            String version,
            String author,
            String libPath,
            String modRootPath,
            boolean enabled) {
        this.id = id;
        this.displayName = displayName;
        this.version = version;
        this.author = author;
        this.libPath = libPath;
        this.modRootPath = modRootPath;
        this.enabled = enabled;
    }

    protected Mod(android.os.Parcel in) {
        this.id = in.readString();
        this.displayName = in.readString();
        this.version = in.readString();
        this.author = in.readString();
        this.libPath = in.readString();
        this.modRootPath = in.readString();
        this.enabled = in.readByte() != 0;
    }

    @Override
    public void writeToParcel(android.os.Parcel dest, int flags) {
        dest.writeString(id);
        dest.writeString(displayName);
        dest.writeString(version);
        dest.writeString(author);
        dest.writeString(libPath);
        dest.writeString(modRootPath);
        dest.writeByte((byte) (enabled ? 1 : 0));
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<Mod> CREATOR = new Creator<Mod>() {
        @Override
        public Mod createFromParcel(android.os.Parcel in) {
            return new Mod(in);
        }

        @Override
        public Mod[] newArray(int size) {
            return new Mod[size];
        }
    };
}