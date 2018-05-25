#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

const char* ssid = "Tesla IoT";
const char* password = "fsL6HgjN";

const char* mqttServer = "m13.cloudmqtt.com";
const int mqttPort = 16110;
const char* mqttUser = "umoogjgo";
const char* mqttPassword = "OamdgsvzmmOu";

WiFiClient espClient;
PubSubClient client(espClient);

String stringOne;

//---------------------------------------------------------------
//Our HTML webpage contents in program memory
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>
<h1>WiFi LED on off demo: 1</h1><br>
Ciclk to turn <a href="ledOff">LED ON</a><br>
Ciclk to turn <a href="ledOn">LED OFF</a><br>
<hr>
</center>

</body>
</html>
)=====";
//---------------------------------------------------------------
//On board LED Connected to GPIO2
#define LED 2  

//Declare a global object variable from the ESP8266WebServer class.
ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 Serial.println("You called root page");
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}

void handleLEDon() { 
 Serial.println("LED on page");
 digitalWrite(LED,HIGH); //LED is connected in reverse
 server.send(200, "text/html", "LED is ON"); //Send ADC value only to client ajax request
}

void handleLEDoff() { 
 Serial.println("LED off page");
 digitalWrite(LED,LOW); //LED off
 server.send(200, "text/html", "LED is OFF"); //Send ADC value only to client ajax request
}


void callback(char* topic, byte* payload, unsigned int length) {
  String a;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    a = String(a + (char)payload[i]);

     if(a == "ledOn"){   
    Serial.println ("het werkt");
    handleLEDon();
    Serial.println(1);
   }
  if( a == "ledOff"){
    handleLEDoff();
    Serial.println(2);
  }
  }
  

 
  Serial.println();
  Serial.println("-----------------------");
 
}

void setup() {
  stringOne= String ("ledOn");
  Serial.begin(115200);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
 
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }

    
  //Onboard LED port Direction output
  pinMode(LED,OUTPUT); 
  //Power on LED state off
  digitalWrite(LED,HIGH);
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/ledOn", handleLEDon); //as Per  <a href="ledOn">, Subroutine to be called
  server.on("/ledOff", handleLEDoff);

  server.begin();                  //Start server
  Serial.println("HTTP server started");
  
  }
 
  client.publish("esp/test", "Hello from ESP8266");
  client.subscribe("esp/test");
  client.subscribe("esp/connected");
  client.subscribe("145.24.238.62");
 
}
 

 
void loop() {
  client.loop();
   server.handleClient();          //Handle client requests
}
