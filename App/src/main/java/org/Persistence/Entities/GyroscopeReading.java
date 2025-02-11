package org.Persistence.Entities;

public class GyroscopeReading {
    private final float xAxis;
    private final float yAxis;
    private final float zAxis;

    public GyroscopeReading(float x, float y, float z) {
        xAxis = x;
        yAxis = y;
        zAxis = z;
    }

    public float getX() { return xAxis; }
    public float getY() { return yAxis; }
    public float getZ() { return zAxis; }

    @Override
    public String toString() {
        return "GyroscopeReading { xAxis: " + xAxis + ", yAxis: " + yAxis + ", zAxis: " + zAxis + " }";
    }
}
