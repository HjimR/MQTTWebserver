#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include <ESP8266WebServer.h>
//
//const char* ssid = "Tesla IoT";
//const char* password = "fsL6HgjN";

const char* ssid = "ZyXEL NBG-418N v2";
const char* password = "XJKUN47796";

const char* mqttServer = "m13.cloudmqtt.com";
//const int mqttPort = 16110;//standard mqtt port SSL = 26110 en websockets port(TLS only) = 36110
const int mqttPort = 26110;
const char* mqttUser = "umoogjgo";
const char* mqttPassword = "OamdgsvzmmOu";
//const char* fingerprint = "06 21 0c a8 11 8d 20 6a 86 79 e9 9f 93 53 c5 41 df 36 2f cf";
bool ledBool = false;
bool ldrBool = false;
int analogValue = 0;
int sensorMin = 1023;        // minimum sensor value
int sensorMax = 0;           // maximum sensor value
String myIP;

//WiFiClient espClient;
WiFiClientSecure espClient;
PubSubClient client(espClient);

String stringOne;

//---------------------------------------------------------------
//Our HTML webpage contents in program memory
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<center>
<h1>WiFi LED on off Buttons </h1><br> //led ON & OFF buttons html
Click to turn <a href="ledOff">LED OFF</a><br>
Click to turn <a href="ledOn">LED ON</a><br>
<hr>

<h2>LDR  on off Buttons </h2><br>  //LDR ON & OFF buttons html
Click to turn <a href="ldrOff">LDR OFF</a><br>
Click to turn <a href="ldrOn">LDR ON</a><br>
<hr>
</center>

</body>
</html>
)=====";
//---------------------------------------------------------------

#define LED 2  //D4 --> LED/relay/lamp
#define LDRPin A0 // analoog pin A0 LDR

//Globale object van esp8266webserver klas maken
ESP8266WebServer server(80); //Server on port 80

// handleRoot wordt gebruikt wanneer je de IP invoert in een browser

void handleRoot() {
 Serial.println("You called root page");
 String s = MAIN_page; //HTML inhoud lezen
 server.send(200, "text/html", s); //Webpage sturen
}

void handleLEDon() { 
 Serial.println("LED on page");
 digitalWrite(LED,HIGH); 
 server.send(200, "text/html", "LED is ON"); 
}

void handleLEDoff() { 
 Serial.println("LED off page");
 digitalWrite(LED,LOW); //LED off
 server.send(200, "text/html", "LED is OFF"); 
}

void handleLDRon(){ //LDR ON methode
 Serial.println("LDR on page");
 bool ldrBool = true;
 server.send(200, "text/html", "LDR is ON"); 
}

void handleLDRoff(){ //LDROff functie
 Serial.println("LDR on page");
 bool ldrBool = false;
 server.send(200, "text/html", "LDR is OFF"); 
 digitalWrite(LED,LOW); //LED off
}

void callback(char* topic, byte* payload, unsigned int length) {
  String a;
  myIP = WiFi.localIP().toString(); //lokale ip addres in String converten en die vervolgens in myIP zetten
  Serial.println("myIP: " + myIP);
  char __myIP[sizeof(myIP)+3]; // +3 voor de 3 puntjes in IP adres (voorbeeld 145.24.238.62), maar telt voor WiFi.localIP niet.
  myIP.toCharArray(__myIP,sizeof(__myIP));
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    a = String(a + (char)payload[i]);

     if(a == "ledOn"){   
    Serial.println ("Lamp is aan");
    handleLEDon();
    Serial.println(1);
   }
  if( a == "ledOff"){
    Serial.println("-----------------------");
    Serial.println("lamp is uit");
    handleLEDoff();
    Serial.println(2);
    }
   if(a == "ldrOn"){  
    Serial.println("-----------------------"); 
    Serial.println ("LDR is nu in gebruik");
    ldrBool = true;
   }
  if(a == "ldrOff"){
    Serial.println ("LDR is niet in gebruik");
    ldrBool = false;
    }
  if(a == __myIP){
    Serial.println("Hello from the other side");
    client.publish("connection",__myIP);
    if(ledBool == true){
      client.publish("server/on","On");
    }
    else{
      client.publish("server/on","Off");
    }
  }
  if(a == "LDR"){
    if(ldrBool == true){
      ldrBool = false;
    }
    else ldrBool = true;
    }
  }
  Serial.println();
  Serial.println("-----------------------");
 }

 void loopDaLDR(){
  analogValue = analogRead(LDRPin); // waarde uitlezen van LDR en die vervolgens gelijk stellen aan analogvalue. analogvalue heeft nu de waarde van ldr
  analogValue = map(analogValue, sensorMin, sensorMax, 0, 255);
  analogValue = constrain(analogValue, 0, 255);
  Serial.println("LDR active");
  Serial.println(analogValue);
  delay(500);
  if (analogValue >= 200){
    digitalWrite(LED,HIGH);
    delay(1000);
  }
  else if (analogValue <= 75){
    digitalWrite(LED,LOW);
    delay(1000);
   }
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

  //Dit wordt uitgeprint als WiFi verbonden wordt
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address van ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/ledOn", handleLEDon); //as Per  <a href="ledOn">, Subroutine to be called
  server.on("/ledOff", handleLEDoff);
  server.begin();                  //Start server
  Serial.println("HTTP server started");
  
  }
  myIP = WiFi.localIP().toString(); //lokale ip addres in String converten en die vervolgens in myIP zetten
  Serial.println("myIP: " + myIP);
  char __myIP[sizeof(myIP)+3]; // +3 voor de 3 puntjes in IP adres (voorbeeld 145.24.238.62), maar telt voor WiFi.localIP niet.
  myIP.toCharArray(__myIP,sizeof(__myIP));
 
  client.publish("esp/test", "Hello from ESP8266"); //stuur een message naar topic esp/test
  client.subscribe("esp/test"); //luister naar binnen komende berichten van topic esp/test
  client.subscribe("esp/connected"); // luisteren naar esp/connected
  client.subscribe("esp/on");
  client.subscribe(__myIP); //subscribe naar eigen ip
  client.publish("connection",__myIP);// publish IP naar topic connection ( die is verbonden met de server)
  client.publish("addDatabaseName","light");
  client.publish("addDatabaseIP",__myIP);
}
 

 
void loop() {
  client.loop();
   server.handleClient();          //Handle client requests
   if (ldrBool == true){
    loopDaLDR();
    }
}
