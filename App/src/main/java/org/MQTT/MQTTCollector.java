package org.MQTT;

import org.CoAPClient;
import org.JUNODB;
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
    private final String broker = "tcp://[fd00::1]:1883";
    private final String clientId = "JavaApp";
    private final String lidarTopic = "lidar";
    private MqttClient mqttClient = null;

    //-----------------------------------------------------------------------*/

    public MQTTCollector() throws InterruptedException {
        do {
            try {
                this.mqttClient = new MqttClient(this.broker,this.clientId);
                System.out.println("Connecting to broker: "+ broker);

                this.mqttClient.setCallback(this);
                this.mqttClient.connect();

                this.mqttClient.subscribe(this.lidarTopic);
                System.out.println("Subscribed to topic: " + lidarTopic);

            }catch(MqttException me) {
                System.out.println("I could not connect, Retrying ...");
            }
        }while(!this.mqttClient.isConnected());
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
            }catch(MqttException me) {
                System.out.println("I could not connect");
            } catch (InterruptedException e) {
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
                // Supponiamo che il messaggio contenga i campi: "new sample distance" e "obstacle (boolean)"
                // Dovrò distinguere tra ostacolo e non ostacolo e tra inclinazione pericolosa e non
                // Dovrò passare i comandi agli attuatori con una richiesta POST
                // Memorizzo infine le info sul db
            } else {
                System.out.println(String.format("Unknown topic: [%s] %s", topic, new String(payload)));
            }
        } catch (ParseException e) {
            System.out.println(String.format("Received badly formatted message: [%s] %s", topic, new String(payload)));
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void deliveryComplete(IMqttDeliveryToken token) {
        System.out.println("I love java -.-\n");
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

    /*public String getRoverStatus() {
        return this.carStatus;
    }*/
}