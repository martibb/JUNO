package org.Persistence;

import org.Persistence.Entities.*;

import java.sql.*;

public class DataManager {
    private static DataManager instance;
    private final Connection connection;

    private static final String URL = "jdbc:mysql://localhost:3306/rover_control";
    private static final String USER = "juno_driver";
    private static final String PASSWORD = "juno_DRIVER_2025";

    Position position = Position.getInstance();

    private DataManager() {
        try {
            connection = DriverManager.getConnection(URL, USER, PASSWORD);
            System.out.println("Connected to JUNO database.");
        } catch (SQLException e) {
            e.printStackTrace();
            throw new RuntimeException("Error during database connection.");
        }
    }

    public static DataManager getInstance() {
        if (instance == null) {
            instance = new DataManager();
        }
        return instance;
    }

    public void closeConnection() {
        if (connection != null) {
            try {
                connection.close();
                System.out.println("Connection with database closed.");
            } catch (SQLException e) {
                e.printStackTrace();
            }
        }
    }

    public void initializePosition(boolean testRunning) {

        if(testRunning) {
            position.resetPosition();
            return;
        }

        String query = "SELECT x, y FROM rover_position ORDER BY timestamp DESC LIMIT 1";

        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                position.setPosition(rs.getInt("x"), rs.getInt("y"));
                System.out.println("Position loaded from database.");
            } else {
                System.out.println("No position founded on db, set as default.");
                position.setPosition(0, 0);
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public void insertLidarReading(LidarReading newRecord) {

        int distanceFront = newRecord.getDistanceFront();
        int distanceRight = newRecord.getDistanceRight();
        int distanceLeft = newRecord.getDistanceLeft();

        String query = "INSERT INTO lidar_readings (distance_front, distance_right, distance_left) VALUES (?, ?, ?)";

        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, distanceFront);
            stmt.setFloat(2, distanceRight);
            stmt.setFloat(3, distanceLeft);
            stmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public LidarReading getLastLidarReading() {
        String query = "SELECT distance_front, distance_right, distance_left FROM lidar_readings ORDER BY timestamp DESC LIMIT 1";

        try (Statement statement = connection.createStatement();
             ResultSet rs = statement.executeQuery(query)) {
            if (rs.next()) {
                return new LidarReading(
                        rs.getInt("distance_front"),
                        rs.getInt("distance_right"),
                        rs.getInt("distance_left")
                );
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public void insertMotorsCommand(MotorsCommand newRecord) {

        int newDirection = newRecord.getNewDirection();
        int stepSize = newRecord.getStepSize();

        String query = "INSERT INTO leg_commands (new_direction, step_size) VALUES (?, ?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, newDirection);
            stmt.setFloat(2, stepSize);
            stmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public MotorsCommand getLastMotorsCommand() {

        String query = "SELECT new_direction, step_size FROM leg_commands ORDER BY timestamp DESC LIMIT 1";

        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return new MotorsCommand(
                        rs.getInt("new_direction"),
                        rs.getInt("step_size")
                );
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        return null;
    }

    public void insertPosition() {

        int newX = position.getX();
        int newY = position.getY();

        String query = "INSERT INTO rover_position (x, y) VALUES (?, ?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setInt(1, newX);
            stmt.setInt(2, newY);
            stmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public Position getLastPosition() {

        String query = "SELECT x,y FROM rover_position ORDER BY timestamp DESC LIMIT 1";

        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                Position position = Position.getInstance();
                position.setPosition(rs.getInt("x"),rs.getInt("y"));
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }

        return null;
    }

    public void insertGyroscopeData(GyroscopeReading newRecord) {

        float angleX = newRecord.getX();
        float angleY = newRecord.getY();
        float angleZ = newRecord.getZ();

        String query = "INSERT INTO gyro_readings (angle_x, angle_y, angle_z) VALUES (?,?,?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, angleX);
            stmt.setFloat(2, angleY);
            stmt.setFloat(3, angleZ);
            stmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public GyroscopeReading getLastGyroReading() {
        String query = "SELECT angle_x, angle_y, angle_z FROM gyro_readings ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return new GyroscopeReading(
                        rs.getFloat("angle_x"),
                        rs.getFloat("angle_y"),
                        rs.getFloat("angle_z")
                );
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public void insertHarpoonCommand(HarpoonsCommand newCommand) {

        int newState = newCommand.getNewCommand();

        String query = "INSERT INTO harpoon_commands (fire) VALUES (?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setInt(1, newState);
            stmt.executeUpdate();
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public HarpoonsCommand getLastHarpoonCommand() {
        String query = "SELECT fire FROM harpoon_commands ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return new HarpoonsCommand(
                        rs.getInt("fire")
                );
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }
}
