package com.example.tv.clientecho;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.TextView;

import java.net.URI;
import java.net.URISyntaxException;

/**
 * @class MainActivity
 * @brief Activité principale de l'application (Thread UI)
 */
public class MainActivity extends AppCompatActivity
{
    private static final String TAG = "MainActivity"; //!< le TAG de la classe pour les logs

    /**
     * @fn onCreate
     * @brief Création de l'activité principale
     */
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        try
        {
            ClientEcho client = new ClientEcho(new URI("ws://192.168.1.38:5000"));
            client.connect();
        }
        catch (URISyntaxException e)
        {
            e.printStackTrace();
            throw new RuntimeException(e);
        }
    }
}
