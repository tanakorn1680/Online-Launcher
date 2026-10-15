package com.gtasan.online.utils;

import android.content.Context;

import java.nio.charset.Charset;

/**
 * Text encoding used between the game and the server (chat, dialogs, ...).
 * The index is stored in the "samp_settings" preferences; the matching iconv name
 * is written to settings.json for the native client.
 */
public final class TextEncoding {
    public static final String PREF_KEY = "text_encoding";

    /** iconv names, same order as {@link #LABELS}. Index 0 is the historical default. */
    public static final String[] CODES = {"CP1251", "UTF-8", "CP874", "CP1252", "GBK"};

    public static final String[] LABELS = {
            "Cyrillic (CP1251)", "UTF-8", "Thai (CP874)", "Western (CP1252)", "Chinese (GBK)"
    };

    private static final String[][] JAVA_NAMES = {
            {"windows-1251"},
            {"UTF-8"},
            {"windows-874", "x-windows-874", "TIS-620"},
            {"windows-1252"},
            {"GBK"}
    };

    private TextEncoding() {
    }

    public static int index(Context context) {
        int i = context.getSharedPreferences("samp_settings", Context.MODE_PRIVATE).getInt(PREF_KEY, 0);
        return (i >= 0 && i < CODES.length) ? i : 0;
    }

    public static String code(int index) {
        return (index >= 0 && index < CODES.length) ? CODES[index] : CODES[0];
    }

    public static Charset charset(Context context) {
        for (String name : JAVA_NAMES[index(context)]) {
            try {
                if (Charset.isSupported(name)) {
                    return Charset.forName(name);
                }
            } catch (RuntimeException ignored) {
                // try the next alias
            }
        }
        return Charset.forName("windows-1251");
    }

    public static byte[] encode(Context context, String text) {
        return text.getBytes(charset(context));
    }
}
