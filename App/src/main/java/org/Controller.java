package org;

import org.CoAP.CoAPClient;
import org.MQTT.MQTTCollector;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;



public class Controller {
    private static CoAPClient coapClient;

    private static String COMMANDS = "--------JUNO Remote Control Application--------\n" +
            "explore       Start a walking session to explore the asteroid\n" +
            "getPosition   Get last reported position of the rover\n" +
            "labTest       Start a laboratory test session of the rover\n" +
            "help          Show all commands\n" +
            "quit          Close controller\n";

    static MQTTCollector MQTTClient;

    public static void main(String args[]) throws IOException, InterruptedException {

        coapClient = new CoAPClient();
        MQTTClient = new MQTTCollector();

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
                    System.out.println("Work in progress!");
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
                default:
                    System.out.println("Command not supported. Try the following:");
                    System.out.println(COMMANDS);
            }
        }
    }
}
