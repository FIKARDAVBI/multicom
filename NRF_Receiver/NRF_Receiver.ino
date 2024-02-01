#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(10, 9);  // CE, CSN

const byte address[6] = "94321";
#include <LiquidCrystal_I2C.h>
int count;

LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Receiver Ready");
  delay(1000);
  lcd.clear();
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    count++;
    radio.read(&text, sizeof(text));
    lcd.clear();
    radio.stopListening();
    radio.openWritingPipe(address);
    radio.write(text, sizeof(text));
    radio.openReadingPipe(0, address);
    radio.startListening();
    lcd.print("Paket Diterima:");
    lcd.setCursor(0, 1);
    lcd.print(count);
    lcd.setCursor(0, 0);
  }
}