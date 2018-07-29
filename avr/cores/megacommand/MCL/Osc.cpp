/* Copyright Justin Mammarella jmamma@gmail.com 2018 */

#include "Math.h"
#include "Osc.h"

float Osc::get_sample(uint32_t sample_number, float freq, float phase) {
 return 0;
}
void Osc::set_sample_rate(float hz) { sample_rate = hz; }

float SineOsc::get_sample(uint32_t sample_number, float freq, float phase) {

  float sample_duration = (float)1 / (float)sample_rate;
  // float sample_duration = (float) 1 / (float) freq;
  return -1 * sin(2 * PI * freq * sample_number * sample_duration);
  // return sin(radians * sample_time);
}

float PulseOsc::get_sample(uint32_t sample_number, float freq, float phase) {

  float n_cycle = floor(sample_rate / freq);

  sample_number = sample_number + (n_cycle);
  float n =
      (float)sample_number - (float)floor(sample_number / n_cycle) * n_cycle;
  float n_edge = floor(n_cycle * width);
  if (n < n_edge) {
    return vmin;
  } else {
    return vmax;
  }
}

void PulseOsc::set_width(float width_) { width = width_; }
void PulseOsc::set_skew(float skew_) { skew = skew_; }

float SawOsc::get_sample(uint32_t sample_number, float freq, float phase) {

  float n_cycle = floor(sample_rate / freq);
  sample_number = sample_number + (n_cycle * .5);
  float n =
      (float)sample_number - (float)floor(sample_number / n_cycle) * n_cycle;

  float a = ((vmin - vmax) / n_cycle);
  float b = vmax;
  float y = a * n + b;
  return y;
}

float TriOsc::get_sample(uint32_t sample_number, float freq, float phase) {
  float n_cycle = floor(sample_rate / freq);
  sample_number = sample_number + (n_cycle * .75);

  float n = sample_number - floor(sample_number / n_cycle) * n_cycle;
  float n_edge = floor(n_cycle * width);

  if (n < n_edge) {
    float b = vmin;
    float a = (vmax - vmin) / n_edge;
    float y = a * n + b;
    return y;
  } else {
    float a = (vmin - vmax) / (float)(n_cycle - n_edge);
    float b = vmax - a * n_edge;
    float y = a * n + b;
    return y;
  }
}

float UsrOsc::get_sample(uint32_t sample_number, float freq, float phase,
                         uint8_t *usr_values) {

  float n_cycle = floor(sample_rate / freq);

  float n = sample_number - floor(sample_number / n_cycle) * n_cycle;

  float partition_size_n = n_cycle / (float)16;

  int start = floor(n / partition_size_n);
  float n_start = start * partition_size_n;
  int end;
  float n_end;
  if (start < 15) {
    end = start + 1;
    n_end = end * partition_size_n;

  } else {
    end = 0;
    n_end = n_cycle;
  }
  float v_start = -1 * (float)(usr_values[start] - 64) / (float)64;
  float v_end = -1 * (float)(usr_values[end] - 64) / (float)64;

  float m = ((v_end - v_start) / (n_end - n_start));
  float b = v_start - m * n_start;
  float y = m * n + b;

  return y;
}
