#include <SoftwareSerial.h>

SoftwareSerial mySerial(10, 11); // RX, TX

const int pinX = A0;
const int pinY = A1;
const int pinBoton = 2;
const int ledX = 3;
const int ledY = 5;

unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 50;

unsigned long ultimoByteRecibido = 0;
const unsigned long timeoutEnlace = 2000;
bool enlacePerdido = false;

unsigned long ultimoAntirrebote = 0;
const unsigned long debounceDelay = 50;
bool estadoAnteriorBoton = HIGH;

void setup() {
  pinMode(pinBoton, INPUT_PULLUP);
  pinMode(ledX, OUTPUT);
  pinMode(ledY, OUTPUT);
  
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  unsigned long currentMillis = millis();

  int rawX = analogRead(pinX);
  int rawY = analogRead(pinY);
  
  int xCoord = (rawX * 100L) / 1024;
  int yCoord = (rawY * 100L) / 1024;

  if (currentMillis - ultimoEnvio >= intervaloEnvio) {
    ultimoEnvio = currentMillis;
    mySerial.write(xCoord);
    mySerial.write(yCoord);
  }

  bool estadoBoton = digitalRead(pinBoton);
  if (estadoBoton == LOW && estadoAnteriorBoton == HIGH && (currentMillis - ultimoAntirrebote) > debounceDelay) {
    ultimoAntirrebote = currentMillis;
    mySerial.write(0xFF); 
  }
  estadoAnteriorBoton = estadoBoton;

  while (mySerial.available() > 0) {
    byte incoming = mySerial.read();
    ultimoByteRecibido = currentMillis;
    enlacePerdido = false;
  }

  if (currentMillis - ultimoByteRecibido > timeoutEnlace) {
    enlacePerdido = true;
    if ((currentMillis / 500) % 2 == 0) {
      analogWrite(ledX, 255);
      analogWrite(ledY, 0);
    } else {
      analogWrite(ledX, 0);
      analogWrite(ledY, 255);
    }
  }
}