#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CmdMessenger.h"

/* ---------------------------------------------------------------------------
 * Set values below to match "door" entry in farm json control file.         *
 * ------------------------------------------------------------------------- */

/* Serial communications */
const char *DEVICE_ID = "COOP_HARDWARE";

/* ---------------------------------------------------------------------------
 * Set values below to indicate which pin is controlling which piece of      *
 * attached hardware.                                                        *
 * ------------------------------------------------------------------------- */

const int AMBIENT_PIN = 0;      // ambient light sensor.  should be analog
const int HALL_OPEN_PIN = 1;    // open hall sensor (any pin)
const int HALL_CLOSED_PIN = 2;  // closed hall sensor (any pin)
const int MOTOR_PIN_0 = 5;      // control pin for motor. should be pwm.
const int MOTOR_PIN_1 = 6;      // control pin for motor. should be pwm.

/* ---------------------------------------------------------------------------
 * Control parameters for the motors themselves.                             *
 * ------------------------------------------------------------------------- */

// value below which the "open" hall sensor is reading closed
const int HALL_OPEN_CUTOFF = 20;

// value below which the "closed" hall sensor is reading closed
const int HALL_CLOSED_CUTOFF = 20;

// maximum amount of time to move the door (milliseconds)
const int DOOR_MOVE_TIMEOUT = 3000;  

// Duty cycle for PWM of door motor. controls motorfrom nothing (0) to full
// speed (255) 
const int DOOR_PWM_DUTY = 100; 

// How often to delay between checking motor movement
const int DOOR_SAMPLING_PERIOD = 50;

/* ---------------------------------------------------------------------------
 *                                                                           *
 * Stuff below here shouldn't need to be changed                             *
 *                                                                           *
 * ------------------------------------------------------------------------- */

const int BAUD_RATE = 9600;
const int SAMPLING_PERIOD = 100;

/* Attach a CmdMessenger instance to the default serial port */
CmdMessenger c = CmdMessenger(Serial,',',';','/');

/* Define available CmdMessenger commands */
enum {
    who_are_you,
    query,
    open_door,
    close_door,
    who_are_you_return,
    query_return,
    communication_error,
};

/* Callbacks */

void on_unknown_command(void){
    
    /* If there is an error */

    c.sendCmd(communication_error,"Command without callback.");
}

void on_who_are_you(void){
    
    /* Return the device */

    c.sendCmd(who_are_you_return,DEVICE_ID);
}

void on_query(void){

    /* Get the current reading from the door sensors and ambient light sensor */

    c.sendCmdStart(query_return);

    c.sendCmdBinArg(analogRead(AMBIENT_PIN));
    c.sendCmdBinArg(analogRead(HALL_OPEN_PIN));
    c.sendCmdBinArg(analogRead(HALL_CLOSED_PIN));

    c.sendCmdEnd();

}

void on_open_door(void){

    unsigned long max_end_time;

    /* Already open! */
    if (analogRead(HALL_OPEN_PIN) < HALL_OPEN_CUTOFF) {
        return;
    }    
    
    max_end_time = millis() + DOOR_MOVE_TIMEOUT;
   
    //* SET TO OPEN PIN CONFIGURATION ... WHATEVER THAT IS  
    while (millis() < max_end_time){
        if (analogRead(HALL_CLOSED_PIN) < HALL_CLOSED_CUTOFF) { 
            /* SET TO NEUTRAL CONFIG */
            return; 
        }
        delay(DOOR_SAMPLING_PERIOD); 
    }

    return;
        
}

void on_close_door(void){

    unsigned long max_end_time;

    /* Already closed! */
    if (analogRead(HALL_CLOSED_PIN) < HALL_CLOSED_CUTOFF) {
        return;
    }    
    
    max_end_time = millis() + DOOR_MOVE_TIMEOUT;
   
    //* SET TO CLOSED PIN CONFIGURATION ... WHATEVER THAT IS  
    while (millis() < max_end_time){
        if (analogRead(HALL_OPEN_PIN) < HALL_OPEN_CUTOFF) { 
            /* SET TO NEUTRAL CONFIG */
            return; 
        }
        delay(DOOR_SAMPLING_PERIOD); 
    }

    return;

}

/* Attach callback methods */
void attach_callbacks(void) { 
  
    c.attach(on_unknown_command);
    c.attach(who_are_you,on_who_are_you);
    c.attach(query,on_query);
    c.attach(open_door,on_open_door);
    c.attach(close_door,on_close_door);

}

/* ---------------------------------------------------------------------------
 * Set up (on boot or reset). 
 * -------------------------------------------------------------------------- */

void setup() {
    
    /* Initialize serial communication at BAUD_RATE bits per second: */
    Serial.begin(BAUD_RATE);

    /* Set up the CmdMessenger */
    attach_callbacks();

}

/* ---------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------- */

void loop() {

    /*
    ambient_val = analogRead(AMBIENT_PIN);
    hall_open_val = analogRead(HALL_OPEN_PIN);
    hall_closed_val = analogRead(HALL_CLOSED_PIN);
    
    Serial.print(ambient_val);
    Serial.print(",");
    Serial.print(hall_open_val);
    Serial.print(",");
    Serial.print(hall_closed_val);
    Serial.print("\n");
    */

    c.feedinSerialData();

    delay(SAMPLING_PERIOD);

}


