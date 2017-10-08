#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CmdMessenger.h"

/* 
Reads the running average of ambient light. 

If the light drops below a defined cutoff the system:

    Reads a hall sensor that will be in contact with a magnet if the door is
    closed.  If this sensor does not detect a magnetic field:

        Activates a motor to close the door. The system then measures the state
        of the "closed" hall sensor.  If it detects a magnet, the system stops
        the motor.  If closing the door takes too long, the system will stop 
        the motor.

    The system turns off an external circuit by setting a defined output pin to
    LOW.  In the system this code was written to control, this external circuit
    has a power-hungry raspberry pi we want to turn off overnight.  

If the light is above the defined cutoff.
    
    Reads a hall sensor that will be in contact with a magnet if the door is 
    open.  If this sensor does nto detect a magnetic field:

        Activates a motor to open the door. The system then measures the state
        of the "open" hall sensor.  If it detects a magnet, the system stops
        the motor.  If opening the door takes too long, the system will stop 
        the motor.

    The system turns on an external circuit by setting a defined output pin to
    HIGH.  In the system this code was written to control, this external circuit
    has a power-hungry raspberry pi we want to turn off overnight but have on 
    during the day.  

The software is also configured to communicate information (on request) from an
external computer connected via the serial port running CmdMessenger.  The 
external computer can query the light and hall sensor levels, as well as open
and close the door.  These commands are transient, in that the main loop on the
arduino will overwrite their state if conditions are right.  For example, if 
the external computer says to open the door at night, the system will close it 
again the next time it reads the ambient light as low.
*/


/* ------------------------------------------------------------------------- *
 * Set values below to match "door" entry in farm json control file if this  *
 * device is interfaced with a raspberry pi running a thefarm.Door instance. *
 * ------------------------------------------------------------------------- */

/* Serial communications */
const char *DEVICE_ID = "COOP_HARDWARE";
const int BAUD_RATE = 9600;

/* ---------------------------------------------------------------------------
 * Set values below to indicate which pin is controlling which piece of      *
 * attached hardware.                                                        *
 * ------------------------------------------------------------------------- */

const int AMBIENT_PIN = 0;      // ambient light sensor.  should be analog
const int HALL_OPEN_PIN = 1;    // open hall sensor (any pin)
const int HALL_CLOSED_PIN = 2;  // closed hall sensor (any pin)
const int MOTOR_PIN_0 = 5;      // control pin for motor. should be pwm.
const int MOTOR_PIN_1 = 6;      // control pin for motor. should be pwm.

// control pin for turning external circuit on or off
const int EXTERNAL_CIRCUIT_PIN = 7; 

/* --------------------------------------------------------------------------*
 * Keep track of moving average for ambient light                            *
 * ------------------------------------------------------------------------- */

// AMBIENT_LIGHT_OBS should have the same length as RUNNING_AVERAGE_LENGTH
int RUNNING_AVERAGE_LENGTH = 5;
int AMBIENT_LIGHT_OBS[5] = {0,0,0,0,0}; 

/* ---------------------------------------------------------------------------
 * Control parameters for the motors themselves.                             *
 * ------------------------------------------------------------------------- */

// value that ambient light sensor is sensing night
const int AMBIENT_LIGHT_CUTOFF = 20;

// value below which the "open" hall sensor is reading closed
const int HALL_OPEN_CUTOFF = 20;

// value below which the "closed" hall sensor is reading closed
const int HALL_CLOSED_CUTOFF = 20;

// maximum amount of time to move the door (milliseconds)
const int DOOR_MOVE_TIMEOUT = 3000;  

// Duty cycle for PWM of door motor. controls motorfrom nothing (0) to full
// speed (255) 
const int DOOR_PWM_DUTY = 100; 

// How often to delay between checking motor movement (milliseconds)
const int DOOR_SAMPLING_PERIOD = 50;

// How often to check the sensor and read serial comms (milliseconds)
const int SAMPLING_PERIOD = 500;

/* ---------------------------------------------------------------------------
 * Stuff below here shouldn't need to be changed                             *
 * ------------------------------------------------------------------------- */

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
    door_open_return,
    door_close_return,
    communication_error,
};

/* CmdMessenger callbacks */

void on_unknown_command(void){
    
    /* If there is an error */

    c.sendCmd(communication_error,"Unknown command.");
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

    /* Process command to open the door */    

    int status;

    status = open_the_door();

    c.sendCmdStart(door_open_return);
    c.sendCmdBinArg(status);
    c.sendCmdEnd();
        
}

void on_close_door(void){

    /* Process command to close the door */   
 
    int status;

    status = close_the_door();

    c.sendCmdStart(door_close_return);
    c.sendCmdBinArg(status);
    c.sendCmdEnd();
        
}

/* Attach callback methods */
void attach_callbacks(void) { 
  
    c.attach(on_unknown_command);
    c.attach(who_are_you,on_who_are_you);
    c.attach(query,on_query);
    c.attach(open_door,on_open_door);
    c.attach(close_door,on_close_door);

}

/* Hardware control functions */

