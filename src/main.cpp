#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define LED_GREEN 4
#define LED_YELLOW 0
#define LED_RED 2

const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

const char *MQTT_BROKER = "test.mosquitto.org";
const int MQTT_PORT = 1883;
const char *MQTT_TOPIC = "ppti2026/leds";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const unsigned long HEARTBEAT_INTERVAL_MS = 1000;
unsigned long lastHeartbeatMillis = 0;
bool heartbeatState = false;

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  digitalWrite(LED_GREEN, payload[0] -'0');
  digitalWrite(LED_YELLOW, payload[1] -'0');
  digitalWrite(LED_RED, payload[2] -'0');
  Serial.printf("Message on %s: %s\n", topic, message.c_str());
}

void connectWifi()
{
  Serial.printf("Connecting to WiFi %s", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMqtt()
{
  while (!mqttClient.connected())
  {
    Serial.print("Connecting to MQTT broker...");
    String clientId = "esp32-" + String(WiFi.macAddress());
    if (mqttClient.connect(clientId.c_str()))
    {
      Serial.println(" connected");
      mqttClient.subscribe(MQTT_TOPIC);
    }
    else
    {
      Serial.printf(" failed, rc=%d, retrying in 2s\n", mqttClient.state());
      delay(2000);
    }
  }
}

void setup()
{
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  Serial.begin(115200);

  connectWifi();

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
}

void loop()
{
  if (!mqttClient.connected())
  {
    connectMqtt();
  }
  mqttClient.loop();

  if (millis() - lastHeartbeatMillis >= HEARTBEAT_INTERVAL_MS)
  {
    lastHeartbeatMillis = millis();
    heartbeatState = !heartbeatState;
    digitalWrite(LED_RED, heartbeatState ? HIGH : LOW);
  }
}
