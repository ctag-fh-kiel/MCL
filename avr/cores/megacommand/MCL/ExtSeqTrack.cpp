#include "ExtSeqTrack.h"
#include "MCL.h"

float ExtSeqTrack::get_scale_multiplier(bool inverse) {
  return get_scale_multiplier(scale, inverse);
}

float ExtSeqTrack::get_scale_multiplier(uint8_t scale, bool inverse) {
  float multi;
  switch (scale) {
  default:
  case EXT_SCALE_1X:
    multi = 1;
    break;
  case EXT_SCALE_2X:
    if (inverse) { multi = 2; }
    else { multi = 0.5; }
    break;
  case EXT_SCALE_3_2X:
    if (inverse) { multi = 3.0 / 4.0; }
    else { multi = (4.0 / 3.0); }
    break;
  case EXT_SCALE_3_4X:
    if (inverse) { multi = 3.0 / 2.0; }
    else { multi = (2.0 / 3.0); }
    break;
  case EXT_SCALE_1_2X:
    if (inverse) { multi = 1.0 / 2.0; }
    else { multi = 2.0; }
    break;
  case EXT_SCALE_1_4X:
    if (inverse) { multi = 1.0 / 4.0; }
    else { multi = 4.0; }
    break;
  case EXT_SCALE_1_8X:
    if (inverse) { multi = 1.0 / 8.0; }
    else { multi = 8.0; }
    break;
  }
  return multi;
}

void ExtSeqTrack::set_scale(uint8_t _scale) {
  uint8_t old_scale = scale;
  float mult = get_scale_multiplier(_scale) / get_scale_multiplier(old_scale);
  for (uint8_t i = 0; i < 128; i++) {
    timing[i] = round(mult * (float)timing[i]);
  }
  scale = _scale;
  uint8_t timing_mid = get_timing_mid();
  if (mod12_counter > timing_mid) {
  mod12_counter = mod12_counter - (mod12_counter / timing_mid) * timing_mid;
  //step_count_inc();
  }
}


void ExtSeqTrack::set_length(uint8_t len) {
  length = len;
  if (step_count >= length) {
    step_count = length % step_count;
  }
  DEBUG_DUMP(step_count);
}

void ExtSeqTrack::seq() {
  if (mute_until_start) {

    if (clock_diff(MidiClock.div16th_counter, start_step) == 0) {
      init();
    }
  }
  uint8_t timing_mid = get_timing_mid();
  if ((MidiUart2.uart_block == 0) && (mute_until_start == false) &&
      (mute_state == SEQ_MUTE_OFF)) {


    uint8_t next_step = 0;
    if (step_count == length) {
      next_step = 0;
    } else {
      next_step = step_count + 1;
    }

    for (uint8_t c = 0; c < 4; c++) {
      uint8_t current_step;
      if (((timing[step_count] >= timing_mid) &&
           ((timing[current_step = step_count] - timing_mid) == mod12_counter)) ||
          ((timing[next_step] < timing_mid) && ((timing[current_step = next_step]) == mod12_counter))) {

        if (notes[c][current_step] < 0) {
          note_off(abs(notes[c][current_step]) - 1);
        } else if (notes[c][current_step] > 0) {
          noteon_conditional(conditional[current_step],
                             abs(notes[c][current_step]) - 1);
        }
      }
    }
  }
    mod12_counter++;

  if (mod12_counter == timing_mid) {
    mod12_counter = 0;
    step_count_inc();
  }

}
void ExtSeqTrack::note_on(uint8_t note) {
  uart->sendNoteOn(channel, note, 100);
  DEBUG_PRINTLN("note on");
  DEBUG_DUMP(note);
  // Greater than 64
  if (IS_BIT_SET(note, 6)) {
    SET_BIT64(note_buffer[1], note - 64);
  } else {
    SET_BIT64(note_buffer[0], note);
  }
}

void ExtSeqTrack::note_off(uint8_t note) {
  uart->sendNoteOff(channel, note, 0);

  // Greater than 64
  if (IS_BIT_SET(note, 6)) {
    CLEAR_BIT64(note_buffer[1], note - 64);
  } else {
    CLEAR_BIT64(note_buffer[0], note);
  }
}

void ExtSeqTrack::noteon_conditional(uint8_t condition, uint8_t note) {
  switch (condition) {
  case 0:
  case 1:
    if (!IS_BIT_SET128(oneshot_mask, step_count)) {
      note_on(note);
    }
    break;
  case 2:
    if (!IS_BIT_SET(iterations_8, 0)) {
      note_on(note);
    }
    break;
  case 3:
    if ((iterations_6 == 3) || (iterations_6 == 6)) {
      note_on(note);
    }
    break;
  case 6:
    if (iterations_6 == 6) {
      note_on(note);
    }
    break;
  case 4:
    if ((iterations_8 == 4) || (iterations_8 == 8)) {
      note_on(note);
    }
    break;
  case 8:
    if ((iterations_8 == 8)) {
      note_on(note);
    }
  case 5:
    if (iterations_5 == 5) {
      note_on(note);
    }
    break;
  case 7:
    if (iterations_7 == 7) {
      note_on(note);
    }
    break;
  case 9:
    if (get_random_byte() <= 13) {
      note_on(note);
    }
    break;
  case 10:
    if (get_random_byte() <= 32) {
      note_on(note);
    }
    break;
  case 11:
    if (get_random_byte() <= 64) {
      note_on(note);
    }
    break;
  case 12:
    if (get_random_byte() <= 96) {
      note_on(note);
    }
    break;
  case 13:
    if (get_random_byte() <= 115) {
      note_on(note);
    }
    break;
  case 14:
    if (!IS_BIT_SET128(oneshot_mask, step_count)) {
      SET_BIT128(oneshot_mask, step_count);
      note_on(note);
    }
  }
}

