#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

//web updater
#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK  "your-password"
#endif
//web updater end

//for LED status
#include <Ticker.h>
Ticker ticker;
Ticker flipper;

const char* mqttServer = "m23.cloudmqtt.com";
const char* mqtt_Server = "192.168.1.107";
const int mqttPort = 18968;
const int mqtt_Port = 1883;
const char* mqttUser = "lnfzbvla";
const char* mqttPassword = "RseqZDqXWgbq";

///web updater
const char* host = "esp8266-webupdate";
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
//web updater end

WiFiClient espClient;
PubSubClient client(espClient);
//------------------------------------------Declare GPIO Pins--------------------------------------

const int lamp1 = 16;

const int lamp2 = 5;

const int lamp3 = 4;

const int lamp4 = 0;


//DIMMER

const int DI0 = 14; 
const int DI1 = 12; 
const int DI2 = 13; 
const int DI3 = 15; 


void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

//-------------------------------------------------Ticker start-------------------------------
void flip()
{
   ESP.restart();
}

 
  WiFiManager wifiManager;
  //reset settings - for testing
 // wifiManager.resetSettings();
 

//-------------------------------------------------Ticker end-------------------------------
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
   
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  //reset settings - for testing
  //wifiManager.resetSettings();

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //here  "AutoConnectAP"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi
  Serial.println("connected...yeey :)");
  ticker.detach();
  //keep LED on
  digitalWrite(BUILTIN_LED, LOW);

//MQTT Connection
client.setServer(mqttServer, mqttPort);
client.setCallback(callback);

while (!client.connected()) {
    Serial.println("Connecting to Online MQTT...");
 
    if (client.connect("espClient", mqttUser, mqttPassword )) {
 
      Serial.println("Connected to online MQTT");  
     
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 client.setServer(mqtt_Server, mqtt_Port);
   while (!client.connected()) {
    Serial.println("Connecting to Offline MQTT...");
 
    if (client.connect("espClient")) {
 
      Serial.println("Connected to offline MQTT");  

    flipper.attach(300, flip);
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
     
    }
  }
   
    }
  }

  //web updater
 
  WiFi.mode(WIFI_AP_STA);
 
  MDNS.begin(host);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
 
  //web updater end

 
  delay(4000);

  client.publish("rooms/lamps", "Hello Cinderella from ESP32");

      client.subscribe("room/lamp1");
      client.subscribe("room/lamp2");
      client.subscribe("room/lamp3");
      client.subscribe("room/lamp4");
      client.subscribe("room/lamps");
      client.subscribe("room/fanspeed");    
       
  //------------------------------------------------------Setup lamp pinMode--------------------------

  pinMode(lamp1, OUTPUT);
  pinMode(lamp2, OUTPUT);
  pinMode(lamp3, OUTPUT);
  pinMode(lamp4, OUTPUT);

digitalWrite(lamp1, HIGH);
digitalWrite(lamp2, HIGH);
digitalWrite(lamp3, HIGH);
digitalWrite(lamp4, HIGH);

//Dimmer
pinMode(DI0, OUTPUT);
pinMode(DI1, OUTPUT);
pinMode(DI2, OUTPUT);
pinMode(DI3, OUTPUT);

digitalWrite(DI0, HIGH);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, HIGH);

}
 
//void callback(String topic, byte* message, unsigned int length) {
void callback(char* topic, byte* message, unsigned int length){
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
//---------------------------------------------------------create message string----------------------
String messageTemp;
 
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
//---------------------------------------------------------Store message in messageTemp----------------------
    messageTemp += (char)message[i];
  }
 
  Serial.println();
  Serial.println("-----------------------");

//-----------------------------------------------------Check message and set lamp status-----------------------------

  //Lamp 1
  if(String(topic)=="room/lamp1"){
      Serial.print("Changing Room lamp1 to ");
      if(messageTemp == "ON"){
        digitalWrite(lamp1, LOW);
        // digitalWrite(BUILTIN_LED, HIGH);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
digitalWrite(lamp1, HIGH);
        Serial.print("Off");
      }
  }

