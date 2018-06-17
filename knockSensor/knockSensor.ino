/*
  Knock Counter 
  T.Goerke 11/2017
  based on Knock Sensor
  http://www.arduino.cc/en/Tutorial/Knock
*/


const int ledPin = 11;          // knock indicator LED
const int knockSensor = A5;    // the piezo is connected to an analog pin
const int knockThreshold = 20; // threshold value to decide when the detected sound is a knock or not

void setup() {
  Serial.begin(9600);       // use the serial port
  pinMode(ledPin, OUTPUT); // declare the ledPin as as OUTPUT

}
  
int knock() {
  Serial.print(analogRead(knockSensor));
  digitalWrite(ledPin, HIGH);
  delay(200);
  int count = -1;
  unsigned long t = millis();
  while (millis()-t < 2000) {
    if (analogRead(knockSensor) >= knockThreshold && millis()-t > 100) {
      digitalWrite(ledPin, HIGH);
      Serial.print("X ");
      count++;
      delay(200);
      t = millis();
      digitalWrite(ledPin, LOW);
    }
  }   
  return count;
}

void loop() {
  if (analogRead(knockSensor) >= 100) {
    Serial.print("knock: ");
    Serial.println(knock());
  }
}
