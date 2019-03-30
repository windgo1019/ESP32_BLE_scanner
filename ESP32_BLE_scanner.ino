/*
 * Regfer list
 * https://github.com/SensorsIot/Bluetooth-BLE-on-Arduino-IDE/blob/master/BLE_Proximity_Sensor/BLE_Proximity_Sensor.ino
 * https://github.com/knolleary/pubsubclient
 * https://github.com/prampec/IotWebConf
 * https://github.com/ropg/ezTime
 * 
 * I add some code to more easy using for mqtt sensor by windgo@gmail.com
 * if your esp32 want to clear the store value , you can earsh flash
 * esptool.exe --port COM4 erase_flash
 * 
 * if you want ro re-config value in web
 * the username is admin / the password you change already at 1st time config
 * https://bbs.hassbian.com/thread-6670-1-1.html
 * https://bbs.hassbian.com/thread-6472-1-1.html
 * HA is home-assistant https://www.home-assistant.io/
 * ==============================================================================================================================
 */
/*
==arduino編譯所需相關庫安裝==
1.參考這篇加入ESP32開發板相關庫
https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md
2.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋pubsubclient並安裝
3.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋iotWebConf並安裝
4.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋eztime並安裝

未來預計加入:
1.加入MQTT subscribe功能來操控ESP
2.語言選擇
==3.利用console控制（或GPIO）
  GPIO 1:TX
  GPIO 3:RX
==4.加入可一次偵測多個BLE裝置（目前一次掃描只能找到一個）  
==5.加入調整mqtt功能
=6.多餘程式碼轉成function
*/

//NTP2+IP
#include <ezTime.h>
Timezone myTZ;
char time2[0];
char ipchar[0];

//Add chinese word support in web網頁中文說明檔案
#include "ch.h"

//webrelay
boolean needReset = false;
int state = HIGH;
int notifysw = HIGH;
//#define CONFIG_PIN 21

//bt check
//#include "nvs.h"
//#include "nvs_flash.h"

//ota
#include <IotWebConf.h>
// -- Initial name of the ESP32. Used e.g. as wifi ssid of your ESP32.
const char thingName[] = "ESP32_BLE";
// -- Initial wifi password to connect to the your ESP32 when it becomes the AP mode. you must change the initial password or your ESP32 will keep AP mode after booting
const char wifiInitialApPassword[] = "configesp";
#define STRING_LEN 96
#define NUMBER_LEN 16
// -- Configuration specific key. The value should be modified if config structure was changed.
#define CONFIG_VERSION "0002"

// -- Callback method declarations.
void configSaved();
boolean formValidator();
DNSServer dnsServer;
WebServer server(80);
HTTPUpdateServer httpUpdater;

char mqtt_server[STRING_LEN];
char mqtt_user[STRING_LEN];
char mqtt_password[STRING_LEN];
//char mqtt_clientid[STRING_LEN];
//char TOPIC[STRING_LEN];
char PAYLOAD1[STRING_LEN];
char blemac1[STRING_LEN];
char PAYLOAD2[STRING_LEN];
char blemac2[STRING_LEN];
char PAYLOAD3[STRING_LEN];
char blemac3[STRING_LEN];
char PAYLOAD4[STRING_LEN];
char blemac4[STRING_LEN];
//char PAYLOAD3[STRING_LEN];
//char mqtt_port[NUMBER_LEN];
char ble_scantime[NUMBER_LEN];
char ble_rssi[NUMBER_LEN];
//char ESPipTOPIC[STRING_LEN];
//char ESPtestmodeTOPIC[STRING_LEN];
//char ESPmode[NUMBER_LEN];
char STOPmode[NUMBER_LEN];

IotWebConf iotWebConf(thingName, &dnsServer, &server, wifiInitialApPassword, CONFIG_VERSION);
//IotWebConfParameter stringParam = IotWebConfParameter("String param", "stringParam", stringParamValue, STRING_LEN);
IotWebConfSeparator separator1 = IotWebConfSeparator();
//IotWebConfParameter intParam = IotWebConfParameter("Int param", "intParam", intParamValue, NUMBER_LEN, "number", "1..100", NULL, "min='1' max='100' step='1'");
//IotWebConfParameter floatParam = IotWebConfParameter("Float param", "floatParam", floatParamValue, NUMBER_LEN, "number", "e.g. 23.4", NULL, "step='0.1'");
IotWebConfParameter mqtt_server_arg = IotWebConfParameter("MQTT_Server(ex: 192.168.31.184)", "mqtt_server", mqtt_server, STRING_LEN);
IotWebConfParameter mqtt_user_arg = IotWebConfParameter("MQTT_User(ex: mqtt, your mqtt server has no set user is ok )", "mqtt_user", mqtt_user, STRING_LEN);
IotWebConfParameter mqtt_password_arg = IotWebConfParameter("MQTT_Password(ex: mqtt, your mqtt server has no set password is ok )", "mqtt_password", mqtt_password, STRING_LEN);
//IotWebConfParameter mqtt_clientid_arg = IotWebConfParameter("MQTT_ClientId(ex: ESP32_BLE, different device must be a differnet clientid)", "mqtt_clientid", mqtt_clientid, STRING_LEN );
//IotWebConfParameter TOPIC_arg = IotWebConfParameter("MQTT publish TOPIC(ex: /ESP32_BLE/ble)", "MQTT publish TOPIC", TOPIC, STRING_LEN);
IotWebConfParameter blemac1_arg = IotWebConfParameter("BLE Device1 MAC(ex: c3:27:9f:d1:44:22)", "BLE Device1 MAC", blemac1, STRING_LEN);
IotWebConfParameter PAYLOAD1_arg = IotWebConfParameter("BLE Device1 Name in HA(ex: windgo)", "PAYLOAD1", PAYLOAD1, STRING_LEN);
IotWebConfParameter blemac2_arg = IotWebConfParameter("BLE Device2 MAC(ex: e5:ce:b4:9b:f5:bb)", "BLE Device2 MAC", blemac2, STRING_LEN);
IotWebConfParameter PAYLOAD2_arg = IotWebConfParameter("BLE Device2 Name in HA(ex: doris)", "PAYLOAD2", PAYLOAD2, STRING_LEN);
IotWebConfParameter blemac3_arg = IotWebConfParameter("BLE Device3 MAC(ex: 02:e1:c8:39:4e:fb)", "BLE Device3 MAC", blemac3, STRING_LEN);
IotWebConfParameter PAYLOAD3_arg = IotWebConfParameter("BLE Device3 Name in HA(ex: windgo2)", "PAYLOAD3", PAYLOAD3, STRING_LEN);
IotWebConfParameter blemac4_arg = IotWebConfParameter("BLE Device4 MAC(ex: 70:05:70:3f:d4:2d)", "BLE Device4 MAC", blemac4, STRING_LEN);
IotWebConfParameter PAYLOAD4_arg = IotWebConfParameter("BLE Device4 Name in HA(ex: doris2)", "PAYLOAD4", PAYLOAD4, STRING_LEN);

