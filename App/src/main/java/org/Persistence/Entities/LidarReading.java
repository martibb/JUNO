package org.Persistence.Entities;

public class LidarReading {
    private final int distanceFront;
    private final int distanceRight;
    private final int distanceLeft;

    public LidarReading(int distanceFront, int distanceRight, int distanceLeft) {
        this.distanceFront = distanceFront;
        this.distanceRight = distanceRight;
        this.distanceLeft = distanceLeft;
    }

    public int getDistanceFront() { return distanceFront; }
    public int getDistanceRight() { return distanceRight; }
    public int getDistanceLeft() { return distanceLeft; }

    @Override
    public String toString() {
        return "LidarReading { Right: " + distanceRight + " cm, Front: " + distanceFront + " cm, Left: " + distanceLeft + " cm }";
    }
}
