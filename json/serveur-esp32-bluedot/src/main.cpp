/**
 * @file src/main.cpp
 * @brief Programme principal
 * @author Thierry Vaira
 * @version 0.1
 */
#include <Arduino.h>
#include <Preferences.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_TSL2591.h"
#include <Adafruit_BME280.h>
#include <afficheur.h>
#include <cmath>

#define DEBUG

#define PORT                5000 //!< définit le numéro de port d'écoute du serveur
#define PERIODE_ACQUISITION 5000 //!< période d'acquisition par défaut en ms pour la sonde
#define TAILLE_JSON         256  //!< définit la taille en octets max pour un document JSON

#define GPIO_LED_ROUGE   17   //!< Led bicolore (rouge relié sur GPIO 17)
#define GPIO_LED_VERTE   16   //!< Led bicolore (vert relié sur GPIO 16)
#define ADRESSE_I2C_OLED 0x3c //!< Adresse I2C de l'OLED
#define BROCHE_I2C_SDA   21   //!< Broche SDA
#define BROCHE_I2C_SCL   22   //!< Broche SCL

String           nomServeur = "serveur-esp32-bluedot"; //!< nom du serveur DNS
WebSocketsServer webSocket  = WebSocketsServer(PORT);  //!< websocket serveur
WiFiManager      wm;                                   //!< gestionnaire de connexion WiFi
Preferences      preferences;                          //!< pour le stockage interne
Adafruit_TSL2591 tsl(1234);                            //!< capteur d'éclairement lumineux
Adafruit_BME280  bme; //!< capteur de température, humidté et pression
Afficheur        afficheur(ADRESSE_I2C_OLED, BROCHE_I2C_SDA,
                    BROCHE_I2C_SCL); //!< afficheur OLED SSD1306

StaticJsonDocument<TAILLE_JSON> documentJSON;       //!< données JSON
bool                            tslDetecte = false; //!< TSL2591 présent
bool                            bmeDetecte = false; //!< BME280 présent
uint16_t                        luminosite = 0;     //!< valeur de l'éclairement lumineux en lux
float                           humidite   = 0.f;   //!< valeur de l'humidité en %
float    temperature = -273.f;                      //!< valeur de la température en degré Celcius
float    pression    = 0.f;                         //!< valeur de la pression atmosphérique en hPa
uint32_t periode     = PERIODE_ACQUISITION; //!< période d'acquisition des valeurs en millisecondes
uint32_t tempsPrecedent = 0;                //!< temps de la dernère acquisition en millisecondes

// Fonctions
void   demarrer();
bool   estEcheance(unsigned long intervalle);
void   acquerirMesures();
void   afficherMesures();
void   envoyerMesures();
void   envoyerEtat();
bool   lireEtat();
void   commanderLed(uint8_t led, bool etat);
void   envoyerPeriode();
double arrondir(double value, double precision = 0.01);

/**
 * @brief Gestion des évènements
 */
void traiterEvenementWebsocket(uint8_t numero, WStype_t type, uint8_t* payload, size_t length)
{
    switch(type)
    {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Déconnecté\n", numero);
            break;
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(numero);
            Serial.printf("[%u] Connecté à l'adresse IP : %d.%d.%d.%d\n",
                          numero,
                          ip[0],
                          ip[1],
                          ip[2],
                          ip[3]);
            envoyerEtat();
            envoyerPeriode();
            envoyerMesures();
        }
        break;
        case WStype_TEXT:
            Serial.printf("[%u] < %s\n", numero, payload);
            DeserializationError erreur = deserializeJson(documentJSON, payload);
            if(!erreur)
            {
                JsonObject objetJSON = documentJSON.as<JsonObject>();
                if(objetJSON.containsKey("etat"))
                {
                    bool etat = (documentJSON["etat"].as<String>() == "on") ||
                                (documentJSON["etat"].as<String>() == "1") ||
                                (documentJSON["etat"].as<String>() == "true");
                    if(etat != lireEtat())
                    {
                        preferences.putBool("etat", etat);
                        if(etat)
                        {
                            digitalWrite(GPIO_LED_VERTE, HIGH);
                            digitalWrite(GPIO_LED_ROUGE, LOW);
                        }
                        else
                        {
                            digitalWrite(GPIO_LED_VERTE, LOW);
                            digitalWrite(GPIO_LED_ROUGE, HIGH);
                        }
                        envoyerEtat();
                    }
                }
                else if(objetJSON.containsKey("periode"))
                {
                    uint32_t nouvellePeriode = documentJSON["periode"].as<uint32_t>();
                    if(nouvellePeriode != periode)
                    {
                        preferences.putULong("periode", periode);
                        envoyerPeriode();
                    }
                }
            }
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

    preferences.begin("eeprom", false); // false pour r/w

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
    MDNS.begin(nomServeur.c_str());

    webSocket.begin();
    // Installation du gestionnaire d'évènement
    webSocket.onEvent(traiterEvenementWebsocket);

    Serial.println("Serveur Wheather-Bluedot (websocket)");
    Serial.println(WiFi.localIP().toString() + String(":") + String(PORT));

    tslDetecte = tsl.begin();
    if(tslDetecte)
    {
        tsl.setGain(TSL2591_GAIN_LOW); // 1x gain (bright light)
        // tsl.setGain(TSL2591_GAIN_MED);      // 25x gain
        // tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
        // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
        tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);
    }
    bmeDetecte = bme.begin();

    pinMode(GPIO_LED_ROUGE, OUTPUT);
    pinMode(GPIO_LED_VERTE, OUTPUT);

    afficheur.initialiser();
    String titre  = "Serveur websocket";
    String stitre = "=====================";
    afficheur.setTitre(titre);
    afficheur.setSTitre(WiFi.localIP().toString() + String(":") + String(PORT));
    afficheur.afficher();

    demarrer();
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

    if(estEcheance(periode))
    {
        acquerirMesures();
        afficherMesures();
        envoyerMesures();
    }
}

