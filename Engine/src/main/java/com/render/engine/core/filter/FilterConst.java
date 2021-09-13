package com.render.engine.core.filter;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FilterConst {
    @StringDef({CONTRAST, SHARPEN, SATURATION})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterType {}

    public static final String CONTRAST = "CONTRAST";
    public static final String SHARPEN = "SHARPEN";
    public static final String SATURATION = "SATURATION";
}
