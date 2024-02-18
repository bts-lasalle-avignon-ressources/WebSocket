/**
 * @file src/main.cpp
 * @brief Programme principal
 * @author Thierry Vaira
 * @version 0.1
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <afficheur.h>

#define DEBUG

#define PORT 5000

#define ADRESSE_I2C_OLED 0x3c //!< Adresse I2C de l'OLED
#define BROCHE_I2C_SDA   21   //!< Broche SDA
#define BROCHE_I2C_SCL   22   //!< Broche SCL

WebSocketsServer webSocket = WebSocketsServer(PORT);
WiFiManager      wm;

Afficheur afficheur(ADRESSE_I2C_OLED, BROCHE_I2C_SDA,
                    BROCHE_I2C_SCL); //!< afficheur OLED SSD1306

/**
 * @brief Gestion des évènements
 *
 */
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length)
{
    switch(type)
    {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Déconnecté\n", num);
            break;
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connecté à l'adresse IP : %d.%d.%d.%d\n",
                          num,
                          ip[0],
                          ip[1],
                          ip[2],
                          ip[3]);
        }
        break;
        case WStype_TEXT:
            Serial.printf("< %s\n", payload);
            webSocket.sendTXT(num, payload);
            Serial.printf("> %s\n", payload);
            break;
    }
}

/**
 * @brief Initialise les ressources du programme
 *
 * @fn setup
 *
 */
void setup()
{
    Serial.begin(115200);
    while(!Serial)
        ;

    // Gestion de la connexion au WiFi
    WiFi.mode(WIFI_STA);
    // wm.resetSettings();
    wm.setTitle("Serveur Echo (websocket)");
    bool res;
    res = wm.autoConnect();
    if(!res)
    {
        Serial.println("Erreur de connexion WiFi !");
        // ESP.restart();
    }

    webSocket.begin();
    // Installation du gestionnaire d'évènement
    webSocket.onEvent(webSocketEvent);

    Serial.printf("Serveur Echo (websocket)");
    Serial.printf(WiFi.localIP().toString() + String(":") + String(PORT));

    afficheur.initialiser();
    String titre  = "Serveur websocket";
    String stitre = "=====================";
    afficheur.setTitre(titre);
    afficheur.setSTitre(stitre);
    afficheur.setMessageLigne(Afficheur::Ligne1,
                              WiFi.localIP().toString() + String(":") + String(PORT));
    afficheur.afficher();
}

/**
 * @brief Boucle infinie d'exécution du programme
 *
 * @fn loop
 *
 */
void loop()
{
    webSocket.loop();
}
