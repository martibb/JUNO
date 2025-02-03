package org;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import org.eclipse.californium.core.CoapClient;

public class CoAPClient {
    private String gateBarrierUri;
    private String monitorUri;

    public CoAPClient() {
        String filePath = "actuatorsURI.txt";
        readActuatorURIs(filePath);
    }

    private void readActuatorURIs(String filePath) {
        try (BufferedReader reader = new BufferedReader(new FileReader(filePath))) {
            // Leggo l'URI dei miei due attuatori
        } catch (IOException e) {
            System.out.println("Failed to read actuator URIs from file");
            e.printStackTrace();
        }
    }
}