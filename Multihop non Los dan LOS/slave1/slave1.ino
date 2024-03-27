#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define topology_T  //T untuk Tree,
//#define topology_S //S untuk Star,
//#define topology_D //D untuk tree lain
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

const uint16_t master = 00;
unsigned char datatosend = 'M';
uint16_t payloadSize;

#ifdef topology_T  // konfigurasi address topology tree
const uint16_t thisnode = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 021;
const uint16_t node05 = 012;
const uint16_t node06 = 022;
#elif defined(topology_S)  // konfigurasi address topology star
const uint16_t this_node = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 03;
const uint16_t node04 = 04;
const uint16_t node05 = 05;
const uint16_t node06 = 015;
#else                 // konfigurasi address topology tree lainnya
const uint16_t this_node = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 012;
const uint16_t node05 = 0111;
const uint16_t node06 = 0112;
#endif

RF24 radio(10, 9);           // nRF24L01 (CE,CSN)
RF24Network network(radio);  // Include the radio in the networ

void setup() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("slave6 (Receiver)");
  delay(1000);
  lcd.clear();
  SPI.begin();
  radio.begin();
  radio.setPALevel(RF24_PA_MAX,1);
  radio.setDataRate(RF24_2MBPS);
  network.begin(90, this_node);  //(channel, node address)
}

void loop() {
  network.update();
}
