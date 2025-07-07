

// To program...
// Click House (Platform ID Build at the bottom)
// Click Arror Right down the bottom.


// Configuration sentences. NOTE: No sentences until received over serial port
// GUID/settings/GUID = Set GUID from serial. 
// GUID/settings/wifi = 1 or 0
// GUID/settings/wifi/SSID = 
// GUID/settings/wifi/password = 
// GUID/settings/wifi/DHCP = 1 or 0
// GUID/settings/wifi/IP = 192.168.0.123
// GUID/settings/wifi/GW = 192.168.0.123
// GUID/settings/wifi/mask = 255.255.255.0
// GUID/settings/ether = 1 or 0
// GUID/settings/ether/DHCP = 1 or 0
// GUID/settings/ether/IP = 192.168.0.123
// GUID/settings/ether/GW = 192.168.0.123
// GUID/settings/ether/mask = 255.255.255.0
// GUID/settings/mqtt/server = 192.168.0.73
// GUID/settings/mqtt/port = 1833
// GUID/settings/mqtt/SSL = 1 or 0
// GUID/settings/mqtt/username = 
// GUID/settings/mqtt/password =
// GUID/settings/mqtt/maximumtime = 30 - send every 30 seconds even if no change
// GUID/settings/mqtt/

// settings to turn on fans even if no control?




// void WiFiEvent(WiFiEvent_t event)
// {
//   switch (event) {
//     case ARDUINO_EVENT_ETH_START:
//       Serial.println("ETH Started");
//       //set eth hostname here
//       ETH.setHostname("esp32-ethernet");
//       break;
//     case ARDUINO_EVENT_ETH_CONNECTED:
//       Serial.println("ETH Connected");
//       break;
//     case ARDUINO_EVENT_ETH_GOT_IP:
//       Serial.print("ETH MAC: ");
//       Serial.print(ETH.macAddress());
//       Serial.print(", IPv4: ");
//       Serial.print(ETH.localIP());
//       if (ETH.fullDuplex()) {
//         Serial.print(", FULL_DUPLEX");
//       }
//       Serial.print(", ");
//       Serial.print(ETH.linkSpeed());
//       Serial.println("Mbps");
//       eth_connected = true;
//       break;
//     case ARDUINO_EVENT_ETH_DISCONNECTED:
//       Serial.println("ETH Disconnected");
//       eth_connected = false;
//       break;
//     case ARDUINO_EVENT_ETH_STOP:
//       Serial.println("ETH Stopped");
//       eth_connected = false;
//       break;
//     case SYSTEM_EVENT_ETH_START:
//       Serial.println("SYS ETH Started");
//       //set eth hostname here
//       ETH.setHostname("esp32-ethernet");
//       break;
//     case SYSTEM_EVENT_ETH_CONNECTED:
//       Serial.println("SYS ETH Connected");
//       break;
//     case SYSTEM_EVENT_ETH_GOT_IP:
//       Serial.print("SYS ETH MAC: ");
//       Serial.print(ETH.macAddress());
//       Serial.print(", IPv4: ");
//       Serial.print(ETH.localIP());
//       if (ETH.fullDuplex()) {
//         Serial.print(", FULL_DUPLEX");
//       }
//       Serial.print(", ");
//       Serial.print(ETH.linkSpeed());
//       Serial.println("Mbps");
//       eth_connected = true;
//       break;
//     case SYSTEM_EVENT_ETH_DISCONNECTED:
//       Serial.println("SYS ETH Disconnected");
//       eth_connected = false;
//       break;
//     case SYSTEM_EVENT_ETH_STOP:
//       Serial.println("SYS ETH Stopped");
//       eth_connected = false;
//       break;
//     default:
//       break;
//   }
// }




// // Callback function
// void callback(char* topic, byte* payload, unsigned int length) {
//   // In order to republish this payload, a copy must be made
//   // as the orignal payload buffer will be overwritten whilst
//   // constructing the PUBLISH packet.

//   // Allocate the correct amount of memory for the payload copy
//   byte* p = (byte*)malloc(length);
//   // Copy the payload to the new buffer
//   memcpy(p,payload,length);
//   client.publish("outTopic", p, length);
//   // Free the memory
//   free(p);
// }

// https://github.com/espressif/arduino-esp32/issues/1262


#include <ETH.h>
#include <PubSubClient.h> // go to Sketch --> Include Library --> Manage Libraries... --> search for PubSubClient by Nick O'Leary
#include <string.h>

void callback(char *topic_, byte *payload, unsigned int length);

#define BUTTON  34

struct settingsStruct {
  boolean wifi_enable = false;
  boolean wifi_dhcp = true;
  IPAddress wifi_ip_address;
  IPAddress wifi_ip_gateway;
  IPAddress wifi_ip_mask;
  IPAddress wifi_ip_DNS_1;
  IPAddress wifi_ip_DNS_2;
  char wifi_ssid[64];
  char wifi_password[64];


