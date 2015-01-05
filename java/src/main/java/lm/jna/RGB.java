package lm.jna;

import com.sun.jna.Structure;

/**
* Represents a RGB
*/ /// <i>native declaration : line 84</i>
/// <i>native declaration : line 84</i>
public class RGB extends Structure {
    public byte r;
    public byte g;
    public byte b;

    public RGB() {
        super();
        initFieldOrder();
    }

    protected void initFieldOrder() {
        setFieldOrder(new String[]{"r", "g", "b"});
    }

    public RGB(int r, int g, int b) {
        super();
        this.r = (byte) r;
        this.g = (byte) g;
        this.b = (byte) b;
        initFieldOrder();
    }

    public static class ByReference extends RGB implements Structure.ByReference {
        public ByReference(byte r, byte g, byte b) {
            super(r, g, b);
        }
    }

    public static class ByValue extends RGB implements Structure.ByValue {
        public ByValue(byte r, byte g, byte b) {
            super(r, g, b);
        }
    }
}