void demarrer()
{
    bool etat = preferences.getBool("etat", true);
    Serial.print("état : ");
    if(etat)
    {
        digitalWrite(GPIO_LED_VERTE, HIGH);
        digitalWrite(GPIO_LED_ROUGE, LOW);
        Serial.println("on");
    }
    else
    {
        digitalWrite(GPIO_LED_VERTE, LOW);
        digitalWrite(GPIO_LED_ROUGE, HIGH);
        Serial.println("off");
    }

    periode = preferences.getULong("periode", PERIODE_ACQUISITION);
    Serial.print("période : ");
    Serial.print(periode);
    Serial.println(" ms");

    acquerirMesures();
    afficherMesures();
}

bool estEcheance(unsigned long intervalle)
{
    unsigned long temps = millis();
    if(temps - tempsPrecedent >= intervalle)
    {
        tempsPrecedent = temps;
        return true;
    }
    return false;
}

void acquerirMesures()
{
    if(tslDetecte)
    {
        luminosite = tsl.getLuminosity(TSL2591_VISIBLE);
    }
    if(bmeDetecte)
    {
        temperature = bme.readTemperature();
        humidite    = bme.readHumidity();
        pression    = bme.readPressure() / 100.0F; // hPa
    }
}

void afficherMesures()
{
    char strMessageDisplay[32];

    if(bmeDetecte)
    {
        sprintf(strMessageDisplay, "Température : %.1f °C", temperature);
        afficheur.setMessageLigne(Afficheur::Ligne1, strMessageDisplay);
        sprintf(strMessageDisplay, "Humidité       : %u %%", (uint16_t)humidite);
        afficheur.setMessageLigne(Afficheur::Ligne2, strMessageDisplay);
        sprintf(strMessageDisplay, "Pression       : %u hPa", (uint16_t)pression);
        afficheur.setMessageLigne(Afficheur::Ligne3, strMessageDisplay);
    }
    if(tslDetecte)
    {
        sprintf(strMessageDisplay, "Eclairement : %u lux", luminosite);
        afficheur.setMessageLigne(Afficheur::Ligne4, strMessageDisplay);
    }
    if(tslDetecte || bmeDetecte)
    {
        afficheur.afficher();
    }
}

void envoyerMesures()
{
    char buffer[TAILLE_JSON];

    documentJSON.clear();
    if(tslDetecte)
    {
        luminosite                 = tsl.getLuminosity(TSL2591_VISIBLE);
        documentJSON["luminosite"] = luminosite;
    }
    if(bmeDetecte)
    {
        documentJSON["temperature"] = arrondir(temperature, 0.01);
        documentJSON["humidite"]    = (uint16_t)humidite;
        documentJSON["pression"]    = (uint16_t)pression;
    }
    if((tslDetecte || bmeDetecte) && lireEtat())
    {
        documentJSON["type"] = "mesure";
        serializeJson(documentJSON, buffer);
        webSocket.broadcastTXT(buffer);
        Serial.printf("[broadcast] > %s\n", buffer);
    }
}

void envoyerEtat()
{
    char buffer[TAILLE_JSON];

    documentJSON.clear();
    documentJSON["type"] = "etat";
    documentJSON["etat"] = (digitalRead(GPIO_LED_VERTE) ? "on" : "off");
    serializeJson(documentJSON, buffer);
    webSocket.broadcastTXT(buffer);
    Serial.printf("[broadcast] > %s\n", buffer);
}

bool lireEtat()
{
    return (digitalRead(GPIO_LED_VERTE) ? true : false);
}

void commanderLed(uint8_t led, bool etat)
{
    if(etat)
    {
        digitalWrite(led, HIGH); // on
    }
    else
    {
        digitalWrite(led, LOW); // off
    }
}

void envoyerPeriode()
{
    char buffer[TAILLE_JSON];

    documentJSON.clear();
    documentJSON["type"]    = "periode";
    documentJSON["periode"] = periode;
    serializeJson(documentJSON, buffer);
    webSocket.broadcastTXT(buffer);
    Serial.printf("[broadcast] > %s\n", buffer);
}

double arrondir(double value, double precision /*= 0.01*/)
{
    return std::round(value / precision) * precision;
}