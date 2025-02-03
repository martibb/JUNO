package org.Persistence;

import java.sql.*;

public class DataManager {
    private static DataManager instance;
    private final Connection connection;

    private static final String URL = "jdbc:mysql://localhost:3306/rover_control";
    private static final String USER = "juno_driver";
    private static final String PASSWORD = "juno_DRIVER_2025";

    private DataManager() {
        try {
            connection = DriverManager.getConnection(URL, USER, PASSWORD);
            System.out.println("Connected with JUNO database.");
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

    public void insertLidarReading(float distanceFront) {
        String query = "INSERT INTO lidar_readings (distance_front) VALUES (?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, distanceFront);
            stmt.executeUpdate();
            System.out.println("Dati Lidar salvati: " + distanceFront + " m");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public float getLastLidarReading() {
        String query = "SELECT distance_front FROM lidar_readings ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return rs.getFloat("distance_front");
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return -1;  // Valore di default se non ci sono dati
    }

    public void insertGyroReading(float angle) {
        String query = "INSERT INTO gyro_readings (angle) VALUES (?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, angle);
            stmt.executeUpdate();
            System.out.println("Dati Giroscopio salvati: " + angle + "°");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public Float getLastGyroReading() {
        String query = "SELECT angle FROM gyro_readings ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return rs.getFloat("angle");
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return null;
    }

    public void insertLegCommand(float newDirection, float stepSize) {
        String query = "INSERT INTO leg_commands (new_direction, step_size) VALUES (?, ?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setFloat(1, newDirection);
            stmt.setFloat(2, stepSize);
            stmt.executeUpdate();
            System.out.println("Comando Gambe salvato: Direzione " + newDirection + "°, Passo " + stepSize + " cm");
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public String getLastLegCommand() {
        String query = "SELECT new_direction, step_size FROM leg_commands ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return "Direzione: " + rs.getFloat("new_direction") + "°, Passo: " + rs.getFloat("step_size") + " cm";
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return "Nessun comando trovato.";
    }

    public void insertHarpoonCommand(boolean fire) {
        String query = "INSERT INTO harpoon_commands (fire) VALUES (?)";
        try (PreparedStatement stmt = connection.prepareStatement(query)) {
            stmt.setBoolean(1, fire);
            stmt.executeUpdate();
            System.out.println("Comando Arpioni salvato: " + (fire ? "LANCIO" : "FERMO"));
        } catch (SQLException e) {
            e.printStackTrace();
        }
    }

    public String getLastHarpoonCommand() {
        String query = "SELECT fire FROM harpoon_commands ORDER BY timestamp DESC LIMIT 1";
        try (Statement stmt = connection.createStatement();
             ResultSet rs = stmt.executeQuery(query)) {
            if (rs.next()) {
                return rs.getBoolean("fire") ? "LANCIO" : "FERMO";
            }
        } catch (SQLException e) {
            e.printStackTrace();
        }
        return "Nessun comando trovato.";
    }
}
