package org.MQTT;

import org.CoAP.CoAPClient;
import org.Persistence.DataManager;
import org.Persistence.Entities.*;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

// MQTT Client class
public class MQTTCollector implements MqttCallback{
    private final String lidarTopic = "lidar";
    private final String gyroscopeTopic = "gyroscope";
    private MqttClient mqttClient = null;
    private boolean testRunning;
    private final DataManager dataManager = DataManager.getInstance();
    private final Position position = Position.getInstance();

    //-----------------------------------------------------------------------*/

    public MQTTCollector() {
        do {
            try {
                String broker = "tcp://[fd00::1]:1883";
                String clientId = "JavaApp";
                mqttClient = new MqttClient(broker, clientId);
                System.out.println("Connecting to broker: "+ broker);

                mqttClient.setCallback(this);
                mqttClient.connect();

            }catch(MqttException me) {
                System.out.println("I could not connect, Retrying ...");
            }
        }while(!mqttClient.isConnected());
    }

    public void startRetrieving() {
        subscribeToTopic(lidarTopic);
        subscribeToTopic(gyroscopeTopic);
    }

    public void subscribeToTopic(String topic) {
        try {
            mqttClient.subscribe(topic);
            System.out.println("Subscribed to topic: " + topic);
        } catch (MqttException e) {
            System.out.println("Failed to subscribe to topic: " + topic);
            e.printStackTrace();
        }
    }

    public void stopRetrieving() {
        unsubscribeFromTopic(lidarTopic);
        //unsubscribeFromTopic(gyroscopeTopic);
    }

    public void unsubscribeFromTopic(String topic) {
        try {
            mqttClient.unsubscribe(topic);
            System.out.println("Unsubscribed from topic: " + topic);
        } catch (MqttException e) {
            System.out.println("Failed to unsubscribe from topic: " + topic);
            e.printStackTrace();
        }
    }

    public void connectionLost(Throwable cause) {
        System.out.println("Connection is broken: " + cause);
        int timeWindow = 3000;
        while (!mqttClient.isConnected()) {
            try {
                System.out.println("Trying to reconnect in " + timeWindow/1000 + " seconds.");
                Thread.sleep(timeWindow);
                System.out.println("Reconnecting ...");
                timeWindow *= 2;
                mqttClient.connect();

                mqttClient.subscribe(lidarTopic);
                //mqttClient.subscribe(gyroscopeTopic);
                System.out.println("Connection is restored");
            }catch(MqttException | InterruptedException me) {
                System.out.println("I could not connect");
            }
        }
    }

    public void messageArrived(String topic, MqttMessage message) {
        byte[] payload = message.getPayload();
        System.out.println("New message received!");

        try {
            JSONObject sensorMessage = (JSONObject) JSONValue.parseWithException(new String(payload));

            if (topic.equals(lidarTopic)) {
                new Thread(() -> handleLidarMessage(sensorMessage)).start();
            } else if (topic.equals(gyroscopeTopic)) {
                new Thread(() -> handleGyroscopeMessage(sensorMessage)).start();
            } else {
                System.out.printf("Unknown topic: [%s] %s%n", topic, new String(payload));
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void handleLidarMessage(JSONObject sensorMessage) {
        try {
            int frontDistance = Integer.parseInt(sensorMessage.get("distance_front").toString());
            int rightDistance = Integer.parseInt(sensorMessage.get("distance_right").toString());
            int leftDistance = Integer.parseInt(sensorMessage.get("distance_left").toString());

            LidarReading newLidarRecord = new LidarReading(frontDistance, rightDistance, leftDistance);
            MotorsCommand newMotorsRecord = new MotorsCommand(newLidarRecord);

            synchronized (this) {
                dataManager.insertLidarReading(newLidarRecord);
                dataManager.insertMotorsCommand(newMotorsRecord);

                position.updatePosition(newMotorsRecord);
                if (!testRunning) {
                    dataManager.insertPosition();
                }
            }

            long startTime = System.currentTimeMillis();
            int newDirection = newMotorsRecord.getNewDirection();
            int stepSize = newMotorsRecord.getStepSize();
            String postPayload = "{ \"direction\": " + newDirection + ", \"angle\": " + stepSize + " }";
            CoAPClient.actuatorsPostRequest(CoAPClient.getServoMotorsUri(), postPayload);

            long endTime = System.currentTimeMillis();
            System.out.println("[LIDAR] CoAP Request Time: " + (endTime - startTime) + " ms");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void handleGyroscopeMessage(JSONObject sensorMessage) {
        try {
            System.out.println("\nWork in progress for gyroscope.\n");

            float angleX = Float.parseFloat(sensorMessage.get("gyro_x").toString());
            float angleY = Float.parseFloat(sensorMessage.get("gyro_y").toString());
            float angleZ = Float.parseFloat(sensorMessage.get("gyro_z").toString());

            GyroscopeReading newGyroscopeRecord = new GyroscopeReading(angleX, angleY, angleZ);
            HarpoonsCommand newHarpoonsCommand = new HarpoonsCommand(newGyroscopeRecord);

            synchronized (this) {
                dataManager.insertGyroscopeData(newGyroscopeRecord);
                dataManager.insertHarpoonCommand(newHarpoonsCommand);

                /* position.updateInclinatio(newMotorsRecord); //TODO
                if (!testRunning) {
                    dataManager.insertPosition();
                }*/
            }

            long startTime = System.currentTimeMillis();
            int stateRequest = newHarpoonsCommand.getNewCommand(); //TODO TOTRY //Provare attivazione, inserimento nel db
            String postPayload = "{ \"request\": " + stateRequest + " }";
            System.out.println(postPayload);
            CoAPClient.actuatorsPostRequest(CoAPClient.getHarpoonsUri(), postPayload);

            long endTime = System.currentTimeMillis();
            System.out.println("[GYROSCOPE] CoAP Request Time: " + (endTime - startTime) + " ms");
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void deliveryComplete(IMqttDeliveryToken token) {
        System.out.println("Delivery completed.\n");
    }

    public void sendControlCommand(String command) {
        if (mqttClient.isConnected()) {

            if(command.equals("start") || command.equals("stop"))
                testRunning = false;
            else if(command.equals("test-session"))
                testRunning = true;

            try {
                MqttMessage message = new MqttMessage(command.getBytes());
                //mqttClient.publish("sensor/control", message); // prima versione test (solo lidar) + legs-servo-motors
                //mqttClient.publish("lidar/control", message);
                //mqttClient.publish("gyroscope/control", message);
                mqttClient.publish("sensor/control", message);
                System.out.println("Sent control command: " + command);
            } catch (MqttException e) {
                System.out.println("Failed to send control command: " + e.getMessage());
            }
        }
    }
}