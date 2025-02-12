package org.Persistence.Entities;

public class GyroscopeReading {
    private final int xAxis;
    private final int yAxis;
    private final int zAxis;

    public GyroscopeReading(int x, int y, int z) {
        xAxis = x;
        yAxis = y;
        zAxis = z;
    }

    public int getX() { return xAxis; }
    public int getY() { return yAxis; }
    public int getZ() { return zAxis; }

    @Override
    public String toString() {
        return "GyroscopeReading { xAxis: " + xAxis + ", yAxis: " + yAxis + ", zAxis: " + zAxis + " }";
    }
}