//IotWebConfParameter PAYLOAD3_arg = IotWebConfParameter("MQTT checking PAYLOAD3(ex : checking)", "MQTT checking PAYLOAD3", PAYLOAD3, STRING_LEN);
//IotWebConfParameter mqtt_port_arg = IotWebConfParameter("MQTT port(ex: 1883)", "MQTT port", mqtt_port, NUMBER_LEN, "number", "1 ~ 65535", NULL, "min='1' max='65535' step='1'");
IotWebConfParameter ble_scantime_arg = IotWebConfParameter("BLE_scantime(secs,ex: 4)", "BLE_scantime", ble_scantime, NUMBER_LEN, "number", "1 ~ 65535", NULL, "min='1' max='65535' step='1'");
IotWebConfParameter ble_rssi_arg = IotWebConfParameter("BLE_rssi(ex: -85)", "BLE_rssi", ble_rssi, NUMBER_LEN, "number", "-100 ~ -20", NULL, "min='-100' max='-20' step='1'");
//IotWebConfParameter ESPipTOPIC_arg = IotWebConfParameter("MQTT ESPip publish TOPIC(ex: /ESP32_BLE/ble/ip)", "MQTT ESPip publish TOPIC", ESPipTOPIC, STRING_LEN);
//IotWebConfParameter ESPtestmodeTOPIC_arg = IotWebConfParameter("MQTT ESPtestmode success rate publish TOPIC(ex: /ESP32_BLE/ble/testmode_success_rate)", "MQTT ESPtestmode success rate publish TOPIC", ESPtestmodeTOPIC, STRING_LEN);
//IotWebConfParameter ESPmode_arg = IotWebConfParameter("ESPmode(ex: 2)", "ESPmode", ESPmode, NUMBER_LEN, "number", "1:WIFI+BLE, 2:BLE", NULL, "min='1' max='2' step='1'");
IotWebConfParameter STOPmode_arg = IotWebConfParameter("STOPmode(ex: 0)", "STOPmode", STOPmode, NUMBER_LEN, "number", "0:find 1 device then stop search, 1:find 4 devices then stop search", NULL, "min='0' max='1' step='1'");

#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>
static BLEAddress *pServerAddress;
BLEScan* pBLEScan;
BLEClient*  pClient;
bool deviceFound = false;;
unsigned long entry;

// Update these with values suitable for your network.以下請改為你需要的數值
//定義LED的腳位，可參考https://goo.gl/6BQwSP
#define LED 22
//定義想要搜尋到的BLE裝置數量(非一般手機藍芽，藍芽4.0以上裝置才支援BLE功能，並且需打開藍芽廣播功能)
//String knownAddresses[] = { "c3:27:9f:d1:44:22", "e5:ce:b4:9b:f5:bb"}; // change for your ble device mac, not "bt" mac! you can use xiaomi sport app to see the miband ble mac
String knownAddresses[4]; // change for your ble device mac, not "bt" mac! you can use xiaomi sport app to see the miband ble mac

//int ble_scantime = 5;  // change for your ble scan time
//int ble_rssi = -85;  // change for your ble device rssi (signal strength) , you can say that the device was found.
//const char* wifissid = "your_wifi_ssid"; // change for your wifi ssid
//const char* wifipassword = "your_wifi_password"; // change for your wifi password
char wifissid;
char wifipassword;
//const char* mqtt_server = "192.168.1.105";  // change for your own MQTT broker address
//const char* mqtt_user = "mqtt";  // change if you have MQTT user, and remove mark "//    if (client.connect(mqtt_clientid.c_str(), mqtt_user, mqtt_password)) {"  and mark  "if (client.connect(mqtt_clientid.c_str())) {" 
//const char* mqtt_password = "mqtt";  // change if your have MQTT password
//String mqtt_clientid = "ESP32_BLE-";  // change for your ESP32 device mqtt client id
//int mqtt_port = 1883;  // change for your own MQTT server port
//const char* TOPIC = "/ESP32_BLE/ble";  // Change for your own topic
//const char* ESPipTOPIC = "/ESP32_BLE/ble/ip" ;
//const char* ESPtestmodeTOPIC = "/ESP32_BLE/ble/testmode_success_rate" ;
//char TOPIC;  // Change for your own topic
//char ESPipTOPIC;
//char ESPname3;
//char ESPname4;
//char* ESPtestmodeTOPIC;
const char* PAYLOAD0 = "checking";    // change for your search status when not find ble device
//const char* PAYLOAD1 = "windgo";    // change for your search status when your find the 1st ble device
//const char* PAYLOAD2 = "doris";    // change for your search status when your find the 2st ble device
//const char* bleman;    // bleman is the person was found
const char* bleman1;    // bleman is the person was found
const char* bleman2;    // bleman is the person was found
const char* bleman3;    // bleman is the person was found
const char* bleman4;    // bleman is the person was found
int runcount = 1; // count how many time to scan ble device
//int scantime = 1; //when scantime=100(no find ble device) , mqtt publish to server to know esp is alive
//int total_runcount; // count scan ble device times
//int total_findtime;
//String ipaddress; //send ESPip to MQTT
String time1; //ESP32 time
int findtime = 0; //count how many time to find ble device
String action; // for web buttom status check
int testmode = 2; // enter BLE testmod if true
int testfindtime = 0; //for BLE testmode

//int testcount = 1; //for BLE testmode
//int testfindtime = 0; //for BLE testmode
//String readfromSerial;
int deviceFoundNum;

/*
Add below in your configuration.yaml and make sure your mqtt server is alive
加入以下資訊到你的configuration.yaml，並且記得確認你的mqtt server服務已正常啟用

sensor:
  - platform: mqtt
    name: "miBLE"
    state_topic: "/ESP32_BLE/ble"
    value_template: '{{ value }}'
  - platform: mqtt
    name: "ESP32_BLE1_ip"
    state_topic: "/ESP32_BLE/ble/ip"
    value_template: '{{ value }}'
 */

WiFiClient espClient;
PubSubClient client(espClient);

//this function must before setup() for ble device scan
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
  Serial.print("Notify callback for characteristic ");
  Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
  Serial.print(" of data length ");
  Serial.println(length);
}

//this function must before setup() for ble device scan
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    /*
        Called for each advertising BLE server.
    */
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.print("BLE Advertised Device found: ");
      Serial.println(advertisedDevice.toString().c_str());
      pServerAddress = new BLEAddress(advertisedDevice.getAddress());

      bool known = false;
      for (int i = 0; i < (sizeof(knownAddresses) / sizeof(knownAddresses[0])); i++) {
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[i].c_str()) == 0) known = true;

//        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[0].c_str()) == 0) bleman = PAYLOAD1;
//        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[1].c_str()) == 0) bleman = PAYLOAD2;
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[0].c_str()) == 0) bleman1 = PAYLOAD1;
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[1].c_str()) == 0) bleman2 = PAYLOAD2;
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[2].c_str()) == 0) bleman3 = PAYLOAD3;
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[3].c_str()) == 0) bleman4 = PAYLOAD4;
      }
      
      if (known) {
        Serial.println("**********************************");
        Serial.print("BLE device RSSI: ");
        Serial.println(advertisedDevice.getRSSI());
        Serial.print("Device found: ");
//        if (advertisedDevice.getRSSI() > atoi(ble_rssi)) deviceFound = true;
//        else deviceFound = false;
        if (advertisedDevice.getRSSI() > atoi(ble_rssi)) { deviceFound = true; deviceFoundNum += 1;}
        else deviceFound = false;
        
        Serial.println(pServerAddress->toString().c_str());
        Serial.println("**********************************");
         if (deviceFoundNum == 4 || (deviceFoundNum > 0 && atoi(STOPmode) == 0 )) advertisedDevice.getScan()->stop();
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);     // Initialize the LED pin as an output
//  pinMode(CONFIG_PIN, INPUT);     // Initialize the LED pin as an output
  digitalWrite(LED, LOW);

//TOPIC
//String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
//mqtt_clientid += String(random(0xffff), HEX);
//char* TOPIC;
//char* ESPipTOPIC;
//char* ESPtestmodeTOPIC;
/*
Serial.println("1");
setTopic();
Serial.println(TOPIC);
Serial.println(ESPipTOPIC);
Serial.println(ESPtestmodeTOPIC);
*/
Serial.println("");
Serial.println("");

