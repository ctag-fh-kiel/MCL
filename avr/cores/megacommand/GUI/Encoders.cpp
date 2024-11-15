/* Copyright (c) 2009 - http://ruinwesen.com/ */

#include "Encoders.h"

#include "MidiTools.h"
#include "Midi.h"

#include "GUI.h"

/* handlers */

/**
 * \addtogroup GUI
 *
 * @{
 *
 * \addtogroup gui_encoders Encoder classes
 *
 * @{
 *
 * \file
 * Encoder classes
 **/
class Encoder;

EncoderParent::EncoderParent(encoder_handle_t _handler) {
  old = 0;
  cur = 0;
  redisplay = false;
  handler = _handler;
}

void EncoderParent::checkHandle() {
  if (cur != old) {
    if (handler != NULL)
      handler(this);
  }

  old = cur;
}

void EncoderParent::setValue(int value, bool handle) {
  if (handle) {
    cur = value;
    checkHandle();
  } else {
    old = cur = value;
  }
  redisplay = true;
}

void EncoderParent::displayAt(int i) {
  redisplay = false;
}

bool EncoderParent::hasChanged() { return old != cur; }

void EncoderParent::clear() {
  old = 0;
  cur = 0;
}

Encoder::Encoder(const char *_name, encoder_handle_t _handler)
    : EncoderParent(_handler) {
  fastmode = true;
}

int Encoder::update_rotations(encoder_t *enc) {
   uint8_t amount = abs(enc->normal);
  int inc = 0;

  while (amount > 0) {
    if (enc->normal > 0) {
      rot_counter_up += 1;
      if (rot_counter_up > rot_res) {
        rot_counter_up = 0;
        inc += 1;
      }
      rot_counter_down = 0;
    }
    if (enc->normal < 0) {
      rot_counter_down += 1;
      if (rot_counter_down > rot_res) {
        rot_counter_down = 0;
        inc -= 1;
      }

      rot_counter_up = 0;
    }
    amount--;
  }
#ifdef INVERT_ENCODER
  inc = -inc;
#endif
 return inc;
}

int Encoder::update(encoder_t *enc) {
  int inc = update_rotations(enc);

#ifdef INVERT_ENCODER
  inc = inc - (fastmode ? 4 * enc->button : enc->button);
#else
  inc = inc + (fastmode ? 4 * enc->button : enc->button);
#endif

  cur += inc;

  return cur;
}

