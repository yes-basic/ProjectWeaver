#include <Arduino.h>
#include <serialCommand.h>

#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> 

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
bool senddata(int symbol,int dataHitStage);
String serialcommand(bool flush);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
Adafruit_MPU6050 mpu;
//init constant motion track variables
  const int gLow=-6;
  const int gHigh=18;
  const int timePerHit=1000;
//init misc var
  long firstHitTime=0;
  long prevHitTime=0;
  int hitStage=0;
  bool debugtoggleblocker=0;
  bool debug=0;
  bool result=0;

//init peerinfo
  //no screen mac address: 58:BF:25:9E:D5:14
  //t-disp with pin:D4:D4:DA:5D:F6:C8
  //heltec mac EC:62:60:B3:B7:BC            EC:62:60:B3:B7:BC 
  uint8_t broadcastAddress[] = {0xD4, 0xD4, 0xDa, 0x5D, 0xF6, 0xC8};
  esp_now_peer_info_t peerInfo;


//init modular pins
  // ttgo 39 38 32 33
  int pin1=39;
  int pin2=32;
  int pin3=38;
  int pin4=33;
// init send Structure
  typedef struct RPSsend {
    int RPS;
    int hitStage;
  } RPSsend;
  RPSsend send;
//init pin percent and  past percent struct
  typedef struct mainStruct{
    int pin1;
    int pin2;
    int pin3;
  }mainStruct;
  mainStruct data,dataTFT;
//init millis last
  int millisLastSend;
  int millisLastTFT;
//init string stuff
char charArray[50];

serialCommand inCom;


void setup() {
  // Init Serial Monitor, analog, and tft
    Serial.begin(115200);
    analogReadResolution(12);
    Serial.println("RPS weaver V0.11");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  //init TFT  
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
    img.createSprite(240, 135);
    img.fillSprite(TFT_BLACK);
  //init mpu6050
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    if (!mpu.begin()) {
      Serial.println("Failed to find MPU6050 chip");
      tft.println("MPU init failed");
      while (1) {
       delay(10);
      }
    }
    Serial.println("MPU6050 Found!");
  // Manage device Wi-Fi
    WiFi.mode(WIFI_STA);
    Serial.print("MAC:");
    Serial.println(WiFi.macAddress());
  // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
      Serial.println("Error initializing ESP-NOW");
      return;
    }

  // Register callback and peer
      esp_now_register_send_cb(OnDataSent);
  
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
  
    // Add peer        
      if (esp_now_add_peer(&peerInfo) != ESP_OK){
        Serial.println("Failed to add peer");
        return;
      }

}
 
void loop() {
  //check sensors
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
  
  //debug
    if(!digitalRead(0)){
      if(!debugtoggleblocker){debug=!debug;debugtoggleblocker=1;}
    }else{
      debugtoggleblocker=0;
    }
    if(debug){
      Serial.print(" hitStage:");
      Serial.print(hitStage);
      Serial.print(" mod:");
      Serial.print(hitStage%2);
      Serial.print(" X:");
      Serial.print(a.acceleration.x);
      Serial.print(" Y:");
      Serial.print(a.acceleration.y);
      Serial.print(" Z:");
      Serial.print(a.acceleration.z);
      Serial.println(" High:20 Low:-20");
      Serial.println();
      
    }
  //temp debug
      if(inCom.check()){       
        inCom.commandString.toCharArray(charArray,50);
        Serial.println(charArray);
        Serial.println(inCom.commandString);
        Serial.println(inCom.commandString=="clear");
        
        if(inCom.commandString=="clear"){
          Serial.println("accepted");
        }else{
          Serial.println("denied");
        }
        
        inCom.flush();
        delay(5000);
        }

  //determine motion  
    if(hitStage<5){
      if(a.acceleration.y<gLow&&hitStage%2==0){
        hitStage++;
        if(hitStage==1){firstHitTime=millis();}
        prevHitTime=millis();
      }
      if(a.acceleration.y>gHigh&&hitStage%2==1){
        hitStage++;
        prevHitTime=millis();
      }
      if(prevHitTime+timePerHit<millis()&&hitStage<6){
        hitStage=0;
      }
    senddata(0,hitStage);
    }else if(prevHitTime+timePerHit<millis()){
      if (analogRead(pin1)<3100){
        if(analogRead(pin2)<3200){
          if(analogRead(pin3)<3100){
            if(senddata(2,hitStage)){
              hitStage=0;
            }
          }else{
            if(senddata(3,hitStage)){
              hitStage=0;
            }
          }
        }
      }else if(analogRead(pin2)>3200&&analogRead(pin3)>3100){
        if(senddata(1,hitStage)){
          hitStage=0;
        }
      }
      
      
      Serial.println("done");
    }    
  
  //tft refresh
    
        img.fillSprite(TFT_BLACK);
        img.setTextSize(2);
       //send success 
         if(result)
        {img.setTextColor(TFT_GREEN);} else 
        {img.setTextColor(TFT_RED);}
        /*
        img.drawString("pin 1:",0,0,2);img.drawNumber(data.pin1,75,0,2);img.drawString("%",100,0,2);
        img.drawString("pin 2:",0,34,2);img.drawNumber(data.pin2,75,34,2);img.drawString("%",100,34,2);
        img.drawString("pin 3:",0,68,2);img.drawNumber(data.pin3,75,68,2);img.drawString("%",100,68,2);
        img.drawString("pin 4:",0,102,2);img.drawNumber(data.pin4,75,102,2);img.drawString("%",100,102,2);
        */
        img.setCursor(0, 0, 2);
        img.println(analogRead(pin1)); 
        img.println(analogRead(pin2));
        img.println(analogRead(pin3));
        img.setCursor(75,0,2); img.println("m/s^2  angle");
        img.setCursor(75,34.2);  img.print("x");img.println(a.acceleration.x);
        img.setCursor(75,68,2);  img.print("y");img.println(a.acceleration.y);
        img.setCursor(75,102,2);  img.print("z");img.println(a.acceleration.z);
        
        img.pushSprite(0, 0);
          

    



}
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

}
bool senddata(int symbol, int dataHitStage){
  //Send message via ESP-NOW
    if(millis()-millisLastSend>25){
      millisLastSend=millis();
      // Set values to send
      send.RPS=symbol;
      send.hitStage=dataHitStage;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &send, sizeof(send));
    
      //check send result
        if (result == ESP_OK) {
          //Serial.println("Sent with success");
          result=true;
          return true;
         }
         else {
           //Serial.println("Error sending the data");
           result=false;
           return false;
         }
      }  
  return false; 
}
