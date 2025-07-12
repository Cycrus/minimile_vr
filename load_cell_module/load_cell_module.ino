/**
 * The software running on the Arduino controlling the load cell.
 * Produces a 10 Hz output signal between 0 and 1 indicating how
 * string the load cell was pulled. This signal is interpreted
 * as different speeds of moving forward.
 * 
 * Compilation requires the HX711 Arduino library by Rob Tillaart.
 * The CLK pin of the HX711 is wired to digital 3 and the data output
 * is wired to digital 4 of the Arduino.
 */

#include "HX711.h"

uint8_t data_pin  = 4;
uint8_t clock_pin = 3;
HX711 treadmill;

float raw_measurement(float value)
{
  return value;
}

float ema_measurement(float value, float alpha)
{
  static float result = NAN;
  if(isnan(result))
  {
    result = value;
  }
  else
  {
    float prev_result = result;
    result = alpha * value + (1 - alpha) * prev_result;
  }
  return result;
}

float normalize_measurement(float value, float min_value, float max_value)
{
  float result = (value - min_value) / (max_value - min_value);
  if(result > 1.0)
    result = 1.0;
  else if(result < 0.0)
    result = 0.0;
  return result;
}


void setup()
{
  treadmill.begin(data_pin, clock_pin);
  Serial.begin(9600);
  while(!Serial) {}
  treadmill.set_scale();
  treadmill.tare();
}

void loop()
{
  float raw_value = treadmill.get_units();
  /*
  This is an active decision against smoothing. A useful EMA smoothing would lead
  to too much latency for a useful game input device.
  */
  // float smoothed_value = ema_measurement(raw_value, 0.07);
  float normalized_value = normalize_measurement(raw_value, 50000.0, 1000000.0);
  Serial.println(normalized_value);
}
