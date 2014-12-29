package lm;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
* Represents a rgb_
*/ /// <i>native declaration : line 76</i>
/// <i>native declaration : line 76</i>
public class rgb_ extends Structure {
    public byte r;
    public byte g;
    public byte b;

    public rgb_() {
        super();
    }

    @Override
    protected List getFieldOrder() {
        return Arrays.asList(new String[]{"r", "g", "b"});
    }

    public rgb_(byte r, byte g, byte b) {
        super();
        this.r = r;
        this.g = g;
        this.b = b;
    }

    public static class ByReference extends rgb_ implements Structure.ByReference {

    }

    ;

    public static class ByValue extends rgb_ implements Structure.ByValue {

    }

    ;
}
