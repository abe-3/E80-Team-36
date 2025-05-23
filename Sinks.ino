/********
E80 Lab 7 Dive Activity Code
Authors:
    Omar Aleman (oaleman@g.hmc.edu) '21 (contributed 2019)
    Wilson Ives (wives@g.hmc.edu) '20 (contributed in 2018)
    Christopher McElroy (cmcelroy@g.hmc.edu) '19 (contributed in 2017)
    Josephine Wong (jowong@hmc.edu) '18 (contributed in 2016)
    Apoorva Sharma (asharma@hmc.edu) '17 (contributed in 2016)
*/

#include <Arduino.h>
#include <Wire.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include <Pinouts.h>
#include <TimingOffsets.h>
#include <SensorGPS.h>
#include <SensorIMU.h>
#include <XYStateEstimator.h>
#include <ZStateEstimator.h>
#include <ADCSampler.h>
#include <ErrorFlagSampler.h>
#include <ButtonSampler.h> // A template of a data source library
#include <MotorDriver.h>
#include <Logger.h>
#include <Printer.h>
#include <DepthControl.h>
#define UartSerial Serial1
#include <GPSLockLED.h>
#include <SolarPanelTemp.h> // Solar Panel Temperature
#include <WaterTemp.h> // Water Temperature
#include <SolarPanelPower.h> // Solar Panel Power Output

/////////////////////////* Global Variables *////////////////////////

MotorDriver motor_driver;
XYStateEstimator xy_state_estimator;
ZStateEstimator z_state_estimator;
DepthControl depth_control;
SensorGPS gps;
Adafruit_GPS GPS(&UartSerial);
ADCSampler adc;
ErrorFlagSampler ef;
ButtonSampler button_sampler;
SensorIMU imu;
Logger logger;
Printer printer;
GPSLockLED led;
SolarPanelTemp solar_panel_temp;
WaterTemp water_temp;
SolarPanelPower solar_power;

// loop start recorder
int loopStartTime;
int currentTime;
volatile bool EF_States[NUM_FLAGS] = {1,1,1};
bool logAtWaypoint = false;

////////////////////////* Setup *////////////////////////////////

void setup() {

  
  logger.include(&imu);
  logger.include(&gps);
  logger.include(&xy_state_estimator);
  logger.include(&z_state_estimator);
  logger.include(&solar_panel_temp); //
  logger.include(&water_temp); //
  logger.include(&solar_power); //
  logger.include(&depth_control);
  logger.include(&motor_driver);
  logger.include(&adc);
  logger.include(&ef);
  logger.include(&button_sampler);
  logger.init();

  printer.init();
  ef.init();
  button_sampler.init();
  imu.init();
  UartSerial.begin(9600);
  gps.init(&GPS);
  motor_driver.init();
  led.init();

  int diveDelay = 5000; // how long robot will stay at depth waypoint before continuing (ms)

  const int num_depth_waypoints = 2;
  double depth_waypoints [] = { 0.5, 1};  // listed as z0,z1,... etc.
  depth_control.init(num_depth_waypoints, depth_waypoints, diveDelay);
  
  xy_state_estimator.init();
  z_state_estimator.init();
  water_temp.init();
  solar_panel_temp.init();
  solar_power.init();

  printer.printMessage("Starting main loop",10);
  loopStartTime = millis();
  printer.lastExecutionTime            = loopStartTime - LOOP_PERIOD + PRINTER_LOOP_OFFSET ;
  imu.lastExecutionTime                = loopStartTime - LOOP_PERIOD + IMU_LOOP_OFFSET;
  adc.lastExecutionTime                = loopStartTime - LOOP_PERIOD + ADC_LOOP_OFFSET;
  ef.lastExecutionTime                 = loopStartTime - LOOP_PERIOD + ERROR_FLAG_LOOP_OFFSET;
  button_sampler.lastExecutionTime     = loopStartTime - LOOP_PERIOD + BUTTON_LOOP_OFFSET;
  xy_state_estimator.lastExecutionTime = loopStartTime - LOOP_PERIOD + XY_STATE_ESTIMATOR_LOOP_OFFSET;
  z_state_estimator.lastExecutionTime  = loopStartTime - LOOP_PERIOD + Z_STATE_ESTIMATOR_LOOP_OFFSET;
  depth_control.lastExecutionTime      = loopStartTime - LOOP_PERIOD + DEPTH_CONTROL_LOOP_OFFSET;
  logger.lastExecutionTime             = loopStartTime - LOOP_PERIOD + LOGGER_LOOP_OFFSET;
  solar_panel_temp.lastExecutionTime   = loopStartTime - LOOP_PERIOD + SOLAR_PANEL_TEMP_LOOP_OFFSET;
  water_temp.lastExecutionTime         = loopStartTime - LOOP_PERIOD + WATER_TEMP_LOOP_OFFSET;
  solar_power.lastExecutionTime        = loopStartTime - LOOP_PERIOD + SOLAR_POWER_LOOP_OFFSET;

}

//////////////////////////////* Loop */////////////////////////

