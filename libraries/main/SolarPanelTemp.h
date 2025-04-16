#ifndef __SOLAR_PANEL_TEMP_H__
#define __SOLAR_PANEL_TEMP_H__

#include <Arduino.h>
#include "DataSource.h"

typedef struct {
    float solartemp = 0;
} solar_temp_state_t;

class SolarPanelTemp : public DataSource {
public:
  SolarPanelTemp(void);

  void init(void);

  void updateState(int analog_reading);
  String printState(void);

  // from DataSource
  size_t writeDataBytes(unsigned char * buffer, size_t idx);

  int lastExecutionTime = -1;

  solar_temp_state_t state;

private:
  const float tempCal_slope = -3.12;
  const float tempCal_intercept = 18.9;
};

#endif
