/* Copyright 2018, Justin Mammarella jmamma@gmail.com */

#ifndef MCLSEQUENCER_H__
#define MCLSEQUENCER_H__
#include "MCL.h"
#include "midiclock.h"
#include "SeqPages.h"
#define NUM_PARAM_PAGES 2;

class MCLSequencer : public ClockCallback {
public:


  uint8_t last_ext_track;

  void setup();
  void onMidiStopCallback();
  void sequencer();
  void trig_conditional();
  void parameter_locks(uint8_t i, uint8_t step_count);
  void seq_buffer_notesoff(uint8_t track);
  void seq_note_on(uint8_t track, uint8_t note);
  void seq_note_off(uint8_t track, uint8_t note);
  void noteon_conditional(uint8_t condition, uint8_t track, uint8_t note);
  void set_track_param(uint8_t track, uint8_t param, uint8_t value);
};

extern MCLSequencer mcl_seq;
#endif /* MCLSEQUENCER_H__ */