String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
char ESPname3[ESPname1.length()+1];
char ESPname4[ESPname2.length()+1];
ESPname1.toCharArray(ESPname3,ESPname1.length()+1);
ESPname2.toCharArray(ESPname4,ESPname2.length()+1);
char* TOPIC = ESPname3;
char* ESPipTOPIC = ESPname4;
//Serial.println(TOPIC);
//Serial.println(String(TOPIC).length());
//Serial.println(ESPipTOPIC);
//Serial.println(String(ESPipTOPIC).length());

  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  Serial.println("Created BLE scan client");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

//ota
//  iotWebConf.setConfigPin(CONFIG_PIN);
  iotWebConf.setupUpdateServer(&httpUpdater);
  iotWebConf.addParameter(&separator1);
  iotWebConf.addParameter(&mqtt_server_arg);
  iotWebConf.addParameter(&mqtt_user_arg);
  iotWebConf.addParameter(&mqtt_password_arg);
  //iotWebConf.addParameter(&mqtt_clientid_arg);
  //iotWebConf.addParameter(&TOPIC_arg);
  //iotWebConf.addParameter(&ESPipTOPIC_arg);  
  //iotWebConf.addParameter(&ESPtestmodeTOPIC_arg);  
  iotWebConf.addParameter(&blemac1_arg);
  iotWebConf.addParameter(&PAYLOAD1_arg);
  iotWebConf.addParameter(&blemac2_arg);
  iotWebConf.addParameter(&PAYLOAD2_arg);
  iotWebConf.addParameter(&blemac3_arg);
  iotWebConf.addParameter(&PAYLOAD3_arg);
  iotWebConf.addParameter(&blemac4_arg);
  iotWebConf.addParameter(&PAYLOAD4_arg);    
  //iotWebConf.addParameter(&PAYLOAD3_arg);
  //iotWebConf.addParameter(&mqtt_port_arg);
  iotWebConf.addParameter(&ble_scantime_arg);
  iotWebConf.addParameter(&ble_rssi_arg);
//  iotWebConf.addParameter(&ESPmode_arg);    
  iotWebConf.addParameter(&STOPmode_arg);    
  iotWebConf.setConfigSavedCallback(&configSaved);
  iotWebConf.setFormValidator(&formValidator);
  iotWebConf.getApTimeoutParameter()->visible = true;
  // -- Initializing the configuration.
  iotWebConf.init();
  // -- Set up required URL handlers on the web server.
  server.on("/", handleRoot);
  server.on("/config", []{ iotWebConf.handleConfig(); });
  server.onNotFound([](){ iotWebConf.handleNotFound(); });
  Serial.println("Web Ready.");
  Serial.println("");
  Serial.println("Please connect the ESP32_BLE AP to config it!"); 

// Serial.println(String(iotWebConf.getWifiSsid()).length());
// Serial.println(String(iotWebConf.getWifiPassword()).length());

  setupwifi_boot();


/*
 if (String(iotWebConf.getWifiSsid()).length() < 1 ){  
   //WiFi.softAP(thingName, wifiInitialApPassword);
   while (String(iotWebConf.getWifiSsid()).length() < 1 ){
     iotWebConf.doLoop();
     checkconsole();
  if (needReset || int(ESP.getFreeHeap()) < 45000 )
    {
    Serial.print("FreeMEM:");
    Serial.println(int(ESP.getFreeHeap()));
    Serial.print("needReset:");
    Serial.println(needReset);
    Serial.println("Rebooting after 1 second.");
    Serial.println("");
    delay(1000);
    ESP.restart();
    }
   }
 }
// }
 else {
      Serial.print("1st time to connect wifi");
      WiFi.mode(WIFI_STA);
      WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
      int i=0;
      while (WiFi.status() != WL_CONNECTED && testmode != 0) {
        delay(100);
        checkconsole();
        if (testmode !=3){
        Serial.print(".");
        }
        i++;
        //Try reconnect wifi every 10 secs
        if (i % 100 == 0 ){
           WiFi.mode(WIFI_STA);
           WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
        }
        if (i == 601 ){
           Serial.println("Connecting wifi fail, ESP restart");
           delay(100);
           ESP.restart();
        }
       }

       if (WiFi.status() == WL_CONNECTED && testmode != 0){
       Serial.println("connected");
       }
 }
*/
//make sure check console if no connect wifi 
 while (WiFi.status() != WL_CONNECTED && testmode != 0) {
  checkconsole();
  delay(100);
}
//  delay(100);
 if (WiFi.status() == WL_CONNECTED && testmode != 0) {
  Serial.println("");
  Serial.print("ESP32 get ip : ");
  Serial.println(WiFi.localIP());  
  blinkled();

//NTP2
  waitForSync();
  myTZ.setLocation(F("Asia/Taipei"));

  String ipaddress = WiFi.localIP().toString();
  char ipchar[ipaddress.length()+1];
  ipaddress.toCharArray(ipchar,ipaddress.length()+1);
  time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
  char time2[String(myTZ.dateTime("Y-m-d H:i:s")).length()+1];
  String(myTZ.dateTime("Y-m-d H:i:s")).toCharArray(time2,String(myTZ.dateTime("Y-m-d H:i:s")).length()+1);
  strcat( time2, " : " );
  strcat( time2, ipchar );
  Serial.println(time2);  
  //initial mqtt
//    mqtt_clientid += String(random(0xffff), HEX);
    client.setServer(mqtt_server, 1883);
  //  client.setServer(mqtt_server, mqtt_port);
//    client.setCallback(callback);
    //initial mqtt sensor value to HA
    //client.connect(mqtt_clientid.c_str(), mqtt_user, mqtt_password);
    //client.connect(mqtt_clientid, mqtt_user, mqtt_password);
    connectMQTT();
    client.publish(TOPIC, "ESP32_BLE alive", false);
    delay(100);
    client.publish(TOPIC, PAYLOAD0, false);
    delay(100); 
    client.publish(ESPipTOPIC, time2, false);
    delay(100); 
    client.disconnect();
//    Serial.println("ESP32_BLE alive");
    if (client.connected()){
    Serial.println("Send MQTT...done");     
    }
}

//    if (testmode != 0){ 
//    testmode = atoi(ESPmode);
//    }    
}

