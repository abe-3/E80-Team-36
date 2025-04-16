#ifndef __WATER_TEMP_H__
#define __WATER_TEMP_H__

#include <Arduino.h>
#include "DataSource.h"

typedef struct {
    float watertemp = 0;
} water_temp_state_t;

class WaterTemp : public DataSource {
public:
  WaterTemp(void);

  void init(void);

  void updateState(int analog_reading);
  String printState(void);

  // from DataSource
  size_t writeDataBytes(unsigned char * buffer, size_t idx);

  int lastExecutionTime = -1;

  water_temp_state_t state;

private:
  const float tempCal_slope = -2.93;
  const float tempCal_intercept = 18.8;
};

#endif
