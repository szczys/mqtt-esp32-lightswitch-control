#include <WiFiUdp.h>
#include <WiFi.h>

#include <PubSubClient.h>

/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// Pin 11 has the LED on Teensy 2.0
// Pin 6  has the LED on Teensy++ 2.0
// Pin 13 has the LED on Teensy 3.0
// give it a name:
int led = 2;

const char* ssid     = "Redacted";
const char* password = "Redacted";

const char* mqtt_server = "192.168.1.135";

char last[4] = "Off";
#define LIGHTOFF 0
#define LIGHTON 1
#define LIGHTUNKNOWN 2

WiFiClient espClient;
PubSubClient client(espClient);

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
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

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
                
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  pinMode(0, INPUT);     
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  last[0] = payload[length-3]; last[1] = payload[length-2]; last[2] = payload[length-1]; last[3] = 0;
  Serial.print(last[0]);Serial.print(last[1]);Serial.print(last[2]);Serial.print(last[3]);
  if (interpretLightCmd() == LIGHTOFF) { digitalWrite(led,0); }
  else if (interpretLightCmd() == LIGHTON) { digitalWrite(led,1); }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("lighting/porchlight/status");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

uint8_t interpretLightCmd() {
  if (strcmp("Off",last) == 0) return LIGHTOFF;
  else if (strcmp(":On",last) == 0) return LIGHTON;
  else return LIGHTUNKNOWN;
}

// the loop routine runs over and over again forever:
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (digitalRead(0)==0) {
    //digitalWrite(led,1);
    if (interpretLightCmd() == LIGHTOFF) client.publish("lighting/porchlight/control", "On");
    else if (interpretLightCmd() > LIGHTOFF) client.publish("lighting/porchlight/control", "Off");
    
    while (digitalRead(0)==0) ;;
  }
  //digitalWrite(led,0);
  delay(50);
  /*
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
  */
}