void loop() {
  
//reboot after configing 設定存檔或按鈕後重開ESP32
  if (int(ESP.getFreeHeap()) - 45000 < 0)
  {
    Serial.print("FreeMEM: ");
    Serial.println(int(ESP.getFreeHeap() - 45000));
    Serial.print("needReset: ");
    Serial.println(needReset);
    Serial.print("(needReset || (int(ESP.getFreeHeap()) - 45000) < 0): ");
    Serial.println((needReset || (int(ESP.getFreeHeap()) - 45000) < 0));
    Serial.println("Less MEM rebooting after 1 second.");
    Serial.println("");
    delay(1000);
    ESP.restart();
  }

  if (needReset)
  {
    Serial.print("FreeMEM: ");
    Serial.println(int(ESP.getFreeHeap() - 45000));
    Serial.print("needReset: ");
    Serial.println(needReset);
    Serial.print("(needReset || (int(ESP.getFreeHeap()) - 45000) < 0): ");
    Serial.println((needReset || (int(ESP.getFreeHeap()) - 45000) < 0));
    Serial.println("needReset rebooting after 1 second.");
    Serial.println("");
    delay(1000);
    ESP.restart();
  }

//set BLE devices mac list if need update
if (knownAddresses[0] !=blemac1 || knownAddresses[1] != blemac2 || knownAddresses[2] != blemac3 || knownAddresses[3] != blemac4 )
{
 knownAddresses[0] = blemac1;
 knownAddresses[1] = blemac2;
 knownAddresses[2] = blemac3;
 knownAddresses[3] = blemac4;
}  


String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
char ESPname3[ESPname1.length()+1];
char ESPname4[ESPname2.length()+1];
ESPname1.toCharArray(ESPname3,ESPname1.length()+1);
ESPname2.toCharArray(ESPname4,ESPname2.length()+1);
char* TOPIC = ESPname3;
char* ESPipTOPIC = ESPname4;
//Serial.println(TOPIC);
//Serial.println(ESPipTOPIC);

checkconsole();

//Enter testmode
if (testmode == 2 ){   
 Serial.println("");  
 Serial.println("Starting BLE testmode");  
 if (testmode != 0){
 Serial.println(""); 
 WiFi.mode(WIFI_OFF);
 Serial.println("**************Turn off WIFI**************"); 
 Serial.println("");
 }


  int testcount = 1; //for BLE testmode
 while (int(ESP.getFreeHeap()) > 45000 && testmode == 2 ) {

    if ((testcount % 200 ) == 0 && testmode != 0 ){
     setupwifi();     
     String ipaddress = WiFi.localIP().toString();
     char ipchar[ipaddress.length()+1];
     ipaddress.toCharArray(ipchar,ipaddress.length()+1);
     time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
     char time2[String(myTZ.dateTime("Y-m-d H:i:s")).length()+1];
     String(myTZ.dateTime("Y-m-d H:i:s")).toCharArray(time2,String(myTZ.dateTime("Y-m-d H:i:s")).length()+1);
     strcat( time2, " : " );
     strcat( time2, ipchar );
     connectMQTT();
     //Set the time+ip to mqtt topic "ESPipTOPIC"
     client.publish(ESPipTOPIC, time2, false);
     delay(100);
     client.publish(TOPIC, "ESP32_BLE alive", false);
     delay(100);
     client.publish(TOPIC, PAYLOAD0, false);         
     delay(100);
     client.disconnect();
     if (testmode != 0){
     Serial.println(""); 
     WiFi.mode(WIFI_OFF);
     Serial.println("**************Turn off WIFI**************");
     Serial.println(""); 
     }
//     Serial.println("ESP32_BLE alive");
     if (client.connected()){
     Serial.println("Send MQTT...done");     
     }     
    }
       
   Serial.println("");
   Serial.println("BLE mode");  
   Serial.println(myTZ.dateTime("Y-m-d H:i:s"));
   Serial.print("Free memory : "); 
   Serial.print(int(ESP.getFreeHeap()) - 45000);
   Serial.println(" ,reboot when Free memory < 0");
   Serial.print("BLE Device1 MAC : ");
   Serial.println(knownAddresses[0]); // should be same with blemac1
   Serial.print("BLE Device2 MAC : ");
   Serial.println(knownAddresses[1]); // should be same with blemac2 
   Serial.print("BLE Device3 MAC : ");
   Serial.println(knownAddresses[2]); // should be same with blemac3
   Serial.print("BLE Device4 MAC : ");
   Serial.println(knownAddresses[3]); // should be same with blemac4 

   deviceFound = false;
   deviceFoundNum = 0;
   bleman1 = "";
   bleman2 = "";
   bleman3 = "";
   bleman4 = "";
   BLEScanResults scanResults = pBLEScan->start(atoi(ble_scantime), false);
   pBLEScan->clearResults();
   checkdevice();

   Serial.print("testmode checktime : ");
   Serial.println(testcount);
   Serial.print("testmode findtime : ");
   Serial.println(testfindtime);
   Serial.print("BLE find success % : ");
   Serial.print((float(testfindtime) / float(testcount)) * 100 );
   Serial.println(" %");
   testcount++;
   checkconsole();
   //find delay time  
   if (deviceFoundNum > 0 && testmode != 0) {
        Serial.println("Waiting for 30 secs for next BLE search");
        int i = 0;
        while (i <= 300 && testmode != 0){  
        checkconsole();  
        delay(100);
        i ++;
        //Serial.println(i);
        }
   }
  }
  //setupwifi();
}

 //ota
 // setupwifi();
 iotWebConf.doLoop();
    
//webrelay,set state = action
action = server.arg("action");
if (action.equals("off"))    
  {
      state = LOW;
  }
  
//BLE scan switch notify 如果網路沒連線的話通知使用者到網頁設定
if (notifysw && WiFi.status() == WL_CONNECTED && state == LOW )
{   
    delay(1500);
    Serial.println("");
    Serial.println("");
    Serial.print("You can config in ");
    Serial.print("http://");
    Serial.println(WiFi.localIP());
    notifysw = LOW;
    delay(100);
}

/*
 //BLE Device Searching loop after wifi was connect and state ==HIGH 連線wifi並且state為HIGH才執行BLE Scan避免ESP回應卡住
 // The if function will run after finishing config in web
 if (WiFi.status() == WL_CONNECTED && state == HIGH) {
 Serial.println();

  String ipaddress = WiFi.localIP().toString();
  char ipchar[ipaddress.length()+1];
  ipaddress.toCharArray(ipchar,ipaddress.length()+1);
  //String time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
  char time2[String(myTZ.dateTime("Y-m-d H:i:s")).length()+1];
  String(myTZ.dateTime("Y-m-d H:i:s")).toCharArray(time2,String(myTZ.dateTime("Y-m-d H:i:s")).length()+1);
  strcat( time2, " : " );
  strcat( time2, ipchar );
 Serial.println("BLE+WIFI mode");  
 Serial.println(time2);
 Serial.print("Free memory : "); 
 Serial.print(int(ESP.getFreeHeap()) - 45000);
 Serial.println(" ,reboot when Free memory < 0");

 Serial.print("BLE Device1 MAC : ");
 Serial.println(knownAddresses[0]); // should be same with blemac1
 Serial.print("BLE Device2 MAC : ");
 Serial.println(knownAddresses[1]); // should be same with blemac2

 deviceFound = false; 
 // start BLE Scan
 BLEScanResults scanResults = pBLEScan->start(atoi(ble_scantime), false);
 pBLEScan->clearResults();
//handle incoming MQTT message
//  client.connect(mqtt_clientid, mqtt_user, mqtt_password);
  client.loop();
//find device mac in list 如果BLE Scan有找到指定裝置的MAC，且訊號強度大於設定值，進行MQTT publish步驟
  if (deviceFound) {
    digitalWrite(LED, HIGH);
    Serial.print("found ");
    Serial.print(bleman1);
    Serial.print(" ");
    Serial.println(bleman2);

//send message to MQTT server
  if (bleman1 == PAYLOAD1 && bleman2 == PAYLOAD2){
  connectMQTT();
  client.publish(TOPIC, PAYLOAD1, false);
  delay(100);
  client.publish(TOPIC, PAYLOAD0, false);
  delay(100);
  client.publish(TOPIC, PAYLOAD2, false);
  }
  else if (bleman1 == PAYLOAD1){
  connectMQTT();
  client.publish(TOPIC, PAYLOAD1, false);
  }
  else if (bleman2 == PAYLOAD2){
  connectMQTT();
  client.publish(TOPIC, PAYLOAD2, false);
  }
//      digitalWrite(LED, HIGH);
//      delay(500);
//      digitalWrite(LED, LOW);
      client.publish(TOPIC, PAYLOAD0, false);
      delay(200);
      client.disconnect();
      Serial.println("Will restart next BL Search");
      findtime++;
      delay(2000);
      digitalWrite(LED, LOW);
      Serial.println("Waiting for 120 secs");
      delay(120000);
   } 
   else {
      // The else function will run after finishing config in web
      Serial.println("not found!");
      }
      if ((runcount % 100) == 0 && testmode !=0 ){
          //Serial.println("ESP32_BLE alive");
          connectMQTT();
          //Set the time+ip to mqtt topic "ESPipTOPIC"
          client.publish(ESPipTOPIC, time2, false);
          delay(100);
          client.publish(TOPIC, "ESP32_BLE alive", false);
          delay(100);
          client.publish(TOPIC, PAYLOAD0, false);         
          delay(100); 
          client.disconnect();
          Serial.println("ESP32_BLE alive");
          Serial.println("Send MQTT...done");     
        }
      Serial.print("BLE scantime : ");
      Serial.println(runcount);
      Serial.print("BLE findtime : ");
      Serial.println(findtime);
      Serial.print("BLE find success % : ");
      Serial.print((float(findtime) / float(runcount)) * 100 );
      Serial.println(" %");
      Serial.print("BLE Scan time : ");
      Serial.print(atoi(ble_scantime));
      Serial.println(" sec");
      runcount++;
   }
*/     
}

