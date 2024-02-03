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

unsigned long timer;

const uint16_t master = 00;
#ifdef topology_T  // konfigurasi address topology tree
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 021;
const uint16_t node05 = 012;
const uint16_t this_node = 022;
#elif defined(topology_S)  // konfigurasi address topology star
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 03;
const uint16_t node04 = 04;
const uint16_t node05 = 05;
const uint16_t this_node = 015;
#else                      // konfigurasi address topology tree lainnya
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 012;
const uint16_t node05 = 0111;
const uint16_t this_node = 0112;
#endif

#define dummyflow
//#define sensorflow

const unsigned long interval = 12000;  //ms  // How often to send data to the other unit
unsigned long last_sent;               // When did we last send?
unsigned char databuffer[MAX_PAYLOAD_SIZE];
unsigned char datatosend[MAX_PAYLOAD_SIZE];
uint16_t payloadSize;

#ifdef dummyflow
double flow = 6.45;
#else
const int intervalflow = 100;
double flow = 0.00;
int faktorkalibrasi = 170;
float kalibrasi;
#endif

const int samplingtime = 100;
unsigned long timer2;
unsigned long prevmillis;
int packetsent = 0;
int packetreceive = 0;
volatile int NumPulses = 0;


void handlingdata() {
  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;    // If so, take a look at it
    payloadSize = network.peek(header);
    packetreceive++;
    memset(databuffer, 0, sizeof(databuffer));
    memset(datatosend, 0, sizeof(datatosend));
    switch (header.from_node) {
      case 00:
        {
          network.read(header, &databuffer, payloadSize);
          sprintf(datatosend, "%f", flow);
          delay(100);
          RF24NetworkHeader header2(master);
          packetsent++;
          bool ok = network.write(header2, &datatosend, sizeof(datatosend));
          break;
        }
      default:
        {
          network.read(header, 0, 0);
        }
        break;
    }
  }
}

void ambildatapower() {
  if (millis() - timer2 > samplingtime) {
    pwr = ina.readBusPower();
    energy += pwr / (3600000 / samplingtime);
  }
}

void tampillcd() {
  lcd.setCursor(0, 0);
  lcd.print("P: ");
  lcd.setCursor(3, 0);
  lcd.print(energy, 2);
  lcd.setCursor(9, 0);
  lcd.print("S: ");
  lcd.setCursor(12, 0);
  lcd.print(packetsent);
  lcd.setCursor(0, 1);
  lcd.print("R: ");
  lcd.setCursor(3, 1);
  lcd.print(packetreceive);
  lcd.setCursor(9, 1);
  lcd.print("F: ");
  lcd.setCursor(12, 1);
  lcd.print(flow);
}

void PulseCount() {
  NumPulses++;
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
  lcd.print("Node6");
  delay(1000);
  lcd.clear();
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)
  radio.setDataRate(RF24_2MBPS);
  timer = millis();
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
  if (milis() - timer > interval) {
    timer = millis();
    memset(datatosend, 0, sizeof(datatosend));
    sprintf(datatosend, "%f", flow);
    RF24NetworkHeader header3(node05);
    packetsent++;
    bool ok2 = network.write(header3, &datatosend, sizeof(datatosend));
  }
#endif
  ambildatapower();
  tampillcd();
}