void ExtSeqTrack::set_ext_track_step(uint8_t step, uint8_t note_num,
                                     uint8_t velocity) {
  uint8_t match = 255;
  // Look for matching note already on this step
  // If it's a note off, then disable the note
  // If it's a note on, set the note note-off.
  for (uint8_t c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step] == -(1 * (note_num + 1))) {
      notes[c][step] = 0;
      buffer_notesoff();
      match = c;
    }
    if (notes[c][step] == (1 * (note_num + 1))) {
      notes[c][step] = (-1 * (note_num + 1));
      match = c;
    }
  }
  // No matches are found, we count number of on and off to determine next
  // note type.
  for (uint8_t c = 0; c < 4 && match == 255; c++) {
    if (notes[c][step] == 0) {
      match = c;
      int8_t ons_and_offs = 0;
      // Check to see if we have same number of note offs as note ons.
      // If there are more note ons for given note, the next note entered
      // should be a note off.
      for (uint8_t a = 0; a < length; a++) {
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
void ExtSeqTrack::record_ext_track_noteoff(uint8_t note_num, uint8_t velocity) {
  uint8_t utiming = (mod12_counter + get_timing_mid());

  uint8_t condition = 0;
  uint8_t match = 255;
  uint8_t c = 0;

  for (c = 0; c < 4 && match == 255; c++) {
    if (abs(notes[c][step_count]) == note_num + 1) {
      match = c;

      if (notes[c][step_count] > 0) {
        step_count = step_count + 1;
        if (step_count > length) {
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

void ExtSeqTrack::record_ext_track_noteon(uint8_t note_num, uint8_t velocity) {

  uint8_t utiming = (mod12_counter + get_timing_mid());
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

void ExtSeqTrack::clear_ext_conditional() {
  for (uint8_t c = 0; c < 128; c++) {
    conditional[c] = 0;
    timing[c] = 0;
  }
}
void ExtSeqTrack::clear_ext_notes() {
  for (uint8_t c = 0; c < 4; c++) {
    for (uint8_t x = 0; x < 128; x++) {
      notes[c][x] = 0;
    }
    // ExtPatternNotesParams[i][c] = 0;
  }
}

void ExtSeqTrack::clear_track() {
  clear_ext_notes();
  clear_ext_conditional();
  buffer_notesoff();
}

void ExtSeqTrack::rotate_left() {

  int8_t new_pos = 0;

  ExtSeqTrackData temp_data;

  memcpy(&temp_data, this, sizeof(ExtSeqTrackData));

  for (uint8_t a = 0; a < 4; a++) {
    lock_masks[a] = 0;
  }
  oneshot_mask[0] = 0;
  oneshot_mask[1] = 0;

  for (uint8_t n = 0; n < length; n++) {
    if (n == 0) {
      new_pos = length - 1;
    } else {
      new_pos = n - 1;
    }

    for (uint8_t a = 0; a < 4; a++) {
      notes[a][new_pos] = temp_data.notes[a][n];
      locks[a][new_pos] = temp_data.locks[a][n];
      if (IS_BIT_SET64(temp_data.lock_masks[a], n)) {
        SET_BIT64(temp_data.lock_masks[a], new_pos);
      }
    }

    conditional[new_pos] = temp_data.conditional[n];
    timing[new_pos] = temp_data.timing[n];
  }
}
void ExtSeqTrack::rotate_right() {

  int8_t new_pos = 0;

  ExtSeqTrackData temp_data;

  memcpy(&temp_data, this, sizeof(ExtSeqTrackData));

  for (uint8_t a = 0; a < 4; a++) {
    lock_masks[a] = 0;
  }
  oneshot_mask[0] = 0;
  oneshot_mask[1] = 0;

  for (uint8_t n = 0; n < length; n++) {
    if (n == length - 1) {
      new_pos = 0;
    } else {
      new_pos = n + 1;
    }

    for (uint8_t a = 0; a < 4; a++) {
      notes[a][new_pos] = temp_data.notes[a][n];
      locks[a][new_pos] = temp_data.locks[0][n];
      if (IS_BIT_SET64(temp_data.lock_masks[a], n)) {
        SET_BIT64(temp_data.lock_masks[a], new_pos);
      }
    }

    conditional[new_pos] = temp_data.conditional[n];
    timing[new_pos] = temp_data.timing[n];
  }
}

void ExtSeqTrack::reverse() {

  int8_t new_pos = 0;

  ExtSeqTrackData temp_data;

  memcpy(&temp_data, this, sizeof(ExtSeqTrackData));

  for (uint8_t a = 0; a < 4; a++) {
    lock_masks[a] = 0;
  }
  oneshot_mask[0] = 0;
  oneshot_mask[1] = 0;

  for (uint8_t n = 0; n < length; n++) {
    new_pos = length - n - 1;

    for (uint8_t a = 0; a < 4; a++) {
      notes[a][new_pos] = temp_data.notes[a][n];
      locks[a][new_pos] = temp_data.locks[0][n];
      if (IS_BIT_SET64(temp_data.lock_masks[a], n)) {
        SET_BIT64(temp_data.lock_masks[a], new_pos);
      }
    }

    conditional[new_pos] = temp_data.conditional[n];
    timing[new_pos] = temp_data.timing[n];
  }
}
