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
    angle_x FLOAT NOT NULL,
    angle_y FLOAT NOT NULL,
    angle_z FLOAT NOT NULL
);

CREATE TABLE leg_commands (
    id INT AUTO_INCREMENT PRIMARY KEY,
    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    new_direction FLOAT NOT NULL,
    step_size FLOAT NOT NULL
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