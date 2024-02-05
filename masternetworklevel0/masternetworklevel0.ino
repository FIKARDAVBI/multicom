#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>
#include <INA226.h>

#define inaaddress 0x40
#define button 2
#define led A3
#define topology_T  //T untuk Tree,
//#define topology_S //S untuk Star,
//#define topology_D //D untuk tree lain
#define protocol_R  //R untuk request based timer up dan downlink
//#define protocol_B //B untuk roundrobin and append hanya uplink
#include <LiquidCrystal_I2C.h>

const uint16_t this_node = 00;

RF24 radio(10, 9);           // nRF24L01 (CE,CSN)
RF24Network network(radio);  // Include the radio in the network
INA226 ina;
float energy;
float pwr;

#ifdef topology_T  // konfigurasi address topology tree
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 021;
const uint16_t node05 = 012;
const uint16_t node06 = 022;
const uint8_t n1 = 1;
const uint8_t n2 = 2;
const uint8_t n3 = 9;
const uint8_t n4 = 17;
const uint8_t n5 = 10;
const uint8_t n6 = 18;
#elif defined(topology_S)  // konfigurasi address topology star
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 03;
const uint16_t node04 = 04;
const uint16_t node05 = 05;
const uint16_t node06 = 015;
const uint8_t n1 = 1;
const uint8_t n2 = 2;
const uint8_t n3 = 3;
const uint8_t n4 = 4;
const uint8_t n5 = 5;
const uint8_t n6 = 13;
#else                      // konfigurasi address topology tree lainnya
const uint16_t node01 = 01;
const uint16_t node02 = 02;
const uint16_t node03 = 011;
const uint16_t node04 = 012;
const uint16_t node05 = 0111;
const uint16_t node06 = 0112;
const uint8_t n1 = 1;
const uint8_t n2 = 2;
const uint8_t n3 = 9;
const uint8_t n4 = 17;
const uint8_t n5 = 73;
const uint8_t n6 = 74;
#endif

#define dummyflow
//#define sensorflow

LiquidCrystal_I2C lcd(0x27, 16, 2);
unsigned char databuffer[MAX_PAYLOAD_SIZE];
unsigned char datareq = 'R';
uint16_t payloadSize;
unsigned long timer;
unsigned long timer2;
int count = 0;
int interval = 4000;
const int samplingtime = 100;
int packetsent = 0;
int packetreceive = 0;

#ifdef dummyflow
double flow = 0.45;
#else
const int intervalflow = 100;
double flow = 0.00;
int faktorkalibrasi = 170;
float kalibrasi;
#endif

volatile int NumPulses = 0;
unsigned long prevmillis;

struct datatosend_t {
  double node1;
  double node2;
  double node3;
  double node4;
  double node5;
  double node6;
};
datatosend_t datatosend;


void handlingdata() {
  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;    // If so, take a look at it
    payloadSize = network.peek(header);
    packetreceive++;
    if (header.from_node == n1) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node1 = atof(databuffer);
    } else if (header.from_node == n2) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node2 = atof(databuffer);
    } else if (header.from_node == n3) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node3 = atof(databuffer);
    } else if (header.from_node == n4) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node4 = atof(databuffer);
    } else if (header.from_node == n5) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node5 = atof(databuffer);
    } else if (header.from_node == n6) {
      network.read(header, &databuffer, payloadSize);
      datatosend.node6 = atof(databuffer);
    } else {
      Serial.println(header.from_node);
      network.read(header, 0, 0);
    }
  }
}

void requestdata() {
  if (millis() - timer > interval) {
    timer = millis();
    if (count++ != 6) packetsent++;
    switch (count) {
      case 1:
        {
          RF24NetworkHeader header2(node01);
          bool ok = network.write(header2, &datareq, sizeof(datareq));
          break;
        }
      case 2:
        {
          RF24NetworkHeader header3(node02);
          bool ok2 = network.write(header3, &datareq, sizeof(datareq));
          break;
        }
      case 3:
        {
          RF24NetworkHeader header4(node03);
          bool ok3 = network.write(header4, &datareq, sizeof(datareq));
          break;
        }
      case 4:
        {
          RF24NetworkHeader header5(node04);
          bool ok4 = network.write(header5, &datareq, sizeof(datareq));
          break;
        }
      case 5:
        {
          RF24NetworkHeader header6(node05);
          bool ok5 = network.write(header6, &datareq, sizeof(datareq));
          break;
        }
      case 6:
        {
          RF24NetworkHeader header7(node06);
          bool ok6 = network.write(header7, &datareq, sizeof(datareq));
          break;
        }
      case 7:
        {
          Serial.print((String)datatosend.node1 + "," + (String)datatosend.node2 + "," + (String)datatosend.node3 + "," + (String)datatosend.node4 + "," + (String)datatosend.node5 + "," + (String)datatosend.node6 + ",");
          Serial.println(flow,2);
          count = 0;
          break;
        }
    }
  }
}

void PulseCount() {
  NumPulses++;
}

void handlingappendeddata() {
  while (network.available()) {
    RF24NetworkHeader header;  // If so, take a look at it
    payloadSize = network.peek(header);
    packetreceive++;
    switch (header.from_node) {
      case 1:
        {
          network.read(header, &databuffer, payloadSize);
          delay(100);
          Serial.print((char)databuffer+",");
          Serial.println(flow,2);
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
  lcd.print("Master");
  delay(1000);
  lcd.clear();
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)
  radio.setDataRate(RF24_2MBPS);
  Serial.begin(9600);
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
  requestdata();
  handlingdata();
#else
  handlingappendeddata();
#endif
  ambildatapower();
  tampillcd();
}