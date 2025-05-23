#include <Arduino.h>
#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN_A = 13;
const int LOADCELL_SCK_PIN_A = 12;

const int LOADCELL_DOUT_PIN_B = 14;
const int LOADCELL_SCK_PIN_B = 27;

const int LOADCELL_DOUT_PIN_C = 26;
const int LOADCELL_SCK_PIN_C = 25;

const int LOADCELL_DOUT_PIN_D = 33;
const int LOADCELL_SCK_PIN_D = 32;

HX711 scaleA;
HX711 scaleB;
HX711 scaleC;
HX711 scaleD;

void setup() {
  //code runs once
  Serial.begin(57600); //set baud rate to 57 600
  //Initialise all load cells
  scaleA.begin(LOADCELL_DOUT_PIN_A, LOADCELL_SCK_PIN_A);
  scaleB.begin(LOADCELL_DOUT_PIN_B, LOADCELL_SCK_PIN_B);
  scaleC.begin(LOADCELL_DOUT_PIN_C, LOADCELL_SCK_PIN_C);
  scaleD.begin(LOADCELL_DOUT_PIN_D, LOADCELL_SCK_PIN_D);
}

void loop() {

  if (scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()) {
    scaleA.set_scale();   
    scaleB.set_scale();
    scaleC.set_scale(); 
    scaleD.set_scale();
      
    Serial.println("Tare... remove any weights from the scale.");
    delay(5000);
    //Tare Callibration
    scaleA.tare();
    scaleB.tare();
    scaleC.tare();
    scaleD.tare();

    Serial.println("Tare done...");
    Serial.print("Place a known weight on the scale...");
    delay(5000);

    long readingA = scaleA.get_units(20);
    long readingB = scaleB.get_units(20);
    long readingC = scaleC.get_units(20);
    long readingD = scaleD.get_units(20);

    Serial.print("Result: ");
    Serial.println(readingA);
    Serial.println(readingB);
    Serial.println(readingC);
    Serial.println(readingD);
  } 
  else {
    Serial.println("HX711 not found.");
  }
  delay(1000);
}

//calibration factor will be the (reading)/(known weight)