/*
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  delay(5000);
}
*/

void connectMQTT() {
  if (testmode != 0 ){
  String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
  mqtt_clientid += String(random(0xffff), HEX);
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Connecting MQTT..........");
      if (client.connect(mqtt_clientid.c_str(), mqtt_user, mqtt_password)){
    //if (client.connect(mqtt_clientid, mqtt_user, mqtt_password)){
      Serial.println("connected");
    } else {
      Serial.print("connect mqtt server failed, mqtt state is : ");
      Serial.println(client.state());
      Serial.println("");
      Serial.println("");
      Serial.println("-4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time");
      Serial.println("-3 : MQTT_CONNECTION_LOST - the network connection was broken");
      Serial.println("-2 : MQTT_CONNECT_FAILED - the network connection failed");
      Serial.println("-1 : MQTT_DISCONNECTED - the client is disconnected cleanly");
      Serial.println("0 : MQTT_CONNECTED - the client is connected");
      Serial.println("1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT");
      Serial.println("2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier");
      Serial.println("3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection");
      Serial.println("4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected");
      Serial.println("5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect");
      Serial.println("");
      Serial.println("");
      }
    }
  }
}

//setupwifi
void setupwifi()
{
      if (WiFi.status() != WL_CONNECTED && testmode != 0) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
      Serial.println("");
      Serial.print("Connecting wifi");
      int i=0;
      while (WiFi.status() != WL_CONNECTED && testmode != 0) {
        delay(100);
        checkconsole();
        Serial.print(".");
        i++;
        //Reconnect wifi after 2 sec,reboot ESP if can not connect wifi after 4.5 sec
        if (i == 25 && testmode != 0 ){
         Serial.println("");
         Serial.println("Reconnecting wifi");
         WiFi.mode(WIFI_STA);
         WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
        }
        if (i == 45 && testmode != 0 ){
           Serial.println("Connecting wifi fail, ESP restart");
           delay(100);
           ESP.restart();
        }
       }
       if (WiFi.status() == WL_CONNECTED){
       Serial.println("connected");
       blinkled();
       }
      }
}
        
