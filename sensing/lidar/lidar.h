#ifndef LIDAR_H_
#define LIDAR_H_

#define LIDAR_SAMPLING_INTERVAL   4
#define LIDAR_UPPER_BOUND        300
#define LIDAR_LOWER_BOUND        10

PROCESS_NAME(lidar_sensor_process);

extern process_event_t LIDAR_DISTANCE_EVENT;
extern process_event_t LIDAR_SUB_EVENT;
extern process_event_t LIDAR_START_EVENT;
extern process_event_t LIDAR_STOP_EVENT;

extern int publishing_enabled;
extern int test_running;

#endif