void loop() {
  currentTime=millis();
    
  if ( currentTime-printer.lastExecutionTime > LOOP_PERIOD ) {
    printer.lastExecutionTime = currentTime;
    printer.printValue(0,adc.printSample());
    printer.printValue(1,ef.printStates());
    printer.printValue(2,logger.printState());
    printer.printValue(3,gps.printState());   
    printer.printValue(4,xy_state_estimator.printState());  
    printer.printValue(5,z_state_estimator.printState());  
    printer.printValue(6,depth_control.printWaypointUpdate());
    printer.printValue(7,depth_control.printString());
    printer.printValue(8,motor_driver.printState());
    printer.printValue(9,imu.printRollPitchHeading());        
    printer.printValue(10,imu.printAccels());
    printer.printValue(11,solar_panel_temp.printState());
    printer.printValue(12,water_temp.printState());
    printer.printValue(13,solar_power.printState());
    printer.printToSerial();  // To stop printing, just comment this line out
  }

  /* ROBOT CONTROL Finite State Machine */

if (depth_control.diveState) {
  depth_control.complete = false;

  if (!depth_control.atDepth) {
    depth_control.dive(&z_state_estimator.state, currentTime);
    motor_driver.drive(0, 0, 0);

    logger.log();

  } else {
    depth_control.diveState = false;
    depth_control.surfaceState = true;
  }
} 
else if (depth_control.surfaceState) {
  if (!depth_control.complete) {
    depth_control.surface(&z_state_estimator.state);
    motor_driver.drive(depth_control.uV, depth_control.uV, 0);
  } else {
    depth_control.surface(&z_state_estimator.state);
    motor_driver.drive(depth_control.uV, depth_control.uV, 0);
  }
}

// Override
if (currentTime > 120000) {
  depth_control.surface(&z_state_estimator.state);
  motor_driver.drive(depth_control.uV, depth_control.uV, 0);
}


// Override
if (currentTime > 120000) {
  depth_control.surface(&z_state_estimator.state);
  motor_driver.drive(depth_control.uV, depth_control.uV, 0);
}


  
  if ( currentTime-adc.lastExecutionTime > LOOP_PERIOD ) {
    adc.lastExecutionTime = currentTime;
    adc.updateSample(); 
  }

  if ( currentTime-ef.lastExecutionTime > LOOP_PERIOD ) {
    ef.lastExecutionTime = currentTime;
    attachInterrupt(digitalPinToInterrupt(ERROR_FLAG_A), EFA_Detected, LOW);
    attachInterrupt(digitalPinToInterrupt(ERROR_FLAG_B), EFB_Detected, LOW);
    attachInterrupt(digitalPinToInterrupt(ERROR_FLAG_C), EFC_Detected, LOW);
    delay(5);
    detachInterrupt(digitalPinToInterrupt(ERROR_FLAG_A));
    detachInterrupt(digitalPinToInterrupt(ERROR_FLAG_B));
    detachInterrupt(digitalPinToInterrupt(ERROR_FLAG_C));
    ef.updateStates(EF_States[0],EF_States[1],EF_States[2]);
    EF_States[0] = 1;
    EF_States[1] = 1;
    EF_States[2] = 1;
  }

  if ( currentTime-button_sampler.lastExecutionTime > LOOP_PERIOD ) {
    button_sampler.lastExecutionTime = currentTime;
    button_sampler.updateState();
  }

  if ( currentTime-imu.lastExecutionTime > LOOP_PERIOD ) {
    imu.lastExecutionTime = currentTime;
    imu.read();     // blocking I2C calls
  }
 
  gps.read(&GPS); // blocking UART calls, need to check for UART data every cycle

  if ( currentTime-xy_state_estimator.lastExecutionTime > LOOP_PERIOD ) {
    xy_state_estimator.lastExecutionTime = currentTime;
    xy_state_estimator.updateState(&imu.state, &gps.state);
  }

  if ( currentTime-z_state_estimator.lastExecutionTime > LOOP_PERIOD ) {
    z_state_estimator.lastExecutionTime = currentTime;
    z_state_estimator.updateState(analogRead(PRESSURE_PIN));
  }

  if (currentTime-solar_panel_temp.lastExecutionTime > LOOP_PERIOD) {
    solar_panel_temp.lastExecutionTime = currentTime;
    solar_panel_temp.updateState(analogRead(SOLAR_PANEL_TEMP_PIN));
  }

  if (currentTime-water_temp.lastExecutionTime > LOOP_PERIOD) {
    water_temp.lastExecutionTime = currentTime;
    water_temp.updateState(analogRead(WATER_TEMP_PIN));
  }

  if (currentTime-solar_power.lastExecutionTime > LOOP_PERIOD) {
    solar_power.lastExecutionTime = currentTime;
    solar_power.updateState(analogRead(SOLAR_PANEL_VOLTAGE_PIN));
  }
  
  if ( currentTime-led.lastExecutionTime > LOOP_PERIOD ) {
    led.lastExecutionTime = currentTime;
    led.flashLED(&gps.state);
  }

//   if ( currentTime- logger.lastExecutionTime > LOOP_PERIOD && logger.keepLogging ) {
//     logger.lastExecutionTime = currentTime;
//     logger.log();
//   }
// }

void EFA_Detected(void){
  EF_States[0] = 0;
}

void EFB_Detected(void){
  EF_States[1] = 0;
}

void EFC_Detected(void){
  EF_States[2] = 0;
}
