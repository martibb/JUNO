package org.MQTT;

import org.CoAP.CoAPClient;
import org.Persistence.DataManager;
import org.Persistence.Entities.LidarReading;
import org.Persistence.Entities.MotorsCommand;
import org.Persistence.Entities.Position;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

// MQTT Client class
public class MQTTCollector implements MqttCallback{
    private final String lidarTopic = "lidar";
    //private final String gyroscopeTopic = "gyroscope";
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
        //subscribeToTopic(gyroscopeTopic);
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

    public void messageArrived(String topic, MqttMessage message) throws Exception {
        byte[] payload = message.getPayload();
        try {
            JSONObject sensorMessage = (JSONObject) JSONValue.parseWithException(new String(payload));
            if(topic.equals(lidarTopic))
            {
                System.out.println(sensorMessage);

                int frontDistance = Integer.parseInt(sensorMessage.get("distance_front").toString());
                int rightDistance = Integer.parseInt(sensorMessage.get("distance_right").toString());
                int leftDistance = Integer.parseInt(sensorMessage.get("distance_left").toString());
                LidarReading newLidarRecord = new LidarReading(frontDistance, rightDistance, leftDistance);
                dataManager.insertLidarReading(newLidarRecord);

                MotorsCommand newMotorsRecord = new MotorsCommand(newLidarRecord);
                dataManager.insertMotorsCommand(newMotorsRecord);

                position.updatePosition(newMotorsRecord);
                if(!testRunning)
                    dataManager.insertPosition();

                int newDirection = newMotorsRecord.getNewDirection();
                int stepSize = newMotorsRecord.getStepSize();
                String PostPayload = "{ \"direction\": " + newDirection + ", \"angle\": " + stepSize + " }";
                CoAPClient.servoMotorsPostRequest(CoAPClient.getServoMotorsUri(), PostPayload);
            } else {
                System.out.printf("Unknown topic: [%s] %s%n", topic, new String(payload));
            }
        } catch (ParseException e) {
            System.out.printf("Received badly formatted message: [%s] %s%n", topic, new String(payload));
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
                mqttClient.publish("sensor/control", message); // prima versione test (solo lidar) + legs-servo-motors
                //mqttClient.publish("lidar/control", message);
                //mqttClient.publish("gyroscope/control", message);
                System.out.println("Sent control command: " + command);
            } catch (MqttException e) {
                System.out.println("Failed to send control command: " + e.getMessage());
            }
        }
    }
}