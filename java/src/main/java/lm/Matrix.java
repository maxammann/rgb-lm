package lm;

import com.sun.jna.Structure;

/**
* Represents a Matrix
*/ /// <i>native declaration : line 12</i>
/// <i>native declaration : line 12</i>
public class Matrix extends Structure {
    public int xx;
    public int xy;
    public int yx;
    public int yy;

    public Matrix() {
        super();
        initFieldOrder();
    }

    protected void initFieldOrder() {
        setFieldOrder(new String[]{"xx", "xy", "yx", "yy"});
    }

    public Matrix(int xx, int xy, int yx, int yy) {
        super();
        this.xx = xx;
        this.xy = xy;
        this.yx = yx;
        this.yy = yy;
        initFieldOrder();
    }

    public static class ByReference extends Matrix implements Structure.ByReference {
        public ByReference(int xx, int xy, int yx, int yy) {
            super(xx, xy, yx, yy);
        }
    }

    public static class ByValue extends Matrix implements Structure.ByValue {
        public ByValue(int xx, int xy, int yx, int yy) {
            super(xx, xy, yx, yy);
        }
    }
}
