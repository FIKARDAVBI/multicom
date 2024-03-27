#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define topology_T  //T untuk Tree,
//#define topology_S //S untuk Star,
//#define topology_D //D untuk tree lain
//#define P2P
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint16_t this_node = 00;
unsigned char datatosend = 'M';
uint16_t payloadSize;

#ifdef topology_T  // konfigurasi address topology tree
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 021;
const uint16_t node05 = 012;
const uint16_t node06 = 022;
#elif defined(topology_S)  // konfigurasi address topology star
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 03;
const uint16_t node04 = 04;
const uint16_t node05 = 05;
const uint16_t node06 = 015;
#elif defined(topology_D)                      // konfigurasi address topology tree lainnya
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 012;
const uint16_t node05 = 0111;
const uint16_t node06 = 0112;
#else
const uint16_t node01 = 01;
#endif
unsigned long starttime;
unsigned long timer;
int counttx;
int packetreceive;
float transmittime;
float totaltxtime;
float packetloss;
float peaktxt;
float lowtxt;
float avgtxtime;

RF24 radio(10, 9);           // nRF24L01 (CE,CSN)
RF24Network network(radio);  // Include the radio in the networ

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Master");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Testing....");
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MAX,1);
  radio.setDataRate(RF24_2MBPS);
  network.begin(90, this_node);  //(channel, node address)
  timer = millis();
}

void loop() {
  network.update();
  if(millis()-timer>3000){
    timer = millis();
#ifdef topology_T
    RF24NetworkHeader header(node06);
#elif defined(topology_S)
    RF24NetworkHeader header(node06);
#elif defined(topology_D)
    RF24NetworkHeader header(node06);
#else
    RF24NetworkHeader header(node01);
#endif
    if(counttx++<20){
      starttime = micros();
      network.write(header,&datatosend,sizeof(datatosend));
    }
  }
  if (network.available()){
    RF24NetworkHeader header1;
    payloadSize = network.peek(header1);
    transmittime = ((float)(micros() - starttime) / 2)/1000;
    totaltxtime += transmittime;
    network.read(header1, 0, 0);
    packetreceive++;
    if(transmittime > peaktxt) peaktxt = transmittime;
    if(transmittime < lowtxt || lowtxt == 0.0) lowtxt = transmittime;
    
  }
  if(counttx>21){
    packetloss = (packetreceive/20)*100;
    avgtxtime = totaltxtime/packetreceive;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("PT :" + (String)peaktxt + "ms");
    lcd.setCursor(7, 0);
    lcd.print("LT :" + (String)lowtxt + "ms");
    lcd.setCursor(0, 1);
    lcd.print("PL :" + (String)packetloss);
    lcd.setCursor(7, 1);
    lcd.print("TA :" + (String)avgtxtime + "ms");
    
  }

}
