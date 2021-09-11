package com.render.engine.core.filter;

import androidx.annotation.StringDef;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

public class FilterConst {
    @StringDef({CONTRAST})
    @Retention(RetentionPolicy.SOURCE)
    public @interface FilterType {}

    public static final String CONTRAST = "CONTRAST";
}
