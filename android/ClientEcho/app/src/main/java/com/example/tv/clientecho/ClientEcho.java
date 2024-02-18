package com.example.tv.clientecho;

import java.net.URI;
import java.net.URISyntaxException;
import org.java_websocket.client.WebSocketClient;
import org.java_websocket.handshake.ServerHandshake;

public class ClientEcho extends WebSocketClient
{
    public ClientEcho(URI serverURI)
    {
        super(serverURI);
    }

    @Override
    public void onOpen(ServerHandshake handshakedata) {
        System.out.println("Connexion établie");
        String message = "Hello world!";
        send(message);
        System.out.println("> " + message);
    }

    @Override
    public void onMessage(String message)
    {
        System.out.println("< " + message);
    }

    @Override
    public void onClose(int code, String reason, boolean remote)
    {
        System.out.println(
                "Connexion fermée " + (remote ? "distant" : "local") + " -> code : " + code + " reason : "
                        + reason);
    }

    @Override
    public void onError(Exception ex) {
        ex.printStackTrace();
    }
}