//ota
void handleRoot()
{
  // -- Let IotWebConf test and handle captive portal requests.
  if (iotWebConf.handleCaptivePortal())
  {
    // -- Captive portal request were already served.
    return;
  }

//webrelay,only set notifysw state in web
  if (server.hasArg("action"))
  {
    action = server.arg("action");
    if (action.equals("on"))
    {
      state = HIGH;
      notifysw = HIGH;
    }
    else if (action.equals("off"))
    {
      state = LOW;
      notifysw = HIGH;
      
    }
    else if (action.equals("reboot"))
    {
      needReset = true;
    }    
//    else if (action.equals("testmode"))
//    {
//      testmode = true;
//    }    
    
  }

//TOPIC
//String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
//mqtt_clientid += String(random(0xffff), HEX);
//char* TOPIC;
//char* ESPipTOPIC;
//char* ESPtestmodeTOPIC;
//Serial.println("3");
//Serial.println(TOPIC);
//Serial.println(ESPipTOPIC);
String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
char ESPname3[ESPname1.length()+1];
char ESPname4[ESPname2.length()+1];
ESPname1.toCharArray(ESPname3,ESPname1.length()+1);
ESPname2.toCharArray(ESPname4,ESPname2.length()+1);
char* TOPIC = ESPname3;
char* ESPipTOPIC = ESPname4;
//Serial.println(TOPIC);
//Serial.println(ESPipTOPIC);

/*
Serial.println("3");
setTopic();
Serial.println(TOPIC);
Serial.println(ESPipTOPIC);
Serial.println(ESPtestmodeTOPIC);
*/

//  int l1 = server.arg(mqtt_server_arg.getId()).length();
  float success1 = (float(findtime) / float(runcount)) * 100 ;  
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
//  s += FPSTR(IOTWEBCONF_HTTP_STYLE);
//  s += "<title>ESP32_BLE scanner with iotWebConf</title><meta http-equiv='refresh' content='120' ></head>";
  s += "<title>ESP32_BLE scanner</title></head>";
  s += "<body>";
  s += iotWebConf.getThingName();
  s += "    ";
  s += "<button type='button' onclick=\"location.href='?action=reboot';\" >Reboot</button>";
  s += "<br>";
  s += "version:20190329";
//  s += CONFIG_VERSION;
  s += "<br>";
  s += "<a href='https://bbs.hassbian.com/thread-6670-1-1.html'>Follow discuss now</a>";
  s += "<br>";
  s += "<a href='https://bbs.hassbian.com/thread-6472-1-1.html'>old version link</a>";
  s += "<br>";
  s += " FreeMemory : ";
  s += int(ESP.getFreeHeap()) - 45000;
  s += "<div>BLE Scan State : ";
  s += (state == HIGH ? "ON" : "OFF");
  s += "</div>";
/*
  s += "<div>BLE check times : ";
  s += runcount;
  s += "</div>";
  s += "</div>";
  s += "<div>BLE find times : ";
  s += findtime;
  s += "</div>";
  s += "<div>BLE find success % : ";
  s += success1;
  s += " %</div>";  
*/  
  s += "<div>upTime : ";
  s += time1;
  s += "<br>";
  s += "</div>";
  s += "<div>RunTime : ";
  s += float(millis() / (1000 * 60));
  s += " mins</div>";
  s += "<div>Clock : ";
  s += myTZ.dateTime("Y-m-d H:i:s");
//  s += "<br>auto Refresh in 120 secs<br>";
/*
  s += "<div>";
  s += "<button type='button' onclick=\"location.href='?action=on';\" >Turn ON</button>";
  s += " ";
  s += "<button type='button' onclick=\"location.href='?action=off';\" >Turn OFF</button>";
  s += " ";
  s += "<button type='button' onclick=\"location.href='?';\" >Refresh</button>";
  s += "</div>";
*/  
/*
  s += "<div>";
  s += "<br><button type='button' onclick=\"location.href='?action=reboot';\" >Reboot</button>";
//  s += " ";
//  s += "<button type='button' onclick=\"location.href='?action=testmode';\" >Enter BLE TestMode</button>";
  s += "</div>"; 
*/  
  s += "<br>";
//  s += "<div>";
//  s += "BLE TestMode will turn off wifi and force BLE scan only. After finish it will send find success % to MQTT server then reboot ESP32_BLE";
//  s += "<br>";
//  s += "Last BLE testmode success rate : ";
//  s += (float(testfindtime) / float(testcount)) * 100;
//  s += "</div>"; 
//  s += "<br>";
  s += "<div>";
//  if (String(iotWebConf.getWifiSsid()).length() > 1) {  
//  s += "<button type='button' onclick=\"location.href='config?action=off';\">Config</button> <button type='button' onclick=\"location.href='firmware?action=off';\" >Firmware</button>";
//  }
//  if (String(iotWebConf.getWifiSsid()).length() < 1) {
  s += "<button type='button' onclick=\"location.href='config';\">Config</button> <button type='button' onclick=\"location.href='firmware';\" >Firmware</button> <button type='button' onclick=\"location.href='?';\" >Refresh</button>";
//  }
  s += "</div>";
/*
  s += "<br>";
  s += "<b>The BLE Scan state will turn OFF when you enter config/firmware webpage, please turn on it again if you do not config anything</b><br>";
  s += "";
  s += ch[3];
*/
  s += "<br>";
//  s += "This is BLE device scanner webconfig page, go to  to upload ESP32 firmware.<br>";
//  s += ch[0];
  s += "";
//only show in initial config  
  if (String(iotWebConf.getWifiSsid()).length() < 1)
  {
  s += "<br><b>You shuould change the default ESP32_BLE AP password after flashing .bin</b> or your esp will stop connect wifi when you reboot.<br>";
  s += ch[1];
  }
  if (String(iotWebConf.getWifiSsid()).length() > 1)
  {
  s += "<br>If you want to config, the username / password are <b>admin / your ESP32_newpassword.</b><br>";
  }  
  s += "<br>If ESP32 FreeMemory < 0, ESP32 will reboot. Suggest 'Startup delay (seconds)' set to 0.<br>";
//  s += ch[2];
  s += "<br><br>";
  s += "====================ESP32 configurable values====================";
  s += "<ul>";
//  s += "<li>SearchMode:";
//  s += testmode;    
  s += "<li>MQTT_server: ";
  s += mqtt_server;
  s += "<li>MQTT_user: ";
  s += mqtt_user;
  s += "<li>MQTT_password: ";
  s += mqtt_password;
  //s += "<li>MQTT_clientid: ";
  //s += mqtt_clientid;
  s += "<li>MQTT publish TOPIC: ";
  s += TOPIC;
  s += "<li>MQTT ESPip publish TPOIC: ";
  s += ESPipTOPIC;
//  s += "<li>MQTT ESP testmode TPOIC: ";
//  s += ESPtestmodeTOPIC;
  s += "<li>MQTT checking PAYLOAD0: ";  
  s += PAYLOAD0;
  s += "<li>BLE Device1 MAC: ";
  s += knownAddresses[0];
//  s += blemac1;
  s += "<li>BLE Device1 PAYLOAD1: ";
  s += PAYLOAD1;
  s += "<li>BLE Device2 MAC: ";
  s += knownAddresses[1];
//  s += blemac2;
  s += "<li>BLE Device2 PAYLOAD2: ";
  s += PAYLOAD2;
  s += "<li>BLE Device3 MAC: ";
  s += knownAddresses[2];
  s += "<li>BLE Device3 PAYLOAD3: ";
  s += PAYLOAD3;
  s += "<li>BLE Device4 MAC: ";
  s += knownAddresses[3];
  s += "<li>BLE Device PAYLOAD4: ";
  s += PAYLOAD4;
  s += "<li>MQTT port: ";
  s += "1883";
//  s += atoi(mqtt_port);  
  s += "<li>BLE scantime: ";
  s += atoi(ble_scantime);
//  s += atoi(ble_scantime);
  s += "<li>BLE rssi: ";
  s += atoi(ble_rssi);
  s += ", find BLE device and rssi > ";
  s += atoi(ble_rssi);
  s += " then send MQTT";
  s+= "<li>STOPmode: ";
  s += atoi(STOPmode);

//  s += atoi(ble_rssi);  
  s += "</ul>";
//  s += "<br>";
//  s += "====================ESP32 configurable values====================";
  s += "===========================================================";
/*
  s += "<br><br>";
  s += "Add below in your configuration.yaml and make sure your mqtt server is alive<br>";
  s+= "sensor:<br>";
s += "  - platform: mqtt<br>";
s += "    name: 'ESP32_BLE'<br>";
s += "    state_topic: '<br>";
s += TOPIC;
s += "'<br>";
s += "    value_template: '{{ value }}'<br>";
s += "  - platform: mqtt<br>";
s += "    name: 'ESP32_BLE1_ip'<br>";
s += "    state_topic: '<br>";
s += ESPipTOPIC;
s += "'<br>";
s += "    value_template: '{{ value }}'<br>";
s += "  - platform: mqtt<br>";
s += "    name: 'ESP32_testmode_success'<br>";
s += "    state_topic: '<br>";
s += ESPtestmodeTOPIC;
s += "'<br>";
s += "    value_template: '{{ value }}'<br>";
s += "    unit_of_measurement: '%'<br>";
*/
//  s += "Go to <button type='button' onclick=\"location.href='config?action=off';\">Config</button> to change values. Or go to <button type='button' onclick=\"location.href='firmware?action=off';\" >Firmware</button> to upload ESP32 firmware.";
  s += "</body></html>\n";
  server.send(200, "text/html", s);
}

void configSaved()
{
  Serial.println("Configuration was updated.");
  needReset = true;
}

