/* Copyright 2018, Justin Mammarella jmamma@gmail.com */

#ifndef PERFENCODER_H__
#define PERFENCODER_H__

#include "MCLEncoder.h"
#include "PerfData.h"

#define PERF_NAME_LENGTH 9

class PerfEncoder : public MCLEncoder {
    /**
       \addtogroup gui_rangeencoder_class
       @{
     **/

  public:
  char name[PERF_NAME_LENGTH] = "CONTROL";

  PerfData perf_data;

  uint8_t active_scene_a = 0;
  uint8_t active_scene_b = 1;

  void init() { perf_data.init(); }
  void send();
  void send_param(uint8_t dest, uint8_t param, uint8_t val);
  void send_params(uint8_t cur_, PerfScene *s1, PerfScene *s2);
  void quick_scene_populate();
  /**
       Create a new range-limited encoder with max and min value, short
       name, initial value, and handling function. The initRangeEncoder
       will be called with the constructor arguments.
     **/
    PerfEncoder(int _max = 127, int _min = 0, int res = 1) : MCLEncoder(_max , _min, res) {
    }

   virtual int update(encoder_t *enc);

};

#endif /* PerfENCODER_H__ */
