package lm;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
* Represents a lmMatrix_
*/ /// <i>native declaration : line 12</i>
/// <i>native declaration : line 12</i>
public class lmMatrix_ extends Structure {
    public int xx;
    public int xy;
    public int yx;
    public int yy;

    public lmMatrix_() {
        super();
    }

    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[]{"xx", "xy", "yx", "yy"});
    }

    public lmMatrix_(int xx, int xy, int yx, int yy) {
        super();
        this.xx = xx;
        this.xy = xy;
        this.yx = yx;
        this.yy = yy;
    }

    public static class ByReference extends lmMatrix_ implements Structure.ByReference {

    }

    ;

    public static class ByValue extends lmMatrix_ implements Structure.ByValue {

    }

    ;
}
