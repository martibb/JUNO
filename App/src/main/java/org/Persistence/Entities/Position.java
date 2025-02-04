package org.Persistence.Entities;

public class Position {
    private static Position instance;
    private int x;
    private int y;

    private Position() {
        x = 0;
        y = 0;
    }

    public static synchronized Position getInstance() {
        if (instance == null) {
            instance = new Position();
        }
        return instance;
    }

    public void updatePosition(MotorsCommand newCommand) {
        int direction = newCommand.getNewDirection();
        int angle = newCommand.getStepSize();

        if(direction == 2 || direction == 4) {
            x += angle;
        }
        else {
            y += angle;
        }
    }

    public void resetPosition() {
        this.x = 0;
        this.y = 0;
        System.out.println("Reset position.");
    }

    public void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    public int getX() {
        return x;
    }

    public int getY() {
        return y;
    }
}