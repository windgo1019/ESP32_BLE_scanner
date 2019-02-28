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
 * 
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
1.加入subscribe功能來操控ESP

*/
//save value
//#include <EEPROM.h>

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
#define CONFIG_VERSION "20190228v1"

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
//char PAYLOAD3[STRING_LEN];
//char mqtt_port[NUMBER_LEN];
char ble_scantime[NUMBER_LEN];
char ble_rssi[NUMBER_LEN];
//char ESPipTOPIC[STRING_LEN];
//char ESPtestmodeTOPIC[STRING_LEN];

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
//IotWebConfParameter PAYLOAD3_arg = IotWebConfParameter("MQTT checking PAYLOAD3(ex : checking)", "MQTT checking PAYLOAD3", PAYLOAD3, STRING_LEN);
//IotWebConfParameter mqtt_port_arg = IotWebConfParameter("MQTT port(ex: 1883)", "MQTT port", mqtt_port, NUMBER_LEN, "number", "1 ~ 65535", NULL, "min='1' max='65535' step='1'");
IotWebConfParameter ble_scantime_arg = IotWebConfParameter("BLE_scantime(secs,ex: 4)", "BLE_scantime", ble_scantime, NUMBER_LEN, "number", "1 ~ 65535", NULL, "min='1' max='65535' step='1'");
IotWebConfParameter ble_rssi_arg = IotWebConfParameter("BLE_rssi(ex: -85)", "BLE_rssi", ble_rssi, NUMBER_LEN, "number", "-100 ~ -20", NULL, "min='-100' max='-20' step='1'");
//IotWebConfParameter ESPipTOPIC_arg = IotWebConfParameter("MQTT ESPip publish TOPIC(ex: /ESP32_BLE/ble/ip)", "MQTT ESPip publish TOPIC", ESPipTOPIC, STRING_LEN);
//IotWebConfParameter ESPtestmodeTOPIC_arg = IotWebConfParameter("MQTT ESPtestmode success rate publish TOPIC(ex: /ESP32_BLE/ble/testmode_success_rate)", "MQTT ESPtestmode success rate publish TOPIC", ESPtestmodeTOPIC, STRING_LEN);

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
String knownAddresses[2]; // change for your ble device mac, not "bt" mac! you can use xiaomi sport app to see the miband ble mac

