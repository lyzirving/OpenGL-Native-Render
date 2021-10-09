package com.render.engine.filter;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FilterConst {
    @StringDef({CONTRAST, SHARPEN, SATURATION, EXPOSURE, HIGHLIGHT_SHADOW, GAUSSIAN, FACE_LIFT})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterType {}

    public static final String CONTRAST = "CONTRAST";
    public static final String SHARPEN = "SHARPEN";
    public static final String SATURATION = "SATURATION";
    public static final String EXPOSURE = "EXPOSURE";
    public static final String HIGHLIGHT_SHADOW = "HIGHLIGHT_SHADOW";
    public static final String GAUSSIAN = "GAUSSIAN";
    public static final String FACE_LIFT = "FACE_LIFT";


    @StringDef({HIGHLIGHT, SHADOW, HOR_GAUSSIAN, VER_GAUSSIAN, FACE_LIFT_INTENSITY})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterProp {}

    public static final String HIGHLIGHT = "HIGHLIGHT";
    public static final String SHADOW = "SHADOW";
    public static final String HOR_GAUSSIAN = "HOR_GAUSSIAN";
    public static final String VER_GAUSSIAN = "VER_GAUSSIAN";
    public static final String FACE_LIFT_INTENSITY = "FACE_LIFT_INTENSITY";
}