boolean formValidator()
{
  Serial.println("Validating Char form.");
  boolean valid = true;

  int l1 = server.arg(mqtt_server_arg.getId()).length();
  int l2 = server.arg(mqtt_user_arg.getId()).length();
  int l3 = server.arg(mqtt_password_arg.getId()).length();
//  int l4 = server.arg(TOPIC_arg.getId()).length();
  int l5 = server.arg(blemac1_arg.getId()).length();
  int l6 = server.arg(PAYLOAD1_arg.getId()).length();
  int l7 = server.arg(blemac2_arg.getId()).length();
  int l8 = server.arg(PAYLOAD2_arg.getId()).length();
//  int l9 = server.arg(PAYLOAD3_arg.getId()).length();
//  int l10 = server.arg(ESPipTOPIC_arg.getId()).length();
//  int l11 = server.arg(ESPtestmodeTOPIC_arg.getId()).length();
  int l12 = server.arg(ble_scantime_arg.getId()).length();
  int l13 = server.arg(ble_rssi_arg.getId()).length();
//  int l14 = server.arg(ESPmode_arg.getId()).length();
  int l15 = server.arg(STOPmode_arg.getId()).length();

  if (l1 < 1)
  {
    mqtt_server_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l2 < 1)
  {
    mqtt_user_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l3 < 1)
  {
    mqtt_password_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
/*  
  if (l4 < 1)
  {
    TOPIC_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
*/  
  if (l5 < 1)
  {
    blemac1_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l6 < 1)
  {
    PAYLOAD1_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l7 < 1)
  {
    blemac2_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l8 < 1)
  {
    PAYLOAD2_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
/*  
  if (l9 < 1)
  {
    PAYLOAD3_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }

  if (l10 < 1)
  {
    ESPipTOPIC_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  
  if (l11 < 1)
  {
    ESPtestmodeTOPIC_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  */
  if (l12 < 1)
  {
    ble_scantime_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }

  if (l13 < 1)
  {
    ble_rssi_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
/*
  if (l14 < 1)
  {
    ESPmode_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
*/
  if (l15 < 1)
  {
    STOPmode_arg.errorMessage = "Please key at least 1 characters for this config!";
    valid = false;
  }
  return valid;
}

void blinkled()
{
      digitalWrite(LED, HIGH);
      delay(150);
      digitalWrite(LED, LOW);
      delay(150);
      digitalWrite(LED, HIGH);
      delay(150);
      digitalWrite(LED, LOW);
      delay(150);
      digitalWrite(LED, HIGH);
      delay(150);
      digitalWrite(LED, LOW);
}
/*
void setTopic()
{
String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
String ESPname3 = "/" + String(iotWebConf.getThingName()) + "/ble/testmode_success_rate";
char ESPname4[ESPname1.length()+1];
char ESPname5[ESPname2.length()+1];
char ESPname6[ESPname3.length()+1];
ESPname1.toCharArray(ESPname4,ESPname1.length()+1);
ESPname2.toCharArray(ESPname5,ESPname2.length()+1);
ESPname3.toCharArray(ESPname6,ESPname3.length()+1);
TOPIC = ESPname4;
ESPipTOPIC = ESPname5;
ESPtestmodeTOPIC = ESPname6;
//Serial.println(TOPIC);
//Serial.println(ESPipTOPIC);
//Serial.println(ESPtestmodeTOPIC);
}
*/

void checkconsole()
{
String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
char ESPname3[ESPname1.length()+1];
char ESPname4[ESPname2.length()+1];
ESPname1.toCharArray(ESPname3,ESPname1.length()+1);
ESPname2.toCharArray(ESPname4,ESPname2.length()+1);
char* TOPIC = ESPname3;
char* ESPipTOPIC = ESPname4;

//console
char fromSerial[32];
String readfromSerial="";
int readserial=0;
int index = 0;
int show3 = 0;
    while (Serial.available() > 0) {
        fromSerial[index] = Serial.read();
        index++;
        readserial = 1;
    }
    int i=0;
    while(index > 0 ){
    readfromSerial += String(fromSerial[i]);
    //Serial.print(fromSerial[i]);
    //Serial.print(readfromSerial);
    index--;
    i++;
    }    
    
if(readfromSerial == "config"){

    testmode = 0;
    state = LOW;
    notifysw = HIGH;
    //setupwifi();
    Serial.println("");
    Serial.println("============================================");       
    Serial.print("Console get input : ");
    Serial.println(readfromSerial);
    Serial.println("Stop BLE search, waiting for config in web.");
    Serial.println("============================================");       
    Serial.println("");
    iotWebConf.doLoop();
    }
    
  else if(readfromSerial == "configwifi"){
    testmode = 0;
    state = LOW;
    notifysw = HIGH;
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("Enter Config mode");
    Serial.println("");
    Serial.println("key wifissid then press enter");
    Serial.println("");

String wifissid2;
String password2;
char fromSerial1[32];
char fromSerial2[32];
int readserial1 = 0;
int readserial2 = 0;
int index1 = 0;
int index2 = 0;
int show = 0;    
while (WiFi.status() != WL_CONNECTED) {
  if (readserial1 == 0){  
    while (Serial.available() > 0) {
        fromSerial1[index1] = Serial.read();
        index1++;
        readserial1 = 1;
        if (show == 0){
            Serial.println("key wifi password then press enter again");
            Serial.println("=========================================");
            Serial.println("");
            show = 1;
        }
    }
    int i=0;
    while(index1 > 0 ){
    wifissid2 += String(fromSerial1[i]);
    index1--;
    i++;
    }
 //  Serial.print("wifissid2:");  
 //  Serial.println(wifissid2);         
  }

  if (readserial1 == 1 && readserial2 == 0 ){  
    while (Serial.available() > 0) {
        fromSerial2[index2] = Serial.read();
        index2++;
        readserial2 = 1;
    }
    int i=0;
    while(index2 > 0 ){
    password2 += String(fromSerial2[i]);
    index2--;
    i++;
    }
    iotWebConf.doLoop();
//   Serial.print("password2:");  
//   Serial.println(password2);                  
  }
  
  if (readserial1 == 1 && readserial2 == 1){
  char wifissid2c[wifissid2.length()+1];
  wifissid2.toCharArray(wifissid2c,wifissid2.length()+1);
  char password2c[password2.length()+1];
  password2.toCharArray(password2c,password2.length()+1);
   Serial.print("wifissid2c:");  
   Serial.println(wifissid2c);         
   Serial.print("password2c:");  
   Serial.println(password2c);                  
  Serial.print("Connecting new wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid2c, password2c);
  while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
        }
  Serial.print("done");
  Serial.println("");
  Serial.print("ESP32 get ip : ");
  Serial.println(WiFi.localIP());
  Serial.println("====================="); 
  }
}

  }

/*
  else if(readfromSerial == "reset"){
    testmode = 0;
    state = LOW;
    notifysw = HIGH;
    Serial.println(readfromSerial);
    Serial.println("Reset config and enter wifi ap mode");
  }    
  else if(readfromSerial == "on"){
    testmode = 2;
    state = HIGH;
    //setupwifi();
  }
 */

/*  
  else if(readfromSerial == "configmqtt"){
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("=========================================");
    Serial.println("Enter Config mode");
    Serial.println("");
    Serial.println("key new mqtt server then press enter");
    Serial.println("");

String mqtt_server2;
String mqtt_user2;
String mqtt_password2;

char fromSerial1[32];
char fromSerial2[32];
char fromSerial3[32];

int readserial1 = 0;
int readserial2 = 0;
int readserial3 = 0;

int index1 = 0;
int index2 = 0;
int index3 = 0;

int show = 0;
int show2 = 0;

while (!client.connected()) {
  if (readserial1 == 0){  
    while (Serial.available() > 0) {
        fromSerial1[index1] = Serial.read();
        index1++;
        readserial1 = 1;
        if (show == 0){
            Serial.println("key new mqtt user then press enter again");
            Serial.println("");
            show = 1;
        }
    }
    int i=0;
    while(index1 > 0 ){
    mqtt_server2 += String(fromSerial1[i]);
    index1--;
    i++;
    }
 //  Serial.print("wifissid2:");  
 //  Serial.println(wifissid2);         
  }

  if (readserial1 == 1 && readserial2 == 0 && readserial3 == 0 ){  
    while (Serial.available() > 0) {
        fromSerial2[index2] = Serial.read();
        index2++;
        readserial2 = 1;
            if (show2 == 0){
            Serial.println("key new mqtt password then press enter again");
            Serial.println("=========================================");
            Serial.println("");
            show2 = 1;
        }
    }
    int i=0;
    while(index2 > 0 ){
    mqtt_user2 += String(fromSerial2[i]);
    index2--;
    i++;
    }
    //iotWebConf.doLoop();           
  }

    if (readserial1 == 1 && readserial2 == 1 && readserial3 == 0 ){  
    while (Serial.available() > 0) {
        fromSerial3[index3] = Serial.read();
        index3++;
        readserial3 = 1;
    }
    int i=0;
    while(index3 > 0 ){
    mqtt_password2 += String(fromSerial3[i]);
    index3--;
    i++;
    }        
  }

  
  if (readserial1 == 1 && readserial2 == 1 && readserial3 == 1 ){

   Serial.print("new mqtt server:");  
   Serial.println(mqtt_server2); 
   Serial.print("new mqtt user:");  
   Serial.println(mqtt_user2); 
   Serial.print("new mqtt password:");  
   Serial.println(mqtt_password2); 
   Serial.println(""); 
   Serial.print("old mqtt server:");  
   Serial.println(mqtt_server); 
   Serial.print("old mqtt user:");  
   Serial.println(mqtt_user); 
   Serial.print("old mqtt password:");  
   Serial.println(mqtt_password); 

   mqtt_server2.toCharArray(mqtt_server, 96);
   mqtt_user2.toCharArray(mqtt_user, 96);
   mqtt_password2.toCharArray(mqtt_password, 96);
   Serial.println(""); 
   Serial.print("get new mqtt server:");  
   Serial.println(mqtt_server); 
   Serial.print("get new mqtt user:");  
   Serial.println(mqtt_user); 
   Serial.print("get new mqtt password:");  
   Serial.println(mqtt_password);
   setupwifi();
   client.setServer(mqtt_server, 1883); 
   connectMQTT();
   client.publish(TOPIC, "ESP32_BLE alive", false);
   delay(100);
   client.publish(TOPIC, PAYLOAD0, false);
   delay(100); 
   client.publish(ESPipTOPIC, time2, false);
   Serial.println("Test MQTT...done");   
  /*  
  char wifissid2c[wifissid2.length()+1];
  wifissid2.toCharArray(wifissid2c,wifissid2.length()+1);
  char password2c[password2.length()+1];
  password2.toCharArray(password2c,password2.length()+1);
   Serial.print("wifissid2c:");  
   Serial.println(wifissid2c);         
   Serial.print("password2c:");  
   Serial.println(password2c);                  
  Serial.print("Connecting new wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifissid2c, password2c);
  while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
        }
  Serial.print("done");
  Serial.println("");
  Serial.print("ESP32 get ip : ");
  Serial.println(WiFi.localIP());
  Serial.println("====================="); 
  }
}


    
     Serial.print("Send mqtt packet for checking mqtt server....");
     setupwifi();
     String ipaddress = WiFi.localIP().toString();
     char ipchar[ipaddress.length()+1];
     ipaddress.toCharArray(ipchar,ipaddress.length()+1);
     time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
     char time2[String(myTZ.dateTime("Y-m-d H:i:s")).length()+1];
     String(myTZ.dateTime("Y-m-d H:i:s")).toCharArray(time2,String(myTZ.dateTime("Y-m-d H:i:s")).length()+1);
     strcat( time2, " : " );
     strcat( time2, ipchar );
     connectMQTT();
     client.publish(ESPipTOPIC, time2, false);
     delay(100);
     client.publish(TOPIC, "ESP32_BLE alive", false);
     delay(100);
     client.publish(TOPIC, PAYLOAD3, false);         
     delay(100);
     client.disconnect();
     Serial.println("done");
     Serial.println("");
     
    }  
   }
  client.disconnect(); 
  testmode = 0;
  state = LOW;
  notifysw = HIGH;
  }
*/  
  else if(readfromSerial == "reboot" || readfromSerial == "reload" ){
  Serial.println("");
  Serial.println("============================");
  Serial.println("Reboot ESP32....");
  Serial.println("============================");
  Serial.println("");
  delay(100);
  ESP.restart();  
  }
  else if(readfromSerial != ""){
    if (show3 == 0){
    Serial.println("");
    Serial.println(""); 
    Serial.println("=======================================================");
    Serial.println("");       
    Serial.println("Please copy & paste all of your command to ESP32");
    Serial.println("Command list:");
    Serial.println("");           
    Serial.println("config");
    Serial.println("configwifi");
//    Serial.println("configmqtt");
    Serial.println("reload or reboot");
    Serial.println("");
    Serial.println("=======================================================");      
    Serial.println("");
    Serial.println("");
    testmode = 3;
    }
  }      
  else if(readfromSerial != ""){
    Serial.println("");
    Serial.println("============================================");       
    Serial.print("Console get input : ");
    Serial.println(readfromSerial);
    Serial.println("============================================");       
    Serial.println("");
  }  
}



void checkdevice()
{
      if (deviceFoundNum > 0) {
      String ESPname1 = "/" + String(iotWebConf.getThingName()) + "/ble";
      String ESPname2 = "/" + String(iotWebConf.getThingName()) + "/ble/ip";
      char ESPname3[ESPname1.length()+1];
      char ESPname4[ESPname2.length()+1];
      ESPname1.toCharArray(ESPname3,ESPname1.length()+1);
      ESPname2.toCharArray(ESPname4,ESPname2.length()+1);
      char* TOPIC = ESPname3;
      char* ESPipTOPIC = ESPname4;  
      
      //digitalWrite(LED, HIGH);
      Serial.println("");
      Serial.print("Found ");
      Serial.print(deviceFoundNum);
      Serial.println(" BLE device:");
      if (bleman1 != ""){
      Serial.println(bleman1);
      }
      if (bleman2 != ""){
      Serial.println(bleman2);
      }
      if (bleman3 != ""){
      Serial.println(bleman3);
      }
      if (bleman4 != ""){
      Serial.println(bleman4);
      }
      testfindtime++;
      setupwifi();
      connectMQTT();

        //send message to MQTT server
        if (bleman1 == PAYLOAD1){
        client.publish(TOPIC, PAYLOAD1, false);
        delay(100);
        client.publish(TOPIC, PAYLOAD0, false);
        delay(200);
        }
        if (bleman2 == PAYLOAD2){
        client.publish(TOPIC, PAYLOAD2, false);
        delay(100);
        client.publish(TOPIC, PAYLOAD0, false);
        delay(200);
        }
        if (bleman3 == PAYLOAD3){
        client.publish(TOPIC, PAYLOAD3, false);
        delay(100);
        client.publish(TOPIC, PAYLOAD0, false);
        delay(200);
        }
        if (bleman4 == PAYLOAD4){
        client.publish(TOPIC, PAYLOAD4, false);
        delay(100);
        client.publish(TOPIC, PAYLOAD0, false);
        delay(200);
        }        
        client.disconnect();

        if (testmode != 0){
        Serial.println("");   
        WiFi.mode(WIFI_OFF);
        Serial.println("**************Turn off WIFI**************"); 
        Serial.println(""); 
        }

        //delay(2000);
        //digitalWrite(LED, LOW);
     } 
     else {
          Serial.println("No found BLE device");  
     } 
   
}

void setupwifi_boot()
{
   if (String(iotWebConf.getWifiSsid()).length() < 1 ){  
   //WiFi.softAP(thingName, wifiInitialApPassword);
   while (String(iotWebConf.getWifiSsid()).length() < 1 ){
     iotWebConf.doLoop();
     checkconsole();
  if (needReset || int(ESP.getFreeHeap()) < 45000 )
    {
    Serial.print("FreeMEM:");
    Serial.println(int(ESP.getFreeHeap()));
    Serial.print("needReset:");
    Serial.println(needReset);
    Serial.println("Rebooting after 1 second.");
    Serial.println("");
    delay(1000);
    ESP.restart();
    }
   }
 }
// }
 else {
      Serial.print("1st time to connect wifi");
      WiFi.mode(WIFI_STA);
      WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
      int i=0;
      while (WiFi.status() != WL_CONNECTED && testmode != 0) {
        delay(100);
        checkconsole();
        if (testmode !=3){
        Serial.print(".");
        }
        i++;
        //Try reconnect wifi every 10 secs
        if (i % 100 == 0 ){
           WiFi.mode(WIFI_STA);
           WiFi.begin(iotWebConf.getWifiSsid(), iotWebConf.getWifiPassword());
        }
        if (i == 601 ){
           Serial.println("Connecting wifi fail, ESP restart");
           delay(100);
           ESP.restart();
        }
       }

       if (WiFi.status() == WL_CONNECTED && testmode != 0){
       Serial.println("connected");
       }
 }  
}

