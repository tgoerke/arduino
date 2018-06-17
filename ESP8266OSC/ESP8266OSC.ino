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

  - send NoteOn & NoteOff OSC messages to Renoise
  - send Analog Filter Cutoff to renoise
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

char ssid[] = "DLRobotik";          // your network SSID (name)
char pass[] = "Aquabot!";           // your network password

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
unsigned int outPort = 9999;          // remote port (not needed for receive)
const unsigned int localPort = 8888;  // local port to listen for UDP packets


OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*
unsigned int osc_host = 215;              // last octet of osc server host
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

}

void osc(OSCMessage &msg) {
  osc_host = msg.getInt(0);
  Serial.print("/osc host: ");
  Serial.println(osc_host); 
  if (msg.isInt(1)) {
     outPort = msg.getInt(1);
     Serial.print("/osc port: ");
     Serial.println(outPort);
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

void sendNums(int sensor, IPAddress outIp, int outPort) {
  OSCMessage msg("/nums");
  msg.add((int)sensor).add((int)42);
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

void loop() {  
  Serial.print("osc: host, port -> ");
  Serial.print(osc_host); 
  Serial.print(", ");
  Serial.println(outPort); 
  IPAddress outIp(192,168,1,osc_host); 

  // fun with ReNoise
  // http://tutorials.renoise.com/wiki/Open_Sound_Control
  int note = map(analogRead(A0),0,1024,0,119);
  Serial.println(note);
  
  //sendRenoiseNoteOn(note, outIp, outPort);   delay(100);
  //sendRenoiseNoteOff(note, outIp, outPort);  //delay(1000);

  int sensor = analogRead(A0);
  Serial.println(sensor);
  sendPot(sensor, outIp, outPort);  //delay(1000);
  sendNums(sensor, outIp, outPort);  //delay(1000);

  /* Receive OSC message */
  OSCMessage msg;
  int size = Udp.parsePacket();
  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/led", led);
      msg.dispatch("/osc", osc);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }
}
