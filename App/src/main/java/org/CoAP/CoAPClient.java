package org.CoAP;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import org.eclipse.californium.core.CoapClient;
import org.eclipse.californium.core.CoapResponse;
import org.eclipse.californium.core.coap.MediaTypeRegistry;

public class CoAPClient {
    private static String servoMotorsUri;
    private static String harpoonsUri;

    public CoAPClient() {
        String filePath = "actuatorsURI.txt";
        readActuatorURIs(filePath);
    }

    private void readActuatorURIs(String filePath) {
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            servoMotorsUri = reader.readLine();
            harpoonsUri = reader.readLine();
        } catch (IOException e) {
            System.out.println("Failed to read actuator URIs from file");
            e.printStackTrace();
        }
    }

    public static String getServoMotorsUri() {
        return ("coap://["+servoMotorsUri+"]/movement");
    }

    public static String getHarpoonsUri() {
        return ("coap://["+harpoonsUri+"]/anchoring");
    }

    public static void actuatorsPostRequest(String uri, String payload) {
        CoapClient coapClient = new CoapClient(uri);
        coapClient.post(String.valueOf(payload), MediaTypeRegistry.APPLICATION_JSON);
    }
}