int open_the_door(){

    /* Open door, returning 0 if successful, some other number if fail. */

    unsigned long max_end_time;

    /* Already open! */
    if (analogRead(HALL_OPEN_PIN) < HALL_OPEN_CUTOFF) {
        return 0;
    }    
    
    max_end_time = millis() + DOOR_MOVE_TIMEOUT;
  
    /* Set motor to open door */ 
    digitalWrite(MOTOR_PIN_0,LOW);
    analogWrite(MOTOR_PIN_1,DOOR_PWM_DUTY);

    while (millis() < max_end_time){
        
        /* Stop door */
        if (analogRead(HALL_OPEN_PIN) < HALL_OPEN_CUTOFF) { 
            digitalWrite(MOTOR_PIN_1,LOW);
            return 0;
        }
        delay(DOOR_SAMPLING_PERIOD); 
    }

    /* Stop door */
    digitalWrite(MOTOR_PIN_1,LOW);
    return 1;

}

int close_the_door(){

    /* Close door, returning 0 if successful, some other number if fail. */

    unsigned long max_end_time;

    /* Already closed! */
    if (analogRead(HALL_CLOSED_PIN) < HALL_CLOSED_CUTOFF) {
        return 0;
    }    
    
    max_end_time = millis() + DOOR_MOVE_TIMEOUT;
  
    /* Set motor to close door */ 
    analogWrite(MOTOR_PIN_0,DOOR_PWM_DUTY);
    digitalWrite(MOTOR_PIN_1,LOW);

    while (millis() < max_end_time){
        if (analogRead(HALL_CLOSED_PIN) < HALL_CLOSED_CUTOFF) { 
            /* stop door */
            digitalWrite(MOTOR_PIN_0,LOW);
            return 0; 
        }
        delay(DOOR_SAMPLING_PERIOD); 
    }

    /* stop door */
    digitalWrite(MOTOR_PIN_0,LOW);
    return 1;

}

void turn_on_external_circuit(void){
    /* Turn on the external circuit */
    digitalWrite(EXTERNAL_CIRCUIT_PIN,HIGH);
}

void turn_off_external_circuit(void){
    /* Turn off the external circuit */
    digitalWrite(EXTERNAL_CIRCUIT_PIN,LOW);
}

void light_check(){

    /* Check to see if the door should be opened or closed and the external 
     * circuit on or off based on the running average of the ambient light
     * sensor. */

    int i;
    int ambient_light, hall_open, hall_closed;
    float ambient_average;
    
    /* Update ambient light obervations. The last observation is placed
     * in the last position */
    ambient_average = 0;
    for (i = 0; i < (RUNNING_AVERAGE_LENGTH-1); i++){
        AMBIENT_LIGHT_OBS[i] = AMBIENT_LIGHT_OBS[i+1];
        ambient_average = ambient_average + AMBIENT_LIGHT_OBS[i];
    }
    AMBIENT_LIGHT_OBS[RUNNING_AVERAGE_LENGTH-1] = analogRead(AMBIENT_PIN);
    ambient_average = ambient_average + AMBIENT_LIGHT_OBS[RUNNING_AVERAGE_LENGTH-1];
    ambient_average = ambient_average/RUNNING_AVERAGE_LENGTH;

    /* See if sensors pass cutoffs */
    ambient_light = ambient_average > AMBIENT_LIGHT_CUTOFF;
    hall_open = analogRead(HALL_OPEN_PIN) < HALL_OPEN_CUTOFF;
    hall_closed = analogRead(HALL_CLOSED_PIN) < HALL_CLOSED_CUTOFF;

    /* Light and closed -> open */
    if (ambient_light){
        if (hall_closed){
            open_the_door();
        }

    /* Dark and open -> closed */
    } else {
        if (hall_open){
            close_the_door();
        }
    }
      
    /* Turn the external circuit on or off depending on ambient light */ 
    if (ambient_light){
        turn_on_external_circuit();
    } else { 
        turn_off_external_circuit();
    }
 
}


/* ---------------------------------------------------------------------------
 * Set up (on boot or reset). 
 * -------------------------------------------------------------------------- */

void setup() {

    int i;
    
    /* Initialize serial communication at BAUD_RATE bits per second: */
    Serial.begin(BAUD_RATE);

    /* Set up the CmdMessenger */
    attach_callbacks();

    /* Set up the motor (stopped) */
    digitalWrite(MOTOR_PIN_0,LOW);
    digitalWrite(MOTOR_PIN_1,LOW);
    pinMode(MOTOR_PIN_0,OUTPUT);
    pinMode(MOTOR_PIN_1,OUTPUT);

    /* Set up the external circuit (on) */
    digitalWrite(EXTERNAL_CIRCUIT_PIN,HIGH);
    pinMode(EXTERNAL_CIRCUIT_PIN,OUTPUT);

    /* Set up the ambient light sensor */
    /* Populate moving average for ambient light with current ambient light */
    for (i = 0; i < RUNNING_AVERAGE_LENGTH; i++){
        AMBIENT_LIGHT_OBS[i] = analogRead(AMBIENT_PIN);
    }
        
}

/* ---------------------------------------------------------------------------
 * Main loop
 * -------------------------------------------------------------------------- */

void loop() {

    /* Serial interface */
    c.feedinSerialData();

    /* Check the light level and respond appropriately*/
    light_check();

    delay(SAMPLING_PERIOD);

}


