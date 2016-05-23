#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"

#include "WIFI_credentials.h"   // define WIFI_SSID and WIFI_PASSWORD here

#define DHTTYPE DHT22
#define DHTPIN D2

const char* ssid = WIFI_SSID;         // defined in WIFI_credentials.h
const char* password = WIFI_PASSWORD; //
const char* mqtt_server = "io.adafruit.com";
const char* mqtt_user = MQTT_USER;
const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

const int led = LED_BUILTIN;

long lastMsg = 0;
char msg[50] = {0};
char last_msg[50] = {0};

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void){
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);

  Serial.begin(9600);
  
  setup_wifi();
  
  dht.begin();
  Serial.println("DHT started.");

  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);  // we do not subscribe yet
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop(void){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 5000) {
    lastMsg = now;

    Serial.println("Reading temperature");
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (isnan(h) || isnan(t)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    
    dtostrf(t, 4, 1, msg);

    if (strcmp(last_msg, msg) != 0)
    {    
      Serial.print("Publish message: ");
      Serial.println(msg);
  
      client.publish("Eddy8/f/fridge-temp", msg);
      strcpy(last_msg, msg);
    } else {
      Serial.print("Skipping message, identical to last: ");
      Serial.println(msg);
    }
 
  }
}
