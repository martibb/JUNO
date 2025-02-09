#ifndef GYROSCOPE_H_
#define GYROSCOPE_H_

#define GYROSCOPE_SAMPLING_INTERVAL 12

PROCESS_NAME(gyroscope_sensor_process);

extern process_event_t GYROSCOPE_SLOPE_EVENT;
extern process_event_t GYROSCOPE_SUB_EVENT;
extern process_event_t GYROSCOPE_ALARM_EVENT;
extern process_event_t GYROSCOPE_STOP_EVENT;

extern int gyro_publishing_enabled;
extern int gyro_test_running;

#endif