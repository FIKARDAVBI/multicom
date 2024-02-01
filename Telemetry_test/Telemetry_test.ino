/*
* Arduino Wireless Communication Tutorial
*     Example 1 - Transmitter Code
*                
* by Dejan Nedelkovski, www.HowToMechatronics.com
* 
* Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
int counter = 0,counttx = 0;
char text[1] = "A";
unsigned long milisekon;
RF24 radio(10, 9);  // CE, CSN
float totaltime = 0;
unsigned long starttime;
bool flagclear=0;

const byte address[6] = "94321";
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  milisekon = millis();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("TX Ready");
  delay(1000);
  lcd.clear();
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.stopListening();
}

void loop() {
  if (millis() - milisekon > 999) {
    milisekon = millis();
    if (counttx++ < 20) {
      radio.stopListening();
      radio.openWritingPipe(address);
      radio.write(&text, sizeof(text));
      starttime = micros();
      radio.openReadingPipe(0, address);
      radio.startListening();
    }
  }
  if (radio.available()) {
    counter++;
    float time = ((float)(micros() - starttime) / 2)/1000;
    totaltime += time;
    char data[32] = "";
    radio.read(&data, sizeof(text));
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("P :" + (String)counter);
    lcd.setCursor(0, 1);
    lcd.print("T :" + (String)time + "ms");
  }
  if (counttx > 20) {
    if(!flagclear){
      lcd.clear();
      flagclear = 1;
    }
    lcd.setCursor(0, 0);
    lcd.print("Ringkasan Tes : ");
    lcd.setCursor(0, 1);
    int hasil = 20 - counter;
    float waktutotal = totaltime/counter;
    lcd.print("PL :" + (String)hasil);
    lcd.setCursor(7, 1);
    lcd.print("T :" + (String)waktutotal + "ms");
  }
}