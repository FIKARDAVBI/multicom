#include <RF24Network.h>
#include <RF24.h>
#include <SPI.h>
#include <stdio.h>

RF24 radio(10, 9);               // nRF24L01 (CE,CSN)
RF24Network network(radio);      // Include the radio in the network

#define topology_T //T untuk Tree, 
//#define topology_S //S untuk Star, 
//#define topology_D //D untuk tree lain
#define protocol_R //R untuk request based timer
//#define protocol_B //B untuk round robin and append
const uint16_t master = 00;
#ifdef topology_T // konfigurasi address topology tree
const uint8_t prevnode = 17;
const uint16_t this_node = 011;
const uint16_t node01 = 01;     
const uint16_t node02 = 02;
const uint16_t node04 = 021;
const uint16_t node05 = 012;
const uint16_t node06 = 022;
#elif defined(topology_S) // konfigurasi address topology star
const uint8_t prevnode = 4;
const uint16_t this_node = 03;
const uint16_t node01 = 01;     
const uint16_t node02 = 02;
const uint16_t node04 = 04;
const uint16_t node05 = 05;
const uint16_t node06 = 015;
#else // konfigurasi address topology tree lainnya 
const uint8_t prevnode = 10;
const uint16_t this_node = 011;
const uint16_t node01 = 01;    
const uint16_t node02 = 02; 
const uint16_t node04 = 012;
const uint16_t node05 = 0111;
const uint16_t node06 = 0112;
#endif

const unsigned long interval = 500;  //ms  //unsigned char databuffer[MAX_PAYLOAD_SIZE];
unsigned char databuffer[MAX_PAYLOAD_SIZE];
float flow = 3.45;
unsigned char datatosend[MAX_PAYLOAD_SIZE];
uint16_t payloadSize;

void handlingdata(){
  while (network.available()) {  // Is there anything ready for us?
    RF24NetworkHeader header;  // If so, take a look at it
    payloadSize = network.peek(header);
    memset(databuffer, 0, sizeof(databuffer));
    memset(datatosend, 0, sizeof(datatosend));
    switch (header.from_node) {
      case 00:
        {network.read(header,&databuffer,payloadSize);
        sprintf(datatosend,"%f",flow);
        delay(100);
        RF24NetworkHeader header2(master);
        bool ok = network.write(header2,&datatosend,sizeof(datatosend));
        break;}
      default:
        {network.read(header, 0,0);}
        break;
    }
  }
}

void appendforward(){
  while(network.available()){
    RF24NetworkHeader header;  // If so, take a look at it
    payloadSize = network.peek(header);
    memset(databuffer, 0, sizeof(databuffer));
    memset(datatosend, 0, sizeof(datatosend));
    switch (header.from_node) {
      case prevnode:
        {network.read(header,&databuffer,payloadSize);
        sprintf(datatosend,"%s,%f",databuffer,flow);
        delay(100);
        RF24NetworkHeader header3(node02);
        bool ok = network.write(header3,&datatosend,sizeof(datatosend));
        break;}
      default:
        {network.read(header, 0,0);}
        break;
    }
  }
}

void setup() {
  SPI.begin();
  radio.begin();
  network.begin(90, this_node);  //(channel, node address)
  radio.setDataRate(RF24_2MBPS);
}

void loop() {
  network.update();

#ifdef protocol_R
  handlingdata();
#else
  appendforward();
#endif

/*
#ifdef topology_T
  #ifdef protocol_R
  handlingdata();
  #else
  #endif
#elif defined(topology_S)
  #ifdef protocol_R
  handlingdata();
  #else
  #endif
#else
  #ifdef protocol_R
  handlingdata();
  #else
  #endif
#endif*/
  /*
  //===== Sending =====//
  unsigned long now = millis();
  if (now - last_sent >= interval) {   // If it's time to send a data, send it!
    last_sent = now;
    RF24NetworkHeader header(master00);   // (Address where the data is going)
    bool ok = network.write(header, &databuffer, sizeof(databuffer)); // Send the data
  }*/
}