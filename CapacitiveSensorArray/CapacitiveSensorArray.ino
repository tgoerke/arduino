#include <CapacitiveSensor.h>
#include <MIDI.h>

MIDI_CREATE_DEFAULT_INSTANCE();

const int KeyCount = 3;  // Number of capacitive sensor keys
const int KeyThreshold = 100; 

CapacitiveSensor Key[KeyCount] = { CapacitiveSensor(8,2), CapacitiveSensor(8,3), CapacitiveSensor(8,4) }; 

int keyPressed(int i) {
  return ( Key[i].capacitiveSensor(10) > KeyThreshold );
}    

int keyValue(int i) {
  return Key[i].capacitiveSensor(10) - KeyThreshold;
}

void keyPrint()
{
  for (int i=0; i<KeyCount; i++) {
    Serial.print(Key[i].capacitiveSensor(10));
    Serial.print("\t");  
  }
  Serial.print("\n");  
}

void keyValuePrint() {
  for (int i=0; i<KeyCount; i++) {
    Serial.print(keyPressed(i));
    Serial.print(" -> ");  
    if (keyPressed(i)) Serial.print(keyValue(i));
    Serial.print("\t"); 
  }
  Serial.print("\n");  
}    

void setup()                    
{   
   for (int i=0; i<KeyCount; i++) 
     Key[i].set_CS_Timeout_Millis(100);

   //MIDI.begin(MIDI_CHANNEL_OMNI);
   
   Serial.begin(9600);
}


void loop()
{
  keyValuePrint();
  //if (keyPressed(1)) MIDI.sendNoteOn(36, 127, 1);
  //if (keyPressed(2)) MIDI.sendNoteOn(48, 127, 1); else MIDI.sendNoteOff(48, 0, 1);;
  //if (keyPressed(3)) MIDI.sendNoteOn(60, 127, 1);
  //delay(1000);		            // Wait for a second
  //MIDI.sendNoteOff(46, 0, 1);
  //MIDI.sendNoteOff(48, 0, 1);
  //MIDI.sendNoteOff(60, 0, 1);
  //MIDI.sendControlChange(26, analogRead(A5), 1);
  // cc A, cc B, cc C, cc D: Value of the Control Change 16, 17, 18 and 19.
  /*

https://mutable-instruments.net/archive/shruthi/manual/

Parameter 	        CC number
SVF Filter cutoff 2 	12
SVF Filter resonance 2 	13
SVF Filter mode 1 	85
SVF Filter mode 2 	86
  */
}

