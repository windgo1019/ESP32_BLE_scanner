# ESP32_BLE_scanner
  
Refer list:  
 https://github.com/SensorsIot/Bluetooth-BLE-on-Arduino-IDE/blob/master/BLE_Proximity_Sensor/BLE_Proximity_Sensor.ino  
 https://github.com/knolleary/pubsubclient  
 https://github.com/prampec/IotWebConf  
 https://github.com/ropg/ezTime  
   
 I add some code to more easy using for mqtt sensor by windgo@gmail.com  
   
 [follow discuss link](https://bbs.hassbian.com/thread-6472-1-1.html)  
 HA is [home-assistant](https://www.home-assistant.io/)    
  
==Arduino編譯所需相關庫安裝==  
1.參考這篇加入ESP32開發板相關庫  
https://github.com/espressif/arduino-esp32/blob/master/docs/arduino-ide/boards_manager.md  
2.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋pubsubclient並安裝  
3.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋iotWebConf並安裝  
4.在Arduino工具列內的Sketch-->Include library-->Manage Libraries-->搜尋eztime並安裝  
  
ESP32_BLE.rar include:  
AutoErase.exe : clear flash ESP32 tool  
FlashESP32.bat : auto flash ESP32 tool  
setup.mp4 : ESP32_BLE scanner setup video  
  
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/web1.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/web2.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/find.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/notfind.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/miband_1.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/miband_2.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/miband_3.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/oe_ibeacon.png)   
![image](https://raw.githubusercontent.com/windgo1019/ESP32_BLE_scanner/master/miband_1.png)   
      
