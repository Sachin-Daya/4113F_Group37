//This is the code that will be used when deploying the system
#include <Arduino.h>
#include <HX711.h>
#include "driver/timer.h"

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

//Global Variables
HX711 scaleA;
HX711 scaleB;
HX711 scaleC;
HX711 scaleD;

hw_timer_t *tim00 = NULL;

void IRAM_ATTR timerInterruptHandler(){
  //This code handles the timer interrupt to occasionally measure the scale before anything is weighed
  scaleTare();
}//timerInterrupt

void readScale(){
    long readingA = scaleA.get_units(20);
    long readingB = scaleB.get_units(20);
    long readingC = scaleC.get_units(20);
    long readingD = scaleD.get_units(20);

    long avgWeight = (readingA+readingB+readingC+readingD)/4;
}//readScale

void initScale(){
  //Initialise all load cells
  scaleA.begin(LOADCELL_DOUT_PIN_A, LOADCELL_SCK_PIN_A);
  scaleB.begin(LOADCELL_DOUT_PIN_B, LOADCELL_SCK_PIN_B);
  scaleC.begin(LOADCELL_DOUT_PIN_C, LOADCELL_SCK_PIN_C);
  scaleD.begin(LOADCELL_DOUT_PIN_D, LOADCELL_SCK_PIN_D);
}//initScale

void setScaleFactors(){
    //Set the scale factors for each load cell
    scaleA.set_scale(CAL_FACTOR_A);   
    scaleB.set_scale(CAL_FACTOR_B);
    scaleC.set_scale(CAL_FACTOR_C); 
    scaleD.set_scale(CAL_FACTOR_D);
}//setScaleFactors

void scaleTare(){
  //Tare Callibration
    scaleA.tare();
    scaleB.tare();
    scaleC.tare();
    scaleD.tare();
}//scaleTare

void setup() {
  //This section initialises the timer interrupt
    tim00 = timerBegin(1);  // Use 1 Hz timer

    timerAttachInterrupt(tim00, &timerInterruptHandler);

    timerWrite(tim00, 300);  //call every 300 seconds
    //code runs once
    Serial.begin(57600); //set baud rate of serial monitor to 57 600
  
    initScale();

    if(scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()){
    setScaleFactors();
    scaleTare(); 
  }//if
}//setup()

void loop(){
  int RFID_flag = 0;

  if(RFID_flag == 1){//If the RFID is detected, begin measuring
  Serial.println("RFID DETECTED - BEGINNING SCALE READING");

  if (scaleA.is_ready() && scaleB.is_ready() && scaleC.is_ready() && scaleD.is_ready()) {
    readScale();
  }else{
    Serial.println("ERROR: HX711 NOT FOUND");
  }//if
  RFID_flag = 0;
}//if2
}//loop()
