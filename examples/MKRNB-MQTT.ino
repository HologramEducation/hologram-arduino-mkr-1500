/*
  Adapted from ArduinoMqttClient - WiFi Simple Receive Callback

  This example connects to a MQTT broker and subscribes to a single topic.
  When a message is received it prints the message to the serial monitor,
  it uses the callback functionality of the library.

  This example also publishes a sensor value to a separate topic

  The circuit:
  - Arduino MKR 1000, MKR 1010 or Uno WiFi Rev.2 board

  This example code is in the public domain.
*/

#include <ArduinoMqttClient.h>
#include <MKRNB.h>

// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// PIN Number
const char PINNUMBER[]     = "";

// MQTT Broker and Port
const char broker[] = "iot.eclipse.org"; //
int        port     = 1883;

// Pub/Sub Topics - Make it something unique since we are using a public broker
const char subscribeTopic[]  = "hologram-YOUR-NAME/to"; 
const char publishTopic[]  = "hologram-YOUR-NAME/from";

// To connect with SSL/TLS:
// 1) Change NBClient to NBSSLClient.
// 2) Change port value from 1883 to 8883.
// 3) Change broker value to a server with a known SSL/TLS root certificate 
//    flashed in the WiFi module.

NBClient client;
GPRS gprs;
NB nbAccess;
MqttClient mqttClient(client);

int LED = 6; // Onboard LED

// Publish interval
long previousMillis = 0; 
long interval = 10000;

void setup() {

  pinMode(LED, OUTPUT); // Set built in LED pinMode
  
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to GSM and GPRS:
  Serial.print("Attempting to connect to GSM and GPRS");
   // connection state
  bool connected = false;

  // After starting the modem with GSM.begin()
  // attach the shield to the GPRS network with the APN, login and password
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("You're connected to the network");
  Serial.println();

  // You can provide a unique client ID, if not set the library uses Arduino-millis()
  
  // Each client can have a unique client ID
  // mqttClient.setId("clientId");

  // You can provide a username and password for authentication
  // mqttClient.setUsernamePassword("username", "password");

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // set the message receive callback
  mqttClient.onMessage(onMqttMessage);

  Serial.print("Subscribing to topic: ");
  Serial.println(subscribeTopic);
  Serial.println();

  // subscribe to a topic
  mqttClient.subscribe(subscribeTopic);

  // topics can be unsubscribed using:
  // mqttClient.unsubscribe(topic);

  Serial.print("Waiting for messages on topic: ");
  Serial.println(subscribeTopic);
  Serial.println();
}

void loop() {
  unsigned long currentMillis = millis(); 
  
  // call poll() regularly to allow the library to receive MQTT messages and
  // send MQTT keep alives which avoids being disconnected by the broker
  mqttClient.poll();

  // Enforce Interval
  if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;

        // Call a function that handles publishing message
        publishSensorVal();
      
  }
}

void publishSensorVal() {
    // read the first Analog pin
    int sensorVal = analogRead(0);     
    String speak = "The sensor val is " + String(sensorVal);
    
    // Publish our sensor value to the publish topic
    mqttClient.beginMessage(publishTopic);
    mqttClient.print(speak);
    mqttClient.endMessage();
}

void handleCommand(String cmd) {
  if (cmd == "ON"){
    digitalWrite(LED, HIGH);
  }else {
    digitalWrite(LED, LOW);
  }
}

void onMqttMessage(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.println("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // We want to read each character into a useable String
  String content = "";
  char character;
  
  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    character = (char)mqttClient.read();
    content.concat(character);
  }

  handleCommand(content); // This function does something with our incoming string

  Serial.println(content);
  Serial.println(); Serial.println(); // double spacing
}
