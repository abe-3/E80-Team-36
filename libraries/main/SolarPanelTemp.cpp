#include "SolarPanelTemp.h"
#include "Printer.h"
extern Printer printer;

SolarPanelTemp::SolarPanelTemp(void)
  : DataSource("Solar Panel Temperature", "float")
{}

void SolarPanelTemp::init(void) {
  state.solartemp = 0;
}

void SolarPanelTemp::updateState(int analog_reading) {
  float voltage = analog_reading * (3.3 / 1023.0);
  state.solartemp = tempCal_slope * voltage + tempCal_intercept;

  String msg = "Solar Panel Temp Voltage: " + String(voltage) + " V";
  printer.printMessage(msg, 20);
}

String SolarPanelTemp::printState(void) {
  String currentState = "Solar Temp: ";
  currentState += String(state.solartemp, 2);
  currentState += " °C";
  return currentState;
}

size_t SolarPanelTemp::writeDataBytes(unsigned char * buffer, size_t idx) {
  float * data_slot = (float *) &buffer[idx];
  data_slot[0] = state.solartemp;
  return idx + sizeof(float);
}
