package com.render.engine.filter;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FilterConst {
    @StringDef({EXPOSURE, HIGHLIGHT_SHADOW, GAUSSIAN, FACE_LIFT, COLOR_ADJUST})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterType {}

    public static final String EXPOSURE = "EXPOSURE";
    public static final String HIGHLIGHT_SHADOW = "HIGHLIGHT_SHADOW";
    public static final String GAUSSIAN = "GAUSSIAN";
    public static final String FACE_LIFT = "FACE_LIFT";
    public static final String COLOR_ADJUST = "COLOR_ADJUST";


    @StringDef({HIGHLIGHT, SHADOW, HOR_GAUSSIAN, VER_GAUSSIAN, FACE_LIFT_INTENSITY, PROP_SHARPEN, PROP_CONTRAST, PROP_SATURATION})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterProp {}

    public static final String HIGHLIGHT = "HIGHLIGHT";
    public static final String SHADOW = "SHADOW";
    public static final String HOR_GAUSSIAN = "HOR_GAUSSIAN";
    public static final String VER_GAUSSIAN = "VER_GAUSSIAN";
    public static final String FACE_LIFT_INTENSITY = "FACE_LIFT_INTENSITY";
    public static final String PROP_SHARPEN = "SHARPEN";
    public static final String PROP_CONTRAST = "CONTRAST";
    public static final String PROP_SATURATION = "SATURATION";
}
