#include <Arduino.h>


#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h> 

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite img = TFT_eSprite(&tft);
Adafruit_MPU6050 mpu;
//init peerinfo
  //no screen mac address: 58:BF:25:9E:D5:14
  //heltec mac EC:62:60:B3:B7:BC            EC:62:60:B3:B7:BC 
  uint8_t broadcastAddress[] = {0xEC, 0x62, 0x60, 0xB3, 0xB7, 0xBC};
  esp_now_peer_info_t peerInfo;


//init modular pins
  // ttgo 39 38 32 33
  int pin1=39;
  int pin2=38;
  int pin3=32;
  int pin4=33;
// init esp-now Structure
  typedef struct MARINOW {
    char command[32];
    int val1;
    int val2;
    int val3;
    int val4;
  } MARINOW;
  MARINOW send1;
//init pin percent and past percent struct
  typedef struct mainStruct{
    int pin1;
    int pin2;
    int pin3;
    int pin4;
    bool sendStat;
  }mainStruct;
  mainStruct data,dataTFT;
//init millis last
  int millisLastSend;
  int millisLastTFT;


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  data.sendStat=status;
}

void setup() {
  // Init Serial Monitor, analog, and tft
    Serial.begin(115200);
    analogReadResolution(12);
    Serial.println("weaver V0.1");
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("MPU6050 Found!");
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(TFT_BLACK);
  //init tft sprite
    img.createSprite(240, 135);
    img.fillSprite(TFT_BLACK);
  // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

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
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  //create percent values 
    data.pin4 =round(map(analogRead(pin4),0,4095,0,100));
    data.pin3 =round(map(analogRead(pin3),0,4095,0,100));
    data.pin2 =round(map(analogRead(pin2),0,4095,0,100));
    data.pin1 =round(map(analogRead(pin1),0,4095,0,100));
  
  
  //Send message via ESP-NOW
    if(millis()-millisLastSend>25){
      millisLastSend=millis();
      // Set values to send
        send1.val1 = data.pin1;
        send1.val2 = data.pin2;
        send1.val3 = data.pin3;
        send1.val4 = data.pin4;
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &send1, sizeof(send1));
    
      //check send result
        if (result == ESP_OK) {
          Serial.println("Sent with success");
         }
         else {
           Serial.println("Error sending the data");
         }
    }     

  //tft refresh
    
        img.fillSprite(TFT_BLACK);
        img.setTextSize(2);
       //send success 
         if(data.sendStat == ESP_NOW_SEND_SUCCESS)
        {img.setTextColor(TFT_GREEN);} else 
        {img.setTextColor(TFT_RED);}
        /*
        img.drawString("pin 1:",0,0,2);img.drawNumber(data.pin1,75,0,2);img.drawString("%",100,0,2);
        img.drawString("pin 2:",0,34,2);img.drawNumber(data.pin2,75,34,2);img.drawString("%",100,34,2);
        img.drawString("pin 3:",0,68,2);img.drawNumber(data.pin3,75,68,2);img.drawString("%",100,68,2);
        img.drawString("pin 4:",0,102,2);img.drawNumber(data.pin4,75,102,2);img.drawString("%",100,102,2);
        */
        img.setCursor(0, 0, 2);
        img.print(data.pin1);img.println("%"); 
        img.print(data.pin2);img.println("%");
        img.print(data.pin3);img.println("%");
        img.print(data.pin4);img.println("%");
        img.setCursor(75,0,2); img.println("m/s^2  angle");
        img.setCursor(75,34.2);  img.print("x");img.println(a.acceleration.x);
        img.setCursor(75,68,2);  img.print("y");img.println(a.acceleration.y);
        img.setCursor(75,102,2);  img.print("z");img.println(a.acceleration.z);
        
        img.pushSprite(0, 0);
        //update tft variables
          dataTFT.pin1=data.pin1;
          dataTFT.pin2=data.pin2;
          dataTFT.pin3=data.pin3;
          dataTFT.pin4=data.pin4;
          dataTFT.sendStat=data.sendStat;

    



}
