package org.Persistence.Entities;

public class MotorsCommand {
    private final int newDirection;
    private final int stepSize;

    public MotorsCommand(int newDirection, int stepSize) {
        this.newDirection = newDirection;
        this.stepSize = stepSize;
    }

    public int getNewDirection() { return newDirection; }
    public int getStepSize() { return stepSize; }

    @Override
    public String toString() {
        return "LegCommand { Direction: " + newDirection + "°, Step Size: " + stepSize + " cm }";
    }
}