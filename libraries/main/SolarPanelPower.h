#ifndef __SOLAR_PANEL_POWER_H__
#define __SOLAR_PANEL_POWER_H__

#include <Arduino.h>
#include "DataSource.h"

typedef struct {
    float solarpower = 0;
} solar_power_state_t;

class SolarPanelPower : public DataSource {
public:
  SolarPanelPower(void);

  void init(void);

  void updateState(int analog_reading);
  String printState(void);

  // from DataSource
  size_t writeDataBytes(unsigned char * buffer, size_t idx);

  int lastExecutionTime = -1;

  solar_power_state_t state;

private:
  const float panelResistance = 55;
};

#endif
