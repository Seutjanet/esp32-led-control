#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

const char* ssid = "ALHN-34E0";
const char* password = "6mKs4t8PrA";

const char* mqtt_server = "21d29e3f1c8b4623a92f309b0fbe53c4.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "espuser";
const char* mqtt_pass = "Esp1234@@";

WiFiClientSecure espClient;
PubSubClient client(espClient);

#define LIGHT_PIN 2
#define FAN_PIN   4
#define TV_PIN    5
#define PUMP_PIN  18
#define GATE_PIN  19

void setupPins() {
  pinMode(LIGHT_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(TV_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(GATE_PIN, OUTPUT);

  digitalWrite(LIGHT_PIN, LOW);
  digitalWrite(FAN_PIN, LOW);
  digitalWrite(TV_PIN, LOW);
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(GATE_PIN, LOW);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg;
  for (int i = 0; i < length; i++) msg += (char)payload[i];

  Serial.println("Received: " + msg);

  int sep = msg.indexOf(':');
  if (sep == -1) return;

  String device = msg.substring(0, sep);
  String state  = msg.substring(sep + 1);

  bool on = (state == "ON");

  if (device == "light") digitalWrite(LIGHT_PIN, on);
  if (device == "fan")   digitalWrite(FAN_PIN, on);
  if (device == "tv")    digitalWrite(TV_PIN, on);
  if (device == "pump")  digitalWrite(PUMP_PIN, on);
  if (device == "gate")  digitalWrite(GATE_PIN, on);

  client.publish("home/status", (device + ":" + state).c_str());
}

void connectMQTT() {
  while (!client.connected()) {
    Serial.println("Connecting MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("MQTT Connected");
      client.subscribe("home/control");
      client.publish("home/status", "online");
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setupPins();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  espClient.setInsecure();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) connectMQTT();
  client.loop();
}
