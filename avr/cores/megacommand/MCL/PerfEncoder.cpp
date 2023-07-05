#include "PerfEncoder.h"
#include "MCLMemory.h"
#include "MD.h"
#include "PerfData.h"

#define DIV_1_127 (1.00f / 127.0f)

static PerfScene PerfData::scenes[NUM_SCENES];

void PerfEncoder::send_param(uint8_t dest, uint8_t param, uint8_t val) {
  if (dest >= NUM_MD_TRACKS + 4) {
    uint8_t channel = dest - NUM_MD_TRACKS - 4;
    DEBUG_PRINTLN("send cc");
    DEBUG_PRINT(channel); DEBUG_PRINT(" "); DEBUG_PRINT(param); DEBUG_PRINT(" "); DEBUG_PRINTLN(val);
    MidiUart2.sendCC(channel, param, val);
  } else if (dest >= NUM_MD_TRACKS) {
    MD.sendFXParam(param, val, MD_FX_ECHO + dest - NUM_MD_TRACKS);
  } else {
    MD.setTrackParam(dest, param, val);
  }
}

void PerfEncoder::send_params(uint8_t cur_, PerfScene *s1, PerfScene *s2) {
  PerfMorph morph;

  morph.populate(s1, s2);

  for (uint8_t n = 0; n < morph.count; n++) {

    PerfFade *f = &morph.fades[n];
    DEBUG_PRINTLN("send para");
    DEBUG_PRINTLN(f->max);
    DEBUG_PRINTLN(f->min);

    uint8_t val = 0;
    if (f->max == 255 || f->min == 255) {
      continue;
    }
    int8_t range = f->max - f->min;
    int16_t q = cur_ * range;
    DEBUG_PRINTLN("range");
    DEBUG_PRINTLN(range);
    DEBUG_PRINTLN(cur);
    val = ((int16_t)q / (int16_t)127) + f->min;
    if (val > 127) {
      continue;
    }
    DEBUG_PRINTLN(val);
    send_param(f->dest - 1, f->param, val);
  }
}
void PerfEncoder::send() {
    PerfScene *s1 = active_scene_a == 255 ? nullptr :  &perf_data.scenes[active_scene_a];
    PerfScene *s2 = active_scene_b == 255 ? nullptr :  &perf_data.scenes[active_scene_b];
    send_params(cur, s1, s2);
}

int PerfEncoder::update(encoder_t *enc) {
  MCLEncoder::update(enc);
  // Update all params
  if (hasChanged()) {
    send();
  }
  return cur;
}

