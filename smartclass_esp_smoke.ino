
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQ2.h>

const char* ssid = "Jack";
const char* password = "namakamu";
const char* mqtt_server = "192.168.100.79";
const char* mqttUser = "smartclass";
const char* mqttPassword = "jone";
const char* topic = "in_smoke_topic";
char* str_smoke = "";


WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

int pin = A0;
float lpg, co, smoke;

MQ2 mq2(pin);


void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  mq2.begin();
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttUser,mqttPassword)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(topic, "0");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float calc_mq2(){
  float* values= mq2.read(true); //set it false if you don't want to print the values to the Serial
  
  // lpg = values[0];
  lpg = mq2.readLPG();
  // co = values[1];
  co = mq2.readCO();
  // smoke = values[2];
  smoke = mq2.readSmoke();
  return smoke;
}


void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    smoke = calc_mq2();
    dtostrf(smoke, 4, 2, str_smoke);
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "%s", str_smoke);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(topic, msg);
  }
}
