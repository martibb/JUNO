package org.MQTT;

import org.CoAP.CoAPClient;
import org.Persistence.DataManager;
import org.Persistence.Entities.LidarReading;
import org.Persistence.Entities.MotorsCommand;
import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttClient;
import org.eclipse.paho.client.mqttv3.MqttException;
import org.eclipse.paho.client.mqttv3.MqttMessage;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.ParseException;

import java.util.AbstractMap;
import java.util.Map;

// MQTT Client class
public class MQTTCollector implements MqttCallback{
    private final String lidarTopic = "lidar";
    private MqttClient mqttClient = null;
    private DataManager dataManager = DataManager.getInstance();

    //-----------------------------------------------------------------------*/

    public MQTTCollector() throws InterruptedException {
        do {
            try {
                String broker = "tcp://[fd00::1]:1883";
                String clientId = "JavaApp";
                this.mqttClient = new MqttClient(broker, clientId);
                System.out.println("Connecting to broker: "+ broker);

                this.mqttClient.setCallback(this);
                this.mqttClient.connect();

            }catch(MqttException me) {
                System.out.println("I could not connect, Retrying ...");
            }
        }while(!this.mqttClient.isConnected());
    }

    public void startRetrieving() {
        subscribeToTopic(lidarTopic);
    }

    public void subscribeToTopic(String topic) {
        try {
            this.mqttClient.subscribe(topic);
            System.out.println("Subscribed to topic: " + topic);
        } catch (MqttException e) {
            System.out.println("Failed to subscribe to topic: " + topic);
            e.printStackTrace();
        }
    }

    public void stopRetrieving() {
        unsubscribeFromTopic(lidarTopic);
    }

    public void unsubscribeFromTopic(String topic) {
        try {
            this.mqttClient.unsubscribe(topic);
            System.out.println("Unsubscribed from topic: " + topic);
        } catch (MqttException e) {
            System.out.println("Failed to unsubscribe from topic: " + topic);
            e.printStackTrace();
        }
    }

    public void connectionLost(Throwable cause) {
        System.out.println("Connection is broken: " + cause);
        int timeWindow = 3000;
        while (!this.mqttClient.isConnected()) {
            try {
                System.out.println("Trying to reconnect in " + timeWindow/1000 + " seconds.");
                Thread.sleep(timeWindow);
                System.out.println("Reconnecting ...");
                timeWindow *= 2;
                this.mqttClient.connect();

                this.mqttClient.subscribe(lidarTopic);
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
                // Supponiamo che il messaggio contenga il campo "distance"
                // Dovrò distinguere tra ostacolo e non ostacolo e tra inclinazione pericolosa e non
                // Dovrò passare i comandi agli attuatori con una richiesta POST
                // Memorizzo infine le info sul db

                int frontDistance = Integer.parseInt(sensorMessage.get("distance_front").toString());
                int rightDistance = Integer.parseInt(sensorMessage.get("distance_right").toString());
                int leftDistance = Integer.parseInt(sensorMessage.get("distance_left").toString());
                LidarReading newLidarRecord = new LidarReading(frontDistance, rightDistance, leftDistance);
                dataManager.insertLidarReading(newLidarRecord);

                Map.Entry<Integer, Integer> motorsCommands = determineDirection(frontDistance, rightDistance, leftDistance);
                int newDirection = motorsCommands.getKey();
                int stepSize = motorsCommands.getValue()/2;
                if (newDirection == 4 || newDirection == 3) {
                    stepSize = -stepSize;
                }
                MotorsCommand newMotorsRecord = new MotorsCommand(newDirection, stepSize);
                dataManager.insertMotorsCommand(newMotorsRecord);

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

    public Map.Entry<Integer, Integer> determineDirection(int frontDistance, int rightDistance, int leftDistance) {
        final int FORWARD = 1;
        final int RIGHT = 2;
        final int BACKWARD = 3;
        final int LEFT = 4;
        final int MAX_DISTANCE = 100;
        final int MIN_DISTANCE = 20;

        int chosenDirection = FORWARD;
        int chosenDistance = frontDistance;

        // If in every direction the lidar senses a distance below 20 meters, the rover is in a dead end
        // so the best decision is to walk backward
        if (frontDistance < MIN_DISTANCE && rightDistance < MIN_DISTANCE && leftDistance < MIN_DISTANCE) {
            return new AbstractMap.SimpleEntry<>(BACKWARD, Math.max(frontDistance, Math.max(rightDistance, leftDistance)));
        }

        // If all directions have equal associated distances there aren't obstacle at all and the rover can go forward
        if (frontDistance == rightDistance && frontDistance == leftDistance) {
            return new AbstractMap.SimpleEntry<>(FORWARD, frontDistance);
        }

        // If the front sensed distance is below the maximum than the sensor can sense, there might be an obstacle,
        // so the rover moves right or left, depending on the most free path
        if (frontDistance < MAX_DISTANCE) {
            if (rightDistance > leftDistance) {
                chosenDirection = RIGHT;
                chosenDistance = rightDistance;
            } else {
                chosenDirection = LEFT;
                chosenDistance = leftDistance;
            }
        }

        return new AbstractMap.SimpleEntry<>(chosenDirection, chosenDistance);
    }

    public void deliveryComplete(IMqttDeliveryToken token) {
        System.out.println("Delivery completed.\n");
    }

    public void sendControlCommand(String command) {
        if (mqttClient.isConnected()) {
            try {
                MqttMessage message = new MqttMessage(command.getBytes());
                mqttClient.publish("sensor/control", message);
                System.out.println("Sent control command: " + command);
            } catch (MqttException e) {
                System.out.println("Failed to send control command: " + e.getMessage());
            }
        }
    }
}