//int ble_scantime = 5;  // change for your ble scan time
//int ble_rssi = -85;  // change for your ble device rssi (signal strength) , you can say that the device was found.
//const char* wifissid = "your_wifi_ssid"; // change for your wifi ssid
//const char* wifipassword = "your_wifi_password"; // change for your wifi password
//const char* mqtt_server = "192.168.1.105";  // change for your own MQTT broker address
//const char* mqtt_user = "mqtt";  // change if you have MQTT user, and remove mark "//    if (client.connect(mqtt_clientid.c_str(), mqtt_user, mqtt_password)) {"  and mark  "if (client.connect(mqtt_clientid.c_str())) {" 
//const char* mqtt_password = "mqtt";  // change if your have MQTT password
//String mqtt_clientid = "ESP32_BLE-";  // change for your ESP32 device mqtt client id
//int mqtt_port = 1883;  // change for your own MQTT server port
//const char* TOPIC = "/ESP32_BLE/ble";  // Change for your own topic
//const char* ESPipTOPIC = "/ESP32_BLE/ble/ip" ;
//const char* ESPtestmodeTOPIC = "/ESP32_BLE/ble/testmode_success_rate" ;
char* TOPIC;  // Change for your own topic
char* ESPipTOPIC;
char* ESPtestmodeTOPIC;
const char* PAYLOAD3 = "checking";    // change for your search status when not find ble device
//const char* PAYLOAD1 = "windgo";    // change for your search status when your find the 1st ble device
//const char* PAYLOAD2 = "doris";    // change for your search status when your find the 2st ble device
const char* bleman;    // bleman is the person was found
int runcount = 1; // count how many time to scan ble device
//int scantime = 1; //when scantime=100(no find ble device) , mqtt publish to server to know esp is alive
//int total_runcount; // count scan ble device times
//int total_findtime;
//String ipaddress; //send ESPip to MQTT
String time1; //ESP32 time
int findtime = 0; //count how many time to find ble device
String action; // for web buttom status check
boolean testmode = false; // enter BLE testmod if true
//int testcount = 1; //for BLE testmode
//int testfindtime = 0; //for BLE testmode

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
  - platform: mqtt
    name: "ESP32_testmode_findtime_percent"
    state_topic: "/ESP32_BLE/ble/testmode_success_rate"
    value_template: '{{ value }} '
    unit_of_measurement: "%"
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
//        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[0].c_str()) == 0) bleman = "windgo";
//        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[1].c_str()) == 0) bleman = "doris";
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[0].c_str()) == 0) bleman = PAYLOAD1;
        if (strcmp(pServerAddress->toString().c_str(), knownAddresses[1].c_str()) == 0) bleman = PAYLOAD2;

      }
      
      if (known) {
        Serial.print("BLE device RSSI: ");
        Serial.println(advertisedDevice.getRSSI());
        Serial.print("Device found: ");
        if (advertisedDevice.getRSSI() > atoi(ble_rssi)) deviceFound = true;
        else deviceFound = false;
        
        Serial.println(pServerAddress->toString().c_str());
        advertisedDevice.getScan()->stop();
      }
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(LED, OUTPUT);     // Initialize the LED pin as an output
  digitalWrite(LED, LOW);

//TOPIC
//String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
//mqtt_clientid += String(random(0xffff), HEX);
//char* TOPIC;
//char* ESPipTOPIC;
//char* ESPtestmodeTOPIC;
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


  BLEDevice::init("");
  pClient  = BLEDevice::createClient();
  Serial.println("Created BLE scan client");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);

//ota

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
  //iotWebConf.addParameter(&PAYLOAD3_arg);
  //iotWebConf.addParameter(&mqtt_port_arg);
  iotWebConf.addParameter(&ble_scantime_arg);
  iotWebConf.addParameter(&ble_rssi_arg);
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
  
//save value
//EEPROM.begin(200);
 


  while (WiFi.status() != WL_CONNECTED) {
  iotWebConf.doLoop();
  delay(100);
  }
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
    client.publish(TOPIC, PAYLOAD3, false);
    delay(100);    
    client.publish(ESPipTOPIC, time2, false);
    client.disconnect();

    Serial.println(); 
    Serial.println(); 
    Serial.println("Please connect the ESP32_BLE AP to config it!"); 
    
}

