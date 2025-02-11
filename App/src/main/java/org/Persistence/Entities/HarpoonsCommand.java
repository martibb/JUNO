package org.Persistence.Entities;

public class HarpoonsCommand {
    private int command = 0;

    public HarpoonsCommand(int command) {
        this.command = command;
    }

    public HarpoonsCommand(GyroscopeReading gyroData) {
        final int THRESHOLD_OK = 45;

        float xAngle = gyroData.getX();
        float yAngle = gyroData.getY();
        float zAngle = gyroData.getZ();

        // If one of the axes is greater than the threshold, activate the harpoons
        if (Math.abs(xAngle) > THRESHOLD_OK || Math.abs(yAngle) > THRESHOLD_OK || Math.abs(zAngle) > THRESHOLD_OK) {
            command = 1;
        } else {
            command = 0;
        }
    }

    public int getNewCommand() { return command; }

    @Override
    public String toString() {
        boolean state;
        state = command != 0;
        return "Harpoons { Activation: " + state + " }";
    }
}