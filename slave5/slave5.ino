#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <stdio.h>
#include <Wire.h>
#include <INA226.h>
#include <LiquidCrystal_I2C.h>

#define inaaddress 0x44
INA226 ina;
float energy;
float pwr;

LiquidCrystal_I2C lcd(0x27, 16, 2);

RF24 radio(10, 9);           // nRF24L01 (CE,CSN)
RF24Network network(radio);  // Include the radio in the network

#define topology_T  //T untuk Tree,
//#define topology_S //S untuk Star,
//#define topology_D //D untuk tree lain
#define protocol_R  //R untuk request based timer
//#define protocol_B //B untuk round robin and append
const uint16_t master = 00;
#ifdef topology_T  // konfigurasi address topology tree
const uint16_t node01 = 01;
const uint8_t prevnode = 18;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 021;
const uint16_t this_node = 012;
const uint16_t node06 = 022;
#elif defined(topology_S)  // konfigurasi address topology star
const uint16_t node01 = 01;
const uint8_t prevnode = 13;
const uint16_t node02 = 02;
const uint16_t node03 = 03;
const uint16_t node04 = 04;
const uint16_t this_node = 05;
const uint16_t node06 = 015;
#else                      // konfigurasi address topology tree lainnya
const uint16_t node01 = 01;
const uint8_t prevnode = 74;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 012;
const uint16_t this_node = 0111;
const uint16_t node06 = 0112;
#endif

#define dummyflow
//#define sensorflow

const unsigned long interval = 500;  //ms  // How often to send data to the other unit
unsigned long last_sent;             // When did we last send?
unsigned char databuffer[MAX_PAYLOAD_SIZE];
unsigned char datatosend[MAX_PAYLOAD_SIZE];

#ifdef dummyflow
double flow = 5.45;
#else
const int intervalflow = 100;
double flow = 0.00;
int faktorkalibrasi = 170;
float kalibrasi;
#endif

uint16_t payloadSize;
const int samplingtime = 100;
unsigned long timer2;
int packetsent = 0;
int packetreceive = 0;
volatile int NumPulses = 0;
unsigned long prevmillis;
unsigned char flowstr[10];
int flag;
const int numboftry = 15;

void handlingdata() {
  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;    // If so, take a look at it
    payloadSize = network.peek(header);
    network.read(header, &databuffer, payloadSize);
    packetreceive++;
    switch (header.from_node) {
      case 00:
        {
          RF24NetworkHeader header2(master);
          dtostrf(flow, sizeof(flow), 2, datatosend);
          bool ok = false;
          flag = 0;
          while (!ok && flag < numboftry) {
            ok = network.write(header2, &datatosend, strlen(datatosend));
            flag++;
            delayMicroseconds(50);
          }
          if (ok) packetsent++;
          break;
        }
      default:
        {
          network.read(header, 0, 0);
          break;
        }
    }
  }
}

void appendforward() {
  while (network.available()) {
    RF24NetworkHeader header;  // If so, take a look at it
    payloadSize = network.peek(header);
    network.read(header, &databuffer, payloadSize);
    packetreceive++;
    switch (header.from_node) {
      case prevnode:
        {
          dtostrf(flow, sizeof(flow), 2, flowstr);
          sprintf(datatosend, "%s,%s", databuffer, flowstr);
          RF24NetworkHeader header3(node04);
          bool ok2 = false;
          flag = 0;
          while (!ok2 && flag < numboftry) {
            ok2 = network.write(header3, &datatosend, strlen(datatosend));
            flag++;
            delayMicroseconds(50);
          }
          if (ok2) packetsent++;
          break;
        }
      default:
        {
          network.read(header, 0, 0);
          break;
        }
    }
  }
}

void PulseCount() {
  NumPulses++;
}

void ambildatapower() {
  if (millis() - timer2 > samplingtime) {
    pwr = ina.readBusPower();
    energy += pwr / (3600000 / samplingtime);
  }
}

void tampillcd() {
  lcd.setCursor(0, 0);
  lcd.print("P:");
  lcd.setCursor(2, 0);
  lcd.print(energy, 2);
  lcd.setCursor(8, 0);
  lcd.print("S:");
  lcd.setCursor(10, 0);
  lcd.print(packetsent);
  lcd.setCursor(0, 1);
  lcd.print("R:");
  lcd.setCursor(2, 1);
  lcd.print(packetreceive);
  lcd.setCursor(8, 1);
  lcd.print("F:");
  lcd.setCursor(10, 1);
  lcd.print(flow, 2);
}
void setup() {
#ifdef sensorflow
  attachInterrupt(1, PulseCount, RISING);
  kalibrasi = (float)faktorkalibrasi / 83;
#endif
  ina.begin(inaaddress);
  ina.configure(INA226_AVERAGES_1, INA226_BUS_CONV_TIME_1100US, INA226_SHUNT_CONV_TIME_1100US, INA226_MODE_SHUNT_BUS_CONT);
  ina.calibrate(0.01, 4);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Node5");
  delay(1000);
  lcd.clear();
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)
  radio.setDataRate(RF24_2MBPS);
  timer2 = millis();
  prevmillis = micros();
}

void loop() {
  network.update();

#ifdef sensorflow
  if (micros() - prevmillis > intervalflow) {
    detachInterrupt(1);
    float flowRate = (float)NumPulses / kalibrasi;
    prevmillis = micros();
    attachInterrupt(1, PulseCount, RISING);
    float flowLitres = (flowRate / 60);
    flow += flowLitres;
  }
#endif

#ifdef protocol_R
  handlingdata();
#else
  appendforward();
#endif
  ambildatapower();
  tampillcd();
}