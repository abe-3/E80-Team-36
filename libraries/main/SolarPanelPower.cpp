#include "SolarPanelPower.h"
#include "Printer.h"
#include <math.h>
extern Printer printer;

SolarPanelPower::SolarPanelPower(void)
  : DataSource("Solar Power", "float")
{}

void SolarPanelPower::init(void) {
  state.solarpower = 0;
}

void SolarPanelPower::updateState(int analog_reading) {
  float voltage = analog_reading * (3.3 / 1023.0);
  state.solarpower = pow(voltage, 2) / panelResistance;

  String msg = "Solar Power Voltage: " + String(voltage) + " V";
  printer.printMessage(msg, 20);
}

String SolarPanelPower::printState(void) {
  String currentState = "Solar Power: ";
  currentState += String(state.solarpower, 2);
  currentState += " W";
  return currentState;
}

size_t SolarPanelPower::writeDataBytes(unsigned char * buffer, size_t idx) {
  float * data_slot = (float *) &buffer[idx];
  data_slot[0] = state.solarpower;
  return idx + sizeof(float);
}
