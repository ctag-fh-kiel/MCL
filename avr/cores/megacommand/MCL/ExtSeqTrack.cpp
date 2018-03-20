#include "ExtSeqTrack.h"

void SeqExtTrack::seq() {

    uint8_t step_count =
        ((MidiClock.div32th_counter / resolution) -
         (pattern_start_clock32th / resolution)) -
        (length *
         ((MidiClock.div32th_counter / resolution -
           (pattern_start_clock32th / resolution)) /
          (length)));

    int8_t utiming = timing[step_count];         // upper
    uint8_t condition = conditional[step_count]; // lower

    int8_t timing_counter = MidiClock.mod12_counter;

    if ((resolution == 1)) {
      timing_counter =
          MidiClock.mod12_counter - (6 * (MidiClock.mod12_counter / 6));
    }
    //        if (step_count == (length * (2 /
    //        resolution) - 1)) {

    if (step_count == length) {
      uint8_t next_step = 0;
    } else {
      uint8_t next_step = step_count + 1;
    }

    int8_t utiming_next = timing[next_step];         // upper
    uint8_t condition_next = conditional[next_step]; // lower
    if (!in_sysex2) {

      if ((utiming >= (6 * resolution)) &&
          (utiming - (6 * resolution) == (int8_t)timing_counter)) {

        for (uint8_t c = 0; c < 4; c++) {
          if (notes[c][step_count] < 0) {
            note_off(abs(notes[c][step_count]) - 1);
          }

          else if (notes[c][step_count] > 0) {
            if ((mutes == SEQ_MUTE_OFF)) {
              noteon_conditional(condition,
                                 abs(notes[c][step_count]) - 1);
            }
          }
        }
      }

      if ((utiming_next < (6 * resolution)) &&
          ((utiming_next) == (int8_t)timing_counter)) {

        for (uint8_t c = 0; c < 4; c++) {

          if (notes[c][step_count + 1] < 0) {
            note_off(abs(notes[c][next_step]) - 1);
          } else if (notes[c][step_count + 1] > 0) {
            if (mutes == SEQ_MUTE_OFF) {
              noteon_conditional(condition,
                                 abs(notes[c][next_step]) - 1);
            }
          }
        }
      }
    }
  }
}

void ExtSeqTrack::buffer_notesoff() {
  for (uint8_t c = 0; c < SEQ_NOTEBUF_SIZE; c++) {
    if (notebufferuffer[c] > 0) {
      uart->sendNoteOff(channel, notebuffer[c], 0);
      notebuffer[c] = 0;
    }
  }
}
void ExtSeqTrack::note_on(uint8_t note) {
  uart->sendNoteOn(channel, note, 100);

  for (uint8_t c = 0; c < SEQ_NOTEBUF_SIZE; c++) {
    if (notebuffer[c] == 0) {
      notebuffer[c] = note;
      return;
    }
  }
}
void ExtSeqTrack::note_off(uint8_t note) {
  uart->sendNoteOff(channel, note, 0);

  for (uint8_t i = 0; i < SEQ_NOTEBUF_SIZE; i++) {
    if (notebuffer[i] == note) {
      notebuffer[i] = 0;
      return;
    }
  }
}

void SeqExtTrack::noteon_conditional(uint8_t condition, uint8_t note) {

  if ((condition == 0)) {
    note_on(note);
  }

  else if (condition <= 8) {
    if (resolution[track] == 2) {
      if (((MidiClock.div16th_counter - pattern_start_clock32th / 2 + length) /
           length) %
              ((condition)) ==
          0) {
        note_on(note);
      }
    } else {
      if (((MidiClock.div32th_counter - pattern_start_clock32th + length) /
           length) %
              ((condition)) ==
          0) {
        note_on(note);
      }
    }
  } else if ((condition == 9) && (random(100) <= 10)) {
    note_on(note);
  } else if ((condition == 10) && (random(100) <= 25)) {
    note_on(note);
  } else if ((condition == 11) && (random(100) <= 50)) {
    note_on(note);
  } else if ((condition == 12) && (random(100) <= 75)) {
    note_on(note);
  } else if ((condition == 13) && (random(100) <= 90)) {
    note_on(note);
  }
}

