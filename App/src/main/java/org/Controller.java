package org;

import org.CoAP.CoAPClient;
import org.MQTT.MQTTCollector;
import org.Persistence.DataManager;
import org.Persistence.Entities.LidarReading;
import org.Persistence.Entities.MotorsCommand;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;



public class Controller {
    static MQTTCollector MQTTClient;
    private static CoAPClient coapClient;
    static DataManager dataManager;

    private static String COMMANDS = "\n\n-------JUNO Remote Control Application--------\n" +
            "explore       Start a walking session to explore the asteroid\n" +
            "getPosition   Get last reported position of the rover\n" +
            "labTest       Start a laboratory test session of the rover\n" +
            "lastLidar     Read last distances sensed by the LiDAR sensor\n" +
            "lastMotors    Read last command executed by the servo motors of the rover's legs\n" +
            "help          Show all commands\n" +
            "quit          Close controller\n";

    public static void main(String args[]) throws IOException, InterruptedException {

        coapClient = new CoAPClient();
        MQTTClient = new MQTTCollector();
        dataManager = DataManager.getInstance();

        System.out.println(COMMANDS);
        String command = "";
        BufferedReader input = new BufferedReader(new InputStreamReader(System.in));

        boolean runningSession = false;

        label:
        while(true) {
            command = input.readLine();
            switch (command) {
                case "quit":
                    if(runningSession) {
                        MQTTClient.stopRetrieving();
                        runningSession = false;
                    }
                    System.exit(0);
                case "help":
                    System.out.println(COMMANDS);
                    break;
                case "explore":
                    System.out.println("Starting a new automatic walking session...");
                    System.out.println("COMMAND AVAILABLE:");
                    System.out.println("stop          To stop the current session and return to main menu\\n");
                    MQTTClient.startRetrieving();
                    MQTTClient.sendControlCommand("start");
                    runningSession = true;
                    break;
                case "getPosition":
                    System.out.println("The last position reported from the rover is:");
                    System.out.println("Work in progress!");
                    break;
                case "labTest":
                    System.out.println("Starting a new laboratory test session...");
                    System.out.println("PRESS THE BUTTON OVER THE SENSOR TO DISABLE IT.");
                    MQTTClient.startRetrieving();
                    MQTTClient.sendControlCommand("test-session");
                    runningSession = true;
                    break;
                case "stop":
                    if(runningSession) {
                        MQTTClient.stopRetrieving();
                        MQTTClient.sendControlCommand("stop");
                        runningSession = false;
                    }
                    System.out.println("WALKING SESSION STOPPED.\n\n");
                    System.out.println(COMMANDS);
                    break;
                case "lastLidar":
                    LidarReading lastLidarReading = dataManager.getLastLidarReading();
                    if(lastLidarReading==null)
                        System.out.println("No records stored yet.");
                    else
                        System.out.println(lastLidarReading);
                    break;
                case "lastMotors":
                    MotorsCommand lastMotorsCommand = dataManager.getLastMotorsCommand();
                    if(lastMotorsCommand==null)
                        System.out.println("No records stored yet.");
                    else
                        System.out.println(lastMotorsCommand);
                    break;
                default:
                    System.out.println("Command not supported. Try the following:");
                    System.out.println(COMMANDS);
            }
        }
    }
}
