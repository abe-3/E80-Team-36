#include "WaterTemp.h"
#include "Printer.h"
extern Printer printer;

WaterTemp::WaterTemp(void)
  : DataSource("Water Temperature", "float")
{}

void WaterTemp::init(void) {
  state.watertemp = 0;
}

void WaterTemp::updateState(int analog_reading) {
  float voltage = analog_reading * (3.3 / 1023.0);
  state.watertemp = tempCal_slope * voltage + tempCal_intercept;

  String msg = "Water Temp Voltage: " + String(voltage) + " V";
  printer.printMessage(msg, 20);
}

String WaterTemp::printState(void) {
  String currentState = "Water Temp: ";
  currentState += String(state.watertemp, 2);
  currentState += " °C";
  return currentState;
}

size_t WaterTemp::writeDataBytes(unsigned char * buffer, size_t idx) {
  float * data_slot = (float *) &buffer[idx];
  data_slot[0] = state.watertemp;
  return idx + sizeof(float);
}