void loop() {
  
//reboot after configing 設定存檔或按鈕後重開ESP32
  if (needReset || int(ESP.getFreeHeap()) < 43500 )
  {
    Serial.println("Rebooting after 1 second.");
    Serial.println("");
    iotWebConf.delay(1000);
    ESP.restart();
  }


//set BLE devices mac list if need update
if (knownAddresses[0] !=blemac1 || knownAddresses[1] != blemac2 )
{
 knownAddresses[0] = blemac1;
 knownAddresses[1] = blemac2;
}  

/*
char* TOPIC;
char* ESPipTOPIC;
char* ESPtestmodeTOPIC;
*/
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


//Enter testmode
if (testmode){
 Serial.println("");  
 Serial.println("");  
 Serial.println("Starting BLE testmode");  
 WiFi.mode(WIFI_OFF);
int testcount = 1; //for BLE testmode
int testfindtime = 0; //for BLE testmode
 while (int(ESP.getFreeHeap()) > 43500) {
   Serial.println("");  
   Serial.println(myTZ.dateTime("Y-m-d H:i:s"));
   Serial.print("Free memory : "); 
   Serial.print(int(ESP.getFreeHeap()) - 43500);
   Serial.println(" ,reboot when Free memory < 0");
   Serial.print("BLE Device1 MAC : ");
   Serial.println(knownAddresses[0]); // should be same with blemac1
   Serial.print("BLE Device2 MAC : ");
   Serial.println(knownAddresses[1]); // should be same with blemac2 
//   Serial.print("Finish test after ");
//   Serial.print(((1000 - testcount) * atoi(ble_scantime)));
//   Serial.print(" secs(=");
//   Serial.print(((1000 - testcount) * atoi(ble_scantime)) / 60);
//   Serial.println(" mins)");
   deviceFound = false;
   BLEScanResults scanResults = pBLEScan->start(atoi(ble_scantime), false);
   pBLEScan->clearResults();
    if (deviceFound) {
        testfindtime++;
     } 
     else {
          Serial.println("not found");  
     }

   Serial.print("testmode checktime : ");
   Serial.println(testcount);
   Serial.print("testmode findtime : ");
   Serial.println(testfindtime);
   Serial.print("BLE find success % : ");
   Serial.print((float(testfindtime) / float(testcount)) * 100 );
   Serial.println(" %"); 
   testcount++;
   }
   
   while (WiFi.status() != WL_CONNECTED) {
      iotWebConf.doLoop();
      delay(100);
     }   
   connectMQTT();
   String testfindtimes = String((float(testfindtime) / float(testcount)) * 100);
   char testfindtimes2[testfindtimes.length()+1];
   testfindtimes.toCharArray(testfindtimes2,testfindtimes.length()+1);
   client.publish(ESPtestmodeTOPIC, testfindtimes2, false);
   delay(100);
   client.disconnect();
   testmode = false;
   Serial.println("Finishing BLE testmode");
   delay(100);
   ESP.restart();
}

 //ota
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
    Serial.println("");
    Serial.println("");
    Serial.print("You need turn on BLE scan in ");
    Serial.print("http://");
    Serial.println(WiFi.localIP());
    notifysw = LOW;
    delay(100);
}
   
 //BLE Device Searching loop after wifi was connect and state ==HIGH 連線wifi並且state為HIGH才執行BLE Scan避免ESP回應卡住
 // The if function will run after finishing config in web
 if (WiFi.status() == WL_CONNECTED && state == HIGH) {
 Serial.println();

 //save value
//total_runcount = EEPROM.read(191);
//total_findtime = EEPROM.read(192);
// Serial.println(total_runcount);
// Serial.println(total_findtime);
  String ipaddress = WiFi.localIP().toString();
  char ipchar[ipaddress.length()+1];
  ipaddress.toCharArray(ipchar,ipaddress.length()+1);
  //String time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
  char time2[String(myTZ.dateTime("Y-m-d H:i:s")).length()+1];
  String(myTZ.dateTime("Y-m-d H:i:s")).toCharArray(time2,String(myTZ.dateTime("Y-m-d H:i:s")).length()+1);
  strcat( time2, " : " );
  strcat( time2, ipchar );
 
 Serial.println(time2);
 Serial.print("Free memory : "); 
 Serial.print(int(ESP.getFreeHeap()) - 43500);
 Serial.println(" ,reboot when Free memory < 0");
 
/*
 Serial.println(EEPROM.read(193));
 Serial.println(EEPROM.read(194));
 Serial.println(EEPROM.read(195));
 Serial.println(EEPROM.read(196));
 Serial.println(EEPROM.read(197));
 Serial.println(EEPROM.read(198));
 Serial.println(EEPROM.read(199));
 Serial.println(EEPROM.read(200));
*/ 

 Serial.print("BLE Device1 MAC : ");
 Serial.println(knownAddresses[0]); // should be same with blemac1
 Serial.print("BLE Device2 MAC : ");
 Serial.println(knownAddresses[1]); // should be same with blemac2

 /*
Serial.print("mqtt_status: '");
Serial.print(client.state());
Serial.println("' , 0 is 'mqtt server connected'");
-4 : MQTT_CONNECTION_TIMEOUT - the server didn't respond within the keepalive time
-3 : MQTT_CONNECTION_LOST - the network connection was broken
-2 : MQTT_CONNECT_FAILED - the network connection failed
-1 : MQTT_DISCONNECTED - the client is disconnected cleanly
0 : MQTT_CONNECTED - the client is connected
1 : MQTT_CONNECT_BAD_PROTOCOL - the server doesn't support the requested version of MQTT
2 : MQTT_CONNECT_BAD_CLIENT_ID - the server rejected the client identifier
3 : MQTT_CONNECT_UNAVAILABLE - the server was unable to accept the connection
4 : MQTT_CONNECT_BAD_CREDENTIALS - the username/password were rejected
5 : MQTT_CONNECT_UNAUTHORIZED - the client was not authorized to connect
  */
 deviceFound = false; 
 // start BLE Scan
 BLEScanResults scanResults = pBLEScan->start(atoi(ble_scantime), false);
 pBLEScan->clearResults();
//handle incoming MQTT message
//  client.connect(mqtt_clientid, mqtt_user, mqtt_password);
  client.loop();
//find device mac in list 如果BLE Scan有找到指定裝置的MAC，且訊號強度大於設定值，進行MQTT publish步驟
  if (deviceFound) {
    Serial.print("found ");
    Serial.println(bleman);
//send message to MQTT server
  if (bleman == PAYLOAD1){
  connectMQTT();
  client.publish(TOPIC, PAYLOAD1, false);
  }
  if (bleman == PAYLOAD2){
  connectMQTT();
  client.publish(TOPIC, PAYLOAD2, false);
  }
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
      client.publish(TOPIC, PAYLOAD3, false);
      delay(200);
      client.disconnect();
      Serial.println("Will restart next BL Search");
      findtime++;
   } 
   else {
      // The else function will run after finishing config in web
      Serial.println("not found!");
      }
      if ((runcount % 100) == 0){
          Serial.println("ESP32_BLE alive");
          connectMQTT();
          //Set the time+ip to mqtt topic "ESPipTOPIC"
          client.publish(ESPipTOPIC, time2, false);
          delay(100);
          client.publish(TOPIC, "ESP32_BLE alive", false);
          delay(100);
          client.publish(TOPIC, PAYLOAD3, false);         
          delay(100);  
          client.disconnect();
          //needReset = true;
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

/*   
//save value   
   if ( runcount == 240 && (total_runcount < runcount || total_findtime < findtime) ){
      EEPROM.write(191, runcount);
      EEPROM.write(192, findtime);
      EEPROM.commit();
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
  String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
  mqtt_clientid += String(random(0xffff), HEX);
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
      if (client.connect(mqtt_clientid.c_str(), mqtt_user, mqtt_password)){
    //if (client.connect(mqtt_clientid, mqtt_user, mqtt_password)){
      Serial.println("connected");
    } else {
      Serial.print("connect mqtt server failed, mqtt state is : ");
      Serial.println(client.state());
/*
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
      delay(50);    
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
      delay(50);
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
*/      
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
    else if (action.equals("testmode"))
    {
      testmode = true;
    }    
    
  }
/*  
//NTP2
  String ipaddress; //send ESPip to MQTT
  String time1; //ESP32 time
  ipaddress = WiFi.localIP().toString();
  char ipchar[ipaddress.length()+1];
  ipaddress.toCharArray(ipchar,ipaddress.length()+1);
  time1 = String(myTZ.dateTime("Y-m-d H:i:s"));
  char time2[time1.length()+1];
  time1.toCharArray(time2,time1.length()+1);
  strcat( time2, " : " );
  strcat( time2, ipchar );
*/    
//TOPIC
//String mqtt_clientid  =  String(iotWebConf.getThingName()) + "-";
//mqtt_clientid += String(random(0xffff), HEX);
//char* TOPIC;
//char* ESPipTOPIC;
//char* ESPtestmodeTOPIC;
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

  float success1 = (float(findtime) / float(runcount)) * 100 ;  
  String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
//  s += FPSTR(IOTWEBCONF_HTTP_STYLE);
//  s += "<title>ESP32_BLE scanner with iotWebConf</title><meta http-equiv='refresh' content='120' ></head>";
  s += "<title>ESP32_BLE scanner with iotWebConf</title></head>";
  s += "<body>";
  s += iotWebConf.getThingName();
  s += "<br>";
  s += "version:";
  s += CONFIG_VERSION;
  s += "<br>";
  s += "<a href='https://bbs.hassbian.com/thread-6472-1-1.html'>Follow discuss now</a>";
  s += "<br>";
  s += " FreeMemory : ";
  s += int(ESP.getFreeHeap()) - 43500;
  s += "<div>BLE Scan State : ";
  s += (state == HIGH ? "ON" : "OFF");
  s += "</div>";
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
  s += "<div>upTime : ";
  s += time1;
  s += "<br>";
  s += "</div>";
  s += "<div>Clock : ";
  s += myTZ.dateTime("Y-m-d H:i:s");
//  s += "<br>auto Refresh in 120 secs<br>";
  s += "<div>";
  s += "<button type='button' onclick=\"location.href='?action=on';\" >Turn ON</button>";
  s += " ";
  s += "<button type='button' onclick=\"location.href='?action=off';\" >Turn OFF</button>";
  s += " ";
  s += "<button type='button' onclick=\"location.href='?';\" >Refresh</button>";
  s += "</div>";
  s += "<div>";
  s += "<br><button type='button' onclick=\"location.href='?action=reboot';\" >Reboot</button>";
//  s += " ";
//  s += "<button type='button' onclick=\"location.href='?action=testmode';\" >Enter BLE TestMode</button>";
  s += "</div>"; 
  s += "<br>";
//  s += "<div>";
//  s += "BLE TestMode will turn off wifi and force BLE scan only. After finish it will send find success % to MQTT server then reboot ESP32_BLE";
//  s += "<br>";
//  s += "Last BLE testmode success rate : ";
//  s += (float(testfindtime) / float(testcount)) * 100;
//  s += "</div>"; 
//  s += "<br>";
  s += "<div>";
  s += "<button type='button' onclick=\"location.href='config?action=off';\">Config</button> <button type='button' onclick=\"location.href='firmware?action=off';\" >Firmware</button>";
  s += "</div>";
  s += "<br>";
  s += "<b>The BLE Scan state will turn OFF when you enter config/firmware webpage, please turn on it again if you do not config anything</b><br>";
  s += "";
  s += ch[3];
  s += "<br>";
//  s += "This is BLE device scanner webconfig page, go to  to upload ESP32 firmware.<br>";
//  s += ch[0];
  s += "";
//only show in initial config  
  int l1 = server.arg(mqtt_server_arg.getId()).length();
  if (l1 > 1)
  {
  s += "<br><b>You shuould change the default ESP32_BLE AP password after flashing .bin</b> or your esp will stop connect wifi when you reboot.<br>";
  s += ch[1];
  }
//  s += "<br>You can use 'esptool.exe --port COM4 erase_flash' to clean ESP32 value.<br>";
//  s += ch[2];
  s += "<br>";
  s += "====================ESP32 configurable values====================";
  s += "<ul>";
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
  s += "<li>MQTT checking PAYLOAD3: ";  
  s += PAYLOAD3;
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
  s+= " then send MQTT";
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


  return valid;
}

void blinkled()
{
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
      delay(50);    
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
      delay(50);
      digitalWrite(LED, HIGH);
      delay(50);
      digitalWrite(LED, LOW);
}
