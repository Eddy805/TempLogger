#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"

#include "credentials.h"   // define WIFI and MQTT credentials here

#define DHTTYPE DHT22
#define DHTPIN D2

const char* ssid = WIFI_SSID;         // defined in WIFI_credentials.h
const char* password = WIFI_PASSWORD; //
const char* mqtt_server = "192.168.178.201";
// const char* mqtt_user = MQTT_USER;
// const char* mqtt_pass = MQTT_PASSWORD;

WiFiClient espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE);

char msg[1024] = {0};
char tmp[10] = {0};
char hum[10] = {0};
int  humstat;

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
    if (client.connect("ESP8266Client")) {
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
  //client.loop();

  Serial.println("Reading temperature");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(1000);
    return;
  }

  /* Publish temperature & humidity to MQTT */
  
  dtostrf(t, 0, 1, tmp);
  dtostrf(h, 0, 1, hum);
  if (h < 40) {
    humstat = 2;             // dry
  } else if (h >= 70) {
    humstat = 3;            // wet
  } else {
    humstat = 1;            // comfortable
  }
  sprintf(msg, "{\"idx\":18, \"nvalue\": 0, \"svalue\": \"%s;%s;%d\"}", tmp, hum, humstat);
  Serial.print("Publish message: ");
  Serial.println(msg);

  client.publish("domoticz/in", msg);

  Serial.println("Going to sleep for 5 minutes");
  
  ESP.deepSleep(5 * 60 * 1000000); // 5 minutes
  //ESP.deepSleep(10 * 1000000); // 10 seconds

  Serial.println("Sleep failed!");
  delay(2000);
}
