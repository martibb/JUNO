package org;

import org.MQTT.MQTTCollector;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.SocketException;



public class Controller {
    private static CoAPClient coapClient;

    private static String COMMANDS = "--------JUNO Remote Control Application--------\n" +
            "explore       Start a walking session to explore the asteroid\n" +
            "getPosition   Get last reported position of the rover\n" +
            "labTest       Start a laboratory test session of the rover\n" +
            "help          Show all commands\n" +
            "quit          Close controller\n";

    public static void main(String args[]) throws IOException, InterruptedException {
        MQTTCollector client = new MQTTCollector();
        coapClient = new CoAPClient();

        System.out.println(COMMANDS);
        String command = "";
        BufferedReader input = new BufferedReader(new InputStreamReader(System.in));

        label:
        while(true) {
            command = input.readLine();
            switch (command) {
                case "quit":
                    break label;
                case "help":
                    System.out.println(COMMANDS);
                    break;
                case "explore":
                    System.out.println("Starting a new automatic walking session...");
                    System.out.println("Currently working on that!");
                    break;
                case "getPosition":
                    System.out.println("The last position reported from the rover is:");
                    System.out.println("Work in progress!");
                    break;
                case "labTest":
                    System.out.println("Starting a new laboratory test session...");
                    System.out.println("Work in progress!");
                    break;
                default:
                    System.out.println("Command not supported. Try the following:");
                    System.out.println(COMMANDS);
            }
        }
    }
}
