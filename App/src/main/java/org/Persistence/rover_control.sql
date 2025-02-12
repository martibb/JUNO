DROP DATABASE IF EXISTS rover_control;
CREATE DATABASE rover_control;
USE rover_control;

CREATE TABLE lidar_readings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    distance_front INT NOT NULL,
    distance_left INT NOT NULL,
    distance_right INT NOT NULL
);

CREATE TABLE gyro_readings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    angle_x INT NOT NULL,
    angle_y INT NOT NULL,
    angle_z INT NOT NULL
);

CREATE TABLE leg_commands (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    new_direction INT NOT NULL,
    step_size INT NOT NULL
);

CREATE TABLE harpoon_commands (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fire INT NOT NULL
);

CREATE TABLE rover_position (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    x INT NOT NULL,
    y INT NOT NULL
);