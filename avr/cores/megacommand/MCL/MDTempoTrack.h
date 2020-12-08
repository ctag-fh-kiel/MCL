/* Justin Mammarella jmamma@gmail.com 2018 */

#pragma once

#include "AUXTrack.h"

class TempoData {
public:
  float tempo;
};

class MDTempoTrack : public AUXTrack, public TempoData {
public:
  MDTempoTrack() { 
    active = MDTEMPO_TRACK_TYPE; 
    static_assert(sizeof(MDTempoTrack) <= AUX_TRACK_LEN);
  }

  void init() { }

  void get_tempo();
  uint16_t calc_latency(uint8_t tracknumber);
  uint16_t send_tempo(bool send = true);
  void transition_send(uint8_t tracknumber, uint8_t slotnumber);
  bool store_in_grid(uint8_t column, uint16_t row,
                     SeqTrack *seq_track = nullptr, uint8_t merge = 0,
                     bool online = false);

  void load_immediate(uint8_t tracknumber, SeqTrack *seq_track);

  virtual uint16_t get_track_size() { return sizeof(MDTempoTrack); }
  virtual uint32_t get_region() { return BANK1_AUX_TRACKS_START; }

  virtual uint8_t get_model() { return MDTEMPO_TRACK_TYPE; }
  virtual uint8_t get_device_type() { return MDTEMPO_TRACK_TYPE; }

  virtual void *get_sound_data_ptr() { return &tempo; }
  virtual size_t get_sound_data_size() { return sizeof(float); }
};
