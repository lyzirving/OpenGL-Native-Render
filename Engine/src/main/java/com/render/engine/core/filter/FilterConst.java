package com.render.engine.core.filter;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FilterConst {
    @StringDef({CONTRAST, SHARPEN, SATURATION, EXPOSURE, HIGHLIGHT_SHADOW})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterType {}

    public static final String CONTRAST = "CONTRAST";
    public static final String SHARPEN = "SHARPEN";
    public static final String SATURATION = "SATURATION";
    public static final String EXPOSURE = "EXPOSURE";
    public static final String HIGHLIGHT_SHADOW = "HIGHLIGHT_SHADOW";


    @StringDef({HIGHLIGHT, SHADOW})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterProp {}

    public static final String HIGHLIGHT = "HIGHLIGHT";
    public static final String SHADOW = "SHADOW";
}