  boolean ether_enable = true;
  boolean ether_dhcp = true;
  IPAddress ether_ip_address;
  IPAddress ether_ip_gateway;
  IPAddress ether_ip_mask;
  IPAddress ether_ip_DNS_1;
  IPAddress ether_ip_DNS_2;

  u16_t mqtt_port = 1833;
  boolean mqtt_ssl = false;
  u16_t mqtt_maximum_time = 30;

  char mqtt_username [64];
  char mqtt_password [64];
  char mqtt_server [128];
};

struct settingsStruct mqttSettings;

// MQTT Broker
const char *mqtt_broker = "2.0.1.1";
const char *topic = "emqx/ESP32-POE_Button";
const char *mqtt_username = "";
const char *mqtt_password = "";
const int mqtt_port = 1883;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
    // Set software serial baud to 115200;
    Serial.begin(115200);
    ETH.begin();

    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) {
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
    // Publish and subscribe
    client.publish(topic, "Hi, I'm Olimex board ESP32-POE\n\rThis is button example\n\rType \"SB\" to scan button level");
    client.subscribe(topic);

    pinMode (BUTTON, INPUT_PULLUP);
}

void callback(char *topic_, byte *payload, unsigned int length)
{
  char OutputMessage[32], SB=0;
  payload[length]=0;

  if (payload > 0){

    if (strcmp(topic,"/settings/wifi/enable")==0){
      if (payload[0] == '1'){
        mqttSettings.wifi_enable = true;
      }
      if (payload[0] == '0'){
        mqttSettings.wifi_enable = false;
      }
    }
    if (strcmp(topic,"/settings/wifi/SSID")==0){
      strncpy (mqttSettings.wifi_ssid, (char *)payload, 64);
    }
    if (strcmp(topic,"/settings/wifi/password")==0){
      strncpy (mqttSettings.wifi_password, (char *)payload, 64);
    }
    if (strcmp(topic,"/settings/wifi/dhcp")==0){
      if (payload[0] == '1'){
        mqttSettings.wifi_dhcp = true;
      }
      if (payload[0] == '0'){
        mqttSettings.wifi_dhcp = false;
      }
    }
    if (strcmp(topic,"/settings/wifi/IP")==0){
      mqttSettings.wifi_ip_address.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/GW")==0){
      mqttSettings.wifi_ip_gateway.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/mask")==0){
      mqttSettings.wifi_ip_mask.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/DNS_1")==0){
      mqttSettings.wifi_ip_DNS_1.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/DNS_2")==0){
      mqttSettings.wifi_ip_DNS_2.fromString((char*) payload);
    }



    if (strcmp(topic,"/settings/ether/enable")==0){
      if (payload[0] == '1'){
        mqttSettings.ether_enable = true;
      }
      if (payload[0] == '0'){
        mqttSettings.ether_enable = false;
      }
    }
    if (strcmp(topic,"/settings/ether/dhcp")==0){
      if (payload[0] == '1'){
        mqttSettings.ether_dhcp = true;
      }
      if (payload[0] == '0'){
        mqttSettings.ether_dhcp = false;
      }
    }
    if (strcmp(topic,"/settings/ether/IP")==0){
      mqttSettings.ether_ip_address.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/ether/GW")==0){
      mqttSettings.ether_ip_gateway.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/ether/mask")==0){
      mqttSettings.ether_ip_mask.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/DNS_1")==0){
      mqttSettings.ether_ip_DNS_1.fromString((char*) payload);
    }
    if (strcmp(topic,"/settings/wifi/DNS_2")==0){
      mqttSettings.ether_ip_DNS_2.fromString((char*) payload);
    }


    if (strcmp(topic,"/settings/mqtt/server")==0){
      strncpy (mqttSettings.mqtt_server, (char *)payload, 64);
    }
    if (strcmp(topic,"/settings/mqtt/port")==0){
      u16_t working = 0;
      sscanf ((char *)payload, "%d", &working);
      mqttSettings.mqtt_port = working; 

    }
    if (strcmp(topic,"/settings/mqtt/SSL")==0){
      if (payload[0] == '1'){
        mqttSettings.mqtt_ssl = true;
      }
      if (payload[0] == '0'){
        mqttSettings.mqtt_ssl = false;
      }
    }
    if (strcmp(topic,"/settings/mqtt/username")==0){
      strncpy (mqttSettings.mqtt_username, (char *)payload, 64);
    }
    if (strcmp(topic,"/settings/mqtt/password")==0){
      strncpy (mqttSettings.mqtt_password, (char *)payload, 64);
    }

    if (strcmp(topic,"/settings/mqtt/maximumtime")==0){
      u16_t working = 0;
      sscanf ((char *)payload, "%d", &working);
      mqttSettings.mqtt_maximum_time = working; 
    }
  }







  // SB  = strstr ((char*)payload, "SB")!=NULL;

  // if (SB)
  // {
  //   sprintf (OutputMessage, "Button is %d\n\r", digitalRead(BUTTON));
  //   client.publish((const char*)topic, (const char*)OutputMessage, strlen(OutputMessage));
  // }
}

void loop() {
    client.loop();
}