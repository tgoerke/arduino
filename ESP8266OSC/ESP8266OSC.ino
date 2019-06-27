/*---------------------------------------------------------------------------------------------

  Open Sound Control (OSC) library for the ESP8266/ESP32

  Example for receiving open sound control (OSC) messages on the ESP8266/ESP32
  Send integers '0' or '1' to the address "/led" to turn on/off the built-in LED of the esp8266.

  This example code is in the public domain.

  Changes:

  T.Goerke 05/18
  - combined send & receive
  - OSC messages can be used to set outgoing osc server (last octet) & port (optional)

    Example: (using https://github.com/nulltask/osc-cli)
    
                                NoceMCU IP            OSC IP
      pi@nodered:~$ osc --host 192.168.1.207:8888 /osc 215 9999
                                                            OSC Port

  - add MPR121 cap sensor board
  - send NoteOn & NoteOff OSC messages to Renoise
  - (send Analog Filter Cutoff to renoise)
--------------------------------------------------------------------------------------------- */
#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

#include <Wire.h>
#include "Adafruit_MPR121.h"
Adafruit_MPR121 cap = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

char* players[]={
"Brian",
"Fynn",
"Jannes",
"Aron",
"Konrad",
"Hannes",
"Julian",
"Alejandro",
"Marco",
"Noah",
"Fernando",
"Achim",
"Torsten"
};

char ssid[] = "DLRobotik";          // your network SSID (name)
char pass[] = "Aquabot!";           // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
const unsigned int localPort = 8888;  // local port to listen for UDP packets

unsigned int maxPort = 9999;          // remote port (not needed for receive)
unsigned int renoisePort = 9000;      // remote port (not needed for receive)
unsigned int p5Port = 9990;           // remote port (not needed for receive)

unsigned int maxHost = 10;            // last octet of osc server host
unsigned int renoiseHost = 10;        // last octet of osc server host
unsigned int p5Host = 10;             // last octet of osc server host

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*
#ifndef BUILTIN_LED
#ifdef LED_BUILTIN
#define BUILTIN_LED LED_BUILTIN
#else
#define BUILTIN_LED 13
#endif
#endif

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led

  Serial.begin(115200);

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

  delay(20);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor...");   
  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
}

void renoise(OSCMessage &msg) {
  renoiseHost = msg.getInt(0);
  Serial.println("/set/renoise");
  Serial.print("host: "); Serial.println(renoiseHost); 
  if (msg.isInt(1)) {
     renoisePort = msg.getInt(1);
     Serial.print("port: ");
     Serial.println(renoisePort);
  }   
}
void maxmsp(OSCMessage &msg) {
  maxHost = msg.getInt(0);
  Serial.println("/set/max");
  Serial.print("host: "); Serial.println(maxHost); 
  if (msg.isInt(1)) {
     maxPort = msg.getInt(1);
     Serial.print("port: ");
     Serial.println(maxPort);
  }   
}
void p5(OSCMessage &msg) {
  p5Host = msg.getInt(0);
  Serial.println("/set/p5");
  Serial.print("host: "); Serial.println(p5Host); 
  if (msg.isInt(1)) {
     p5Port = msg.getInt(1);
     Serial.print("port: ");
     Serial.println(p5Port);
  }   
}

void led(OSCMessage &msg) {
  ledState = msg.getInt(0);
  digitalWrite(BUILTIN_LED, ledState);
  Serial.print("/led: ");
  Serial.println(ledState);
}

void sendRenoiseNoteOn(int note, IPAddress outIp, int outPort) {
  OSCMessage msg("/renoise/trigger/note_on");
  msg.add((int)1).add((int)1).add(note).add((int)127);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendPot(int sensor, IPAddress outIp, int outPort) {
  OSCMessage msg("/pot");
  msg.add((int)sensor);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendRenoiseNoteOff(int note, IPAddress outIp, int outPort) {  
  OSCMessage msg("/renoise/trigger/note_off");
  msg.add((int)1).add((int)1).add(note);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}  

void sendRenoiseEffect(float cutoff, IPAddress outIp, int outPort) { 
  OSCMessage msg("/renoise/song/track/1/device/2/set_parameter_by_index");
  msg.add(2).add((float)cutoff);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendRenoiseBPM(int bpm, IPAddress outIp, int outPort) { 
  OSCMessage msg("/renoise/song/bpm");
  msg.add(2).add((int)bpm);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendRenoiseTransportStart(IPAddress outIp, int outPort) { 
  OSCMessage msg("/renoise/transport/start");
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendRenoiseTransportStop(IPAddress outIp, int outPort) { 
  OSCMessage msg("/renoise/transport/stop");
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void sendPlayer(int id, char* name, IPAddress outIp, int outPort) {
  OSCMessage msg("/player");
  msg.add(id);
  msg.add(name);
  Udp.beginPacket(outIp, outPort);
  msg.send(Udp);
  Udp.endPacket();
  msg.empty();
}

void loop() {  

  IPAddress maxIp(192,168,1,maxHost); 
  IPAddress p5Ip(192,168,1,p5Host);
  IPAddress renoiseIp(192,168,1,renoiseHost);
  
  currtouched = cap.touched();

  // Get the currently touched pads
  currtouched = cap.touched();
  
  for (uint8_t i=0; i<12; i++) {
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & _BV(i)) && !(lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.print(" "); Serial.print(players[i]); Serial.println(" touched");
      sendRenoiseNoteOn(48+i, renoiseIp, renoisePort);   // C2 is 36, C3 is 48, C4 is 60
      sendPlayer(i, players[i], p5Ip, p5Port);
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
      Serial.print(i); Serial.print(" "); Serial.print(players[i]); Serial.println(" released");
      sendRenoiseNoteOff(48+i, renoiseIp, renoisePort); 
    }
  }

  // reset our state
  lasttouched = currtouched;

  // fun with ReNoise
  // http://tutorials.renoise.com/wiki/Open_Sound_Control
  //int note = map(analogRead(A0),0,1024,0,119);
  //Serial.println(note);
  //sendRenoiseNoteOn(note, outIp, outPort);   delay(100);
  //sendRenoiseNoteOff(note, outIp, outPort);  //delay(1000);
  
  //int sensor = analogRead(A0);
  //Serial.println(sensor);
  //sendPot(sensor, outIp, outPort);  //delay(1000);

  /* Receive OSC message */
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/led", led);
      msg.dispatch("/set/max", maxmsp);
      msg.dispatch("/set/renoise", renoise);
      msg.dispatch("/set/p5", p5);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
