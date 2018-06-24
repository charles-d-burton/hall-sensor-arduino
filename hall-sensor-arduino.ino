#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>

const char* ssid = "prettyflyforawifi";
const char* password = "";
const int hallPin = 14;
const int ledPin = 0;
const char* mqtt_server = "mosquitto.localdomain";
long lastMsg = 0;

WiFiClient espClient;
PubSubClient client(espClient);
//WiFiUDP ntpUDP;

// By default 'time.nist.gov' is used with 60 seconds update interval and
// no offset
//NTPClient timeClient(ntpUDP);

// Setup and connect to the wifi
void setup_wifi() {
  delay(100);
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  //randomSeed(micros());
  Serial.println("");
  Serial.println("Wifi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Gateway: ");
  Serial.println(WiFi.gatewayIP());
}

//Reconnect to the MQTT broker
void reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
      setup_wifi();
  }                   
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
      client.publish("/homeassistant/devices/doorbell/status", "hello world");
      espClient.flush();
      // ... and resubscribe
      client.subscribe("/homeassistant/devices/doorbell/receive");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Process messages incoming from the broker
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(hallPin, INPUT);
  Serial.begin(19200);
  setup_wifi();
  
  //timeClient.begin();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
 
void loop() {
  digitalWrite(ledPin, LOW);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (digitalRead(hallPin) == LOW) {
    //digitalWrite(ledPin, HIGH);
    generateMessage(); 
    //client.publish("/homeassistant/devices/doorbell", msg);
    //espClient.flush();
    
  }
  delay(100); //Add in a delay so it doesn't send messages extremely rapidly
}

void generateMessage() {
  //timeClient.update();
  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    Serial.println("{\"sensor\":\"doorbell\",\"value\":1}");
    client.publish("/homeassistant/devices/doorbell/message", "{\"sensor\":\"doorbell\",\"value\":1}");
    espClient.flush();
  }
}