void SeqExtTrack::set_step(uint8_t step, uint8_t note_num, uint8_t velocity) {
  uint8_t match = 255;
  // Look for matching note already on this step
  // If it's a note off, then disable the note
  // If it's a note on, set the note note-off.
  for (uint8_t c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step] == -(1 * (note_num + 1))) {
      notes[c][step] = 0;
      seq_notesoff(track);
      match = c;
    }
    if (notes[c][step] == (1 * (note_num + 1))) {
      notes[c][step] = (-1 * (note_num + 1));
      match = c;
    }
  }
  // No matches are found, we count number of on and off to determine next note
  // type.
  for (uint8_t c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step] == 0) {
      match = c;
      int8_t ons_and_offs = 0;
      // Check to see if we have same number of note offs as note ons.
      // If there are more note ons for given note, the next note entered should
      // be a note off.
      for (uint8_t a = 0; a < mcl_seq.ext_tracks[track].length; a++) {
        for (uint8_t b = 0; b < 4; b++) {
          if (notes[b][a] == -(1 * (note_num + 1))) {
            ons_and_offs -= 1;
          }
          if (notes[b][a] == (1 * (note_num + 1))) {
            ons_and_offs += 1;
          }
        }
      }
      if (ons_and_offs <= 0) {
        notes[c][step] = (note_num + 1);
      } else {
        notes[c][step] = -1 * (note_num + 1);
      }
    }
  }
}
void SeqExtTrack::record_ext_track_noteoff( uint8_t note_num,
                                      uint8_t velocity) {
  uint8_t step_count =
      ((MidiClock.div32th_counter / resolution) -
       (pattern_start_clock32th / resolution)) -
      (mcl_seq.ext_tracks[track].length * ((MidiClock.div32th_counter / resolution -
                                    (pattern_start_clock32th / resolution)) /
                                   (mcl_seq.ext_tracks[track].length)));

  uint8_t utiming =
      6 + MidiClock.mod12_counter - (6 * (MidiClock.mod12_counter / 6));

  if (resolution > 1) {
    utiming = (MidiClock.mod12_counter + 12);
  }

  uint8_t condition = 0;
  uint8_t match = 255;
  uint8_t c = 0;

  for (c = 0; c < 4 && match == 255; c++) {
    if (abs(notes[c][step_count]) == note_num + 1) {
      match = c;

      if (notes[c][step_count] > 0) {
        step_count = step_count + 1;
        if (step_count > mcl_seq.ext_tracks[track].length) {
          step_count = 0;
        }
        utiming = MidiClock.mod12_counter - (6 * (MidiClock.mod12_counter / 6));
        // timing = 0;
      }
    }
  }
  for (c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step_count] == 0) {
      match = c;
    }
  }

  if (match != 255) {
    notes[match][step_count] = -1 * (note_num + 1);
    // SET_BIT64(ExtLockMasks[track], step_count);
  }
  conditional[step_count] = condition;
  timing[step_count] = utiming;
}

void SeqExtTrack::record_ext_track_noteon( uint8_t note_num,
                                     uint8_t velocity) {
  uint8_t step_count =
      ((MidiClock.div32th_counter / resolution) -
       (pattern_start_clock32th / resolution)) -
      (mcl_seq.ext_tracks[track].length * ((MidiClock.div32th_counter / resolution -
                                    (pattern_start_clock32th / resolution)) /
                                   (mcl_seq.ext_tracks[track].length)));

  uint8_t utiming =
      6 + MidiClock.mod12_counter - (6 * (MidiClock.mod12_counter / 6));

  if (resolution > 1) {
    utiming = (MidiClock.mod12_counter + 12);
  }
  uint8_t condition = 0;

  uint8_t match = 255;
  uint8_t c = 0;
  // Let's try and find an existing param

  for (c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step_count] == note_num + 1) {
      match = c;
    }
  }
  for (c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step_count] == 0) {
      match = c;
    }
  }
  if (match != 255) {
    // We dont want to record steps if note off mask is set
    //  if (notes[match][step_count] < 0) { return; }
    notes[match][step_count] = note_num + 1;
    // SET_BIT64(ExtLockMasks[track], step_count);

    conditional[step_count] = condition;
    timing[step_count] = utiming;
  }
}

void SeqExtTrack::clear_ext_conditional() {
  for (uint8_t c = 0; c < 128; c++) {
    conditional[c] = 0;
    timing[c] = 0;
  }
}
void SeqExtTrack::clear_ext_locks() {
  for (uint8_t c = 0; c < 4; c++) {
    for (uint8_t x = 0; x < 128; x++) {
      notes[c][x] = 0;
    }
    // ExtPatternNotesParams[i][c] = 0;
  }
}

void SeqExtTrack::clear_track() {
  clear_locks();
  clear_conditional();
  seq_buffer_notesoff();
  lockmask = 0;


}
