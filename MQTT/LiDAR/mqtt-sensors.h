#ifndef LIDAR_H_
#define LIDAR_H_

#define LIDAR_SAMPLING_INTERVAL   2
#define LIDAR_UPPER_BOUND        300
#define LIDAR_LOWER_BOUND        10

PROCESS_NAME(lidar_sensor_process);
	
extern process_event_t LINDAR_DISTANCE_EVENT;
extern process_event_t LINDAR_SUB_EVENT;

#endif