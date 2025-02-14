package org.Persistence.Entities;

public class MotorsCommand {
    private int newDirection = 1;
    private int stepSize = 50;

    public MotorsCommand(int newDirection, int stepSize) {
        this.newDirection = newDirection;
        this.stepSize = stepSize;
    }

    public MotorsCommand(LidarReading newLidarRecord) {
        final int FORWARD = 1;
        final int RIGHT = 2;
        final int BACKWARD = 3;
        final int LEFT = 4;
        final int MAX_DISTANCE = 100;
        final int MIN_DISTANCE = 20;

        int frontDistance = newLidarRecord.getDistanceFront();
        int rightDistance = newLidarRecord.getDistanceRight();
        int leftDistance = newLidarRecord.getDistanceLeft();

        // If in every direction the lidar senses a distance below 20 meters, the rover is in a dead end
        // so the best decision is to walk backward
        if (frontDistance < MIN_DISTANCE && rightDistance < MIN_DISTANCE && leftDistance < MIN_DISTANCE) {
            newDirection = BACKWARD;
            stepSize = Math.max(frontDistance, Math.max(rightDistance, leftDistance))/2;
        }

        // If the sensed front direction is equal to the maximum possible,
        // there is no obstacle and the rover can go FORWARD
        else if (frontDistance == MAX_DISTANCE) {
            newDirection = FORWARD;
            stepSize = frontDistance/2;
        }

        // If the front sensed distance is below the maximum than the sensor can sense, there might be an obstacle,
        // so the rover moves right or left, depending on the most free path
        else if (rightDistance > leftDistance) {
            newDirection = RIGHT;
            stepSize = rightDistance/2;
        }

        else {
            newDirection = LEFT;
            stepSize = leftDistance/2;
        }

        if (newDirection == 4 || newDirection == 3) {
            stepSize = -stepSize;
        }
    }

    public int getNewDirection() { return newDirection; }
    public int getStepSize() { return stepSize; }

    @Override
    public String toString() {
        return "LegCommand { Direction: " + newDirection + "°, Step Size: " + stepSize + " cm }";
    }
}