//Lamp 2
    if(String(topic)=="room/lamp2"){
      Serial.print("Changing Room lamp2 to ");
      if(messageTemp == "ON"){
        digitalWrite(lamp2, LOW);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
        digitalWrite(lamp2, HIGH);
        Serial.print("Off");
      }
  }

  //Lamp 3
    if(String(topic)=="room/lamp3"){
      Serial.print("Changing Room lamp3 to ");
      if(messageTemp == "ON"){
        digitalWrite(lamp3, LOW);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
        digitalWrite(lamp3, HIGH);
        Serial.print("Off");
      }
  }

    //Lamp4
    if(String(topic)=="room/lamp4"){
      Serial.print("Changing Room lamp4 to ");
      if(messageTemp == "ON"){
        digitalWrite(lamp4, LOW);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
        digitalWrite(lamp4, HIGH);
        Serial.print("Off");
      }
  }

    //Lamps
    if(String(topic)=="room/lamps"){
      Serial.print("Changing Room lamps to ");
      if(messageTemp == "ON"){
        digitalWrite(lamp1, LOW);
        digitalWrite(lamp2, LOW);
        digitalWrite(lamp3, LOW);
        digitalWrite(lamp4, LOW);
        Serial.print("On");
      }
      else if(messageTemp == "OFF"){
        digitalWrite(lamp1, HIGH);
        digitalWrite(lamp2, HIGH);
        digitalWrite(lamp3, HIGH);
        digitalWrite(lamp4, HIGH);
        Serial.print("OFF");
      }
  }

 //fan speed
if(String(topic)=="room/fanspeed"){
      Serial.print("Changing fanspeed to ");
      if(messageTemp == "0"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, HIGH); 
        Serial.print("0%");
      }
    
     else if(messageTemp == "1"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, HIGH); 
        Serial.print("1%");
      }
      
     else if(messageTemp == "2"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, LOW);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, HIGH);
        Serial.print("2%");
      }
     else if(messageTemp == "3"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, LOW);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, HIGH); 
        Serial.print("3%");
      }
     else if(messageTemp == "4"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, LOW);
digitalWrite(DI3, HIGH);
        Serial.print("4%");
      }

     else if(messageTemp == "5"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, LOW);
digitalWrite(DI3, HIGH); 
        Serial.print("5%");
      }

     else if(messageTemp == "6"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, LOW);
digitalWrite(DI2, LOW);
digitalWrite(DI3, HIGH); 
        Serial.print("6%");
      }

     else if(messageTemp == "7"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, LOW);
digitalWrite(DI2, LOW);
digitalWrite(DI3, HIGH);
        Serial.print("7%");
      }


     else if(messageTemp == "8"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, LOW);
        Serial.print("8%");
      }
     else if(messageTemp == "9"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, LOW);
        Serial.print("9%");
      }
     else if(messageTemp == "10"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, LOW);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, LOW);
        Serial.print("10%");
      }
else if(messageTemp == "11"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, LOW);
digitalWrite(DI2, HIGH);
digitalWrite(DI3, LOW);
        Serial.print("11%");
      }
else if(messageTemp == "12"){
digitalWrite(DI0, HIGH);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, LOW);
digitalWrite(DI3, LOW);
        Serial.print("12%");
      }

else if(messageTemp == "13"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, HIGH);
digitalWrite(DI2, LOW);
digitalWrite(DI3, LOW);
        Serial.print("13%");
      }
else if(messageTemp == "14"){
digitalWrite(DI0, LOW);
digitalWrite(DI1, LOW);
digitalWrite(DI2, LOW);
digitalWrite(DI3, LOW);
        Serial.print("14%");
      }
      
  }


}

void loop() {
  // put your main code here, to run repeatedly:
client.loop();


 if (client.connect("ESP32Client", mqttUser, mqttPassword )) {
 
    } else {
      Serial.println("Could not connect to online MQTT - Restarting.......");
       ESP.restart();
    }
  //web updater
  httpServer.handleClient();
  MDNS.update();
    //web updater end

}
