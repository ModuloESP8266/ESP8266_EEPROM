#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
//#include <string>
//#include <cstring>

#define MQTT_SERVER_WAN "idirect.dlinkddns.com"

// WIFI ETB//
const char* ssid_1 = "Consola";
const char* pass_1 = "tyrrenal";

int ssid_add=0;
int pass_add=100;
int mqtt_server_add=200;
int mqtt_server_wan_add=300;
int topic_sub_add=400;
int topic_pub_add=500;
int topic_sub_default_add=600;

String ssid=" ";
String pass=" ";
String mqtt_server=" ";
String mqtt_server_wan=" ";
String topic_sub=" ";
String topic_pub=" ";
String topic_sub_default=" ";

//LED on ESP8266 GPIO2
const int light1= 0;
const int light2= 2;
const int Entrada_1= 12; // interruptor 1
const int Entrada_2= 14;  // interruptor 2



//*******  G R A B A R  EN LA  E E P R O M  ***********
void graba(int addr, String a) {
  int i=0;
  int tamano = (a.length() + 1);
  Serial.println(tamano);
  char inchar[100];    //'30' Tamaño maximo del string
  a.toCharArray(inchar, tamano);
  EEPROM.write(addr, tamano);
  for (i = 0; i < tamano; i++) {
    addr++;
    EEPROM.write(addr, inchar[i]);
  }
   EEPROM.commit();
}

//*******  L E E R   EN LA  E E P R O M    **************
  String lee(int addr) {
  String nuevoString;
  int valor;
  int tamano = EEPROM.read(addr);
  for (int i = 0; i < tamano; i++) {
    addr++;
    valor = EEPROM.read(addr);
    nuevoString += (char)valor;
  }
  return nuevoString;
}



WiFiClient wifiClient;

PubSubClient client(MQTT_SERVER_WAN, 1883, callback, wifiClient);

void setup() {
  // put your setup code here, to run once:
 Serial.begin(115200);
 EEPROM.begin(4096);
  graba(ssid_add, "Consola");
  graba(pass_add, "tyrrenal");
  graba(mqtt_server_add, "192.168.0.106");
  graba(mqtt_server_wan_add, "idirect.dlinkddns.com");
  graba(topic_sub_add, "prueba/light1");
  graba(topic_pub_add, "prueba/light1/confirm");
  graba(topic_sub_default_add, "default");
  delay(1000);
  ssid=lee(ssid_add);
  pass=lee(pass_add);
  mqtt_server=lee(mqtt_server_add);
  mqtt_server_wan=lee(mqtt_server_wan_add);
  topic_sub=lee(topic_sub_add);
  topic_pub=lee(topic_pub_add);
  topic_sub_default=lee(topic_sub_default_add);;
  
  Serial.println(ssid.c_str());
  Serial.println(pass.c_str());
  Serial.println(mqtt_server);
  Serial.println(mqtt_server_wan);
  Serial.println(topic_sub);
  Serial.println(topic_pub);
  Serial.println(topic_sub_default);

  
  delay(200);
 WiFi.mode(WIFI_STA);
 
 //WiFi.begin(ssid.c_str(),pass.c_str());
  WiFi.begin(ssid_1,pass_1);
  reconnect();
 delay(250);

 
}


void loop() {
   if (!client.connected() && WiFi.status() == 3) {reconnect();}

  //maintain MQTT connection
  client.loop();
  //MUST delay to allow ESP8266 WIFI functions to run
  delay(10); 
  
}




void callback(char* topic, byte* payload, unsigned int length) {

  //convert topic to string to make it easier to work with
  String topicStr = topic; 

  //Print out some debugging info
  Serial.println("Callback update.");
  Serial.print("Topic: ");
  Serial.println(topicStr);

   if(topicStr == topic_pub){

      //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
        if(payload[0] == '1'){
          digitalWrite(light1, HIGH);
          client.publish(topic_pub.c_str(), "Light1 On");
          }
      //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
        else if (payload[0] == '0'){
          digitalWrite(light1, LOW);
          client.publish(topic_pub.c_str(), "Light1 Off");
        }
   }
  /*  if(topicStr == lightTopic2){

       //turn the light on if the payload is '1' and publish to the MQTT server a confirmation message
        if(payload[0] == '1'){
          digitalWrite(light2, HIGH);
          client.publish("prueba/light2/confirm", "Light2 On");
          }
      //turn the light off if the payload is '0' and publish to the MQTT server a confirmation message
        else if (payload[0] == '0'){
          digitalWrite(light2, LOW);
          client.publish("prueba/light2/confirm", "Light2 Off");
        }

      
    }*/
}

void reconnect() {
 int c=0;
  //attempt to connect to the wifi if connection is lost
  if(WiFi.status() != WL_CONNECTED){
    //debug printing
    Serial.print("Connecting to ");
    Serial.println(ssid_1);

    //loop while we wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      c++;
   }

    //print out some more debug once connected
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  //make sure we are connected to WIFI before attemping to reconnect to MQTT
  if(WiFi.status() == WL_CONNECTED){
  // Loop until we're reconnected to the MQTT server
    while (!client.connected()) {
      Serial.println("Attempting MQTT connection...");

      // Generate client name based on MAC address and last 8 bits of microsecond counter
      String clientName;
      
      clientName += "esp8266-";
      uint8_t mac[6];
      WiFi.macAddress(mac);
      clientName += macToStr(mac);
       Serial.println(clientName);

      //if connected, subscribe to the topic(s) we want to be notified about
      if (client.connect((char*) clientName.c_str(),"diego","24305314")){
        Serial.println("MTQQ Connected");
        client.subscribe(topic_sub.c_str());
    //    client.subscribe(lightTopic2);
      }

      //otherwise print failed for debugging
      else{Serial.println("Failed."); abort();}
    }
  }
}

//generate unique name from MAC addr
String macToStr(const uint8_t* mac){

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5){
      result += ':';
    }
  }

  return result;

}
