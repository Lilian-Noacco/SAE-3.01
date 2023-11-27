#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>
#include <BearSSLHelpers.h>
#include <CertStoreBearSSL.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiGratuitous.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiType.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiServer.h>
#include <WiFiServerSecure.h>
#include <WiFiServerSecureBearSSL.h>
#include <WiFiUdp.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.

const char* mqtt_server = "broker.hivemq.com"; //adresse du broker
const int buttonPin = 13; // Pin où le bouton-poussoir est connecté
const char* deviceName = "esp1"; // Nom de cette appareil, doit être identique dans la trame JSON

bool ledState = HIGH; // État actuel de la LED
bool lastLedState = HIGH; // État précédent de la LED

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;



WiFiManager wm;
//wm custom_mqtt_server("server", "mqtt server", mqtt_server, 40);
//WiFiManager.addParameter(&custom_mqtt_server);

void setup() {
  Serial.begin(115200);
    // WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:

    //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around


    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    // wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result
    wm.setConfigPortalTimeout(60);
    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("APPrise1"); // nom de l'ap et son mot de passe lors de l'installation de la prise

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(13, INPUT_PULLUP);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  // Convert the payload to a string
  payload[length] = '\0'; // Null-terminate the payload
  String payloadString = String((char*)payload);

  Serial.println(payloadString);

  // Parse the JSON payload
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, payloadString);

  // Check for parsing errors
  if (error) {
    Serial.print(F("Failed to parse JSON, error: "));
    Serial.println(error.c_str());
    return;
  }

  // Check if the JSON object has a key named "device"
  if (doc.containsKey("device")) {
    // Get the value of the "device" key
    String receivedDeviceName = doc["device"];

    // Check if the received device name matches the current device name
    if (receivedDeviceName == deviceName) {
      // Check if the JSON object has a key named "ledState"
      if (doc.containsKey("ledState")) {
        // Get the value of the "ledState" key
        String receivedLedState = doc["ledState"];

        // Update the LED state if the received LED state is valid
        if (receivedLedState == "on") {
          digitalWrite(BUILTIN_LED, LOW); // Turn the LED on
          ledState = LOW; // Update the local LED state
        } else if (receivedLedState == "off") {
          digitalWrite(BUILTIN_LED, HIGH); // Turn the LED off
          ledState = HIGH; // Update the local LED state
        } else {
          Serial.println("Invalid LED state received");
        }
      }
    } else {
      Serial.println("Received message for a different device");
    }
  }
}

void reconnect() {
  wm.setConfigPortalTimeout(10);
  wm.setConfigPortalBlocking(false);
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...

      
      client.publish("dlj/prisetest/prises", "hello world"); //topic + message
      // ... and resubscribe
      client.subscribe("dlj/prisetest/prises"); //topic

      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    
    while (!wm.autoConnect()){
      Serial.println("Attempting reconnection...");
      delay(1000);
    }

  }
}
void loop() {
  wm.process();
    // put your main code here, to run repeatedly:   
    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Vérifiez l'état du bouton-poussoir
  int buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  // Mettez à jour l'état de la LED en fonction de l'état du bouton-poussoir
  if (buttonState == LOW) {
    ledState = !ledState; // Inversez l'état de la LED si le bouton est enfoncé
    while (buttonState == LOW){
      buttonState = digitalRead(buttonPin);
    }
    
  }
  if (ledState == HIGH){
    digitalWrite(BUILTIN_LED, HIGH);
  }else if (ledState == LOW){
    digitalWrite(BUILTIN_LED, LOW);
  }
  // Vérifiez si l'état de la LED a changé
  if (ledState != lastLedState) {
    
    // Créez un objet JSON pour le message
    DynamicJsonDocument doc(1024);
    doc["deviceupdate"] = deviceName; // Ajoutez le nom de la prise à l'objet JSON
    doc["ledState"] = (ledState == LOW) ? "on" : "off"; // Utilisez l'état actuel de la LED
    
    // Convertissez l'objet JSON en chaîne
    String jsonString;
    serializeJson(doc, jsonString);

    // Affichez et publiez le message JSON
    Serial.print("Publish message: ");
    Serial.println(jsonString);
    
    client.publish("dlj/prisetest/prises", jsonString.c_str());

    // Mettez à jour l'état précédent de la LED
    lastLedState = ledState;
  }
  
}
