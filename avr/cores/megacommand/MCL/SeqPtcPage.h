/* Justin Mammarella jmamma@gmail.com 2018 */

#ifndef SEQPTCPAGE_H__
#define SEQPTCPAGE_H__

#include "MidiActivePeering.h"
#include "Scales.h"
#include "SeqPage.h"
#include "SeqPages.h"

#define MAX_POLY_NOTES 16

extern scale_t *scales[24];

void ptc_pattern_len_handler(EncoderParent *enc);

class SeqPtcMidiEvents : public MidiCallback {
public:
  bool state;

  void setup_callbacks();
  void remove_callbacks();

  void onNoteOnCallback_Midi2(uint8_t *msg);
  void onNoteOffCallback_Midi2(uint8_t *msg);
  void onControlChangeCallback_Midi(uint8_t *msg);
  void onControlChangeCallback_Midi2(uint8_t *msg);
};

class SeqPtcPage : public SeqPage, public ClockCallback {

public:
  bool re_init = false;
  uint8_t focus_track = 255;
  uint8_t key = 0;
  uint8_t poly_count = 0;
  uint8_t poly_max = 0;
  int8_t poly_notes[MAX_POLY_NOTES];
  uint64_t note_mask = 0;
  bool cc_link_enable;

  SeqPtcMidiEvents midi_events;
  SeqPtcPage(Encoder *e1 = NULL, Encoder *e2 = NULL, Encoder *e3 = NULL,
             Encoder *e4 = NULL)
      : SeqPage(e1, e2, e3, e4) {}
  uint8_t calc_poly_count();
  uint8_t seq_ext_pitch(uint8_t note_num, MidiDevice* device = &generic_midi_device);
  uint8_t process_ext_pitch(uint8_t note_num, bool note_type, MidiDevice* device = &generic_midi_device);
  uint8_t get_machine_pitch(uint8_t track, uint8_t note_num);
  uint8_t get_next_voice(uint8_t pitch);
  uint8_t calc_scale_note(uint8_t note_num);

  void set_last_ext_track(uint8_t channel);
  void trig_md(uint8_t note_num, MidiUartParent *uart = nullptr);
  void trig_md_fromext(uint8_t note_num);
  void clear_trig_fromext(uint8_t note_num);

  void config_encoders();
  void init_poly();

  void render_arp();

  void recalc_notemask();

  virtual bool handleEvent(gui_event_t *event);
  virtual void display();
  virtual void setup();
  virtual void cleanup();
  virtual void loop();
  virtual void init();
  virtual void config();
};

#endif /* SEQPTCPAGE_H__ */
