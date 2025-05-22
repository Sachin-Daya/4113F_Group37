//This is the code that will be used when deploying the system
#include <Arduino.h>
#include <HX711.h>

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN_A = 13;
const int LOADCELL_SCK_PIN_A = 12;

const int LOADCELL_DOUT_PIN_B = 14;
const int LOADCELL_SCK_PIN_B = 27;

const int LOADCELL_DOUT_PIN_C = 26;
const int LOADCELL_SCK_PIN_C = 25;

const int LOADCELL_DOUT_PIN_D = 33;
const int LOADCELL_SCK_PIN_D = 32;

//Loadcell Callibration Factors, obtained using the Scale_Calibration File with a known weight
const int CAL_FACTOR_A = -32.177;
const int CAL_FACTOR_B = -105.496;
const int CAL_FACTOR_C = -147.5453;
const int CAL_FACTOR_D = -83.841;

HX711 scaleA;
HX711 scaleB;
HX711 scaleC;
HX711 scaleD;

void setup() {

  //code runs once
  Serial.begin(57600); //set baud rate of serial monitor to 57 600
  //Initialise all load cells
  scaleA.begin(LOADCELL_DOUT_PIN_A, LOADCELL_SCK_PIN_A);
  scaleB.begin(LOADCELL_DOUT_PIN_B, LOADCELL_SCK_PIN_B);
  scaleC.begin(LOADCELL_DOUT_PIN_C, LOADCELL_SCK_PIN_C);
  scaleD.begin(LOADCELL_DOUT_PIN_D, LOADCELL_SCK_PIN_D);

  if(scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()){
  //Set the scale factors for each load cell
    scaleA.set_scale(CAL_FACTOR_A);   
    scaleB.set_scale(CAL_FACTOR_B);
    scaleC.set_scale(CAL_FACTOR_C); 
    scaleD.set_scale(CAL_FACTOR_D);

    //Tare Callibration
    Serial.println("TARE...");
    scaleA.tare();
    scaleB.tare();
    scaleC.tare();
    scaleD.tare();
    Serial.println("PLACE ITEM ON SCALE NOW");
    delay(5000);
}//if

}//setup()

void loop() {
  int RFID_flag = 0;

  // put your main code here, to run repeatedly:
if(RFID_flag == 1){
  Serial.println("RFID DETECTED - BEGINNING SCALE READING");

  if (scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()) {
    Serial.println("READING...");
    long readingA = scaleA.get_units(20);
    long readingB = scaleB.get_units(20);
    long readingC = scaleC.get_units(20);
    long readingD = scaleD.get_units(20);

    Serial.println("RESULTS...");
    long avgWeight = (readingA+readingB+readingC+readingD)/4;
    Serial.println(readingA);
    Serial.println(readingB);
    Serial.println(readingC);
    Serial.println(readingD);
    Serial.println("AVERAGE WEIGHT:");
    Serial.print(avgWeight);
    Serial.println("_____________________________________________________________");
    
  }else{
    Serial.println("ERROR: HX711 NOT FOUND");
  }//if
  RFID_flag = 0;
}//if2
}//loop()
