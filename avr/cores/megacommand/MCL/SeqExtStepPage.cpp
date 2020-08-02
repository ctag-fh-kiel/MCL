#include "MCL.h"
#include "SeqExtStepPage.h"

void SeqExtStepPage::setup() { SeqPage::setup(); }
void SeqExtStepPage::config() {
#ifdef EXT_TRACKS
//  seq_param3.cur = mcl_seq.ext_tracks[last_ext_track].length;
#endif
  // config info labels
  constexpr uint8_t len1 = sizeof(info1);

#ifdef EXT_TRACKS
/*
  if (mcl_seq.ext_tracks[last_ext_track].speed == EXT_SPEED_2X) {
    strcpy(info1, "HI-RES");
  } else {
    strcpy(info1, "LOW-RES");
  }
*/
#endif

  strcpy(info2, "EXT");

  // config menu
  config_as_trackedit();

  // use continuous page index display
  display_page_index = false;
}

void SeqExtStepPage::config_encoders() {
#ifdef EXT_TRACKS
  uint8_t timing_mid = mcl_seq.ext_tracks[last_ext_track].get_timing_mid();
  seq_param1.max = 127;
  seq_param1.cur = 64;
  seq_param1.old = 64;

  seq_param2.cur = 64;
  seq_param2.old = 64;

  seq_param3.handler = NULL;
  seq_param3.cur = 64;
  seq_param3.old = 64;
  seq_param3.max = 128;

  fov_offset = 0;
  cur_x = 0;
  cur_y = 64;
  fov_y = 64;
  cur_w = timing_mid;

  seq_param2.max = 127;
  seq_param4.max = 128;
  seq_param4.cur = 16;
  seq_param4.min = 4;
  config();
  SeqPage::midi_device = midi_active_peering.get_device(UART2_PORT);
#endif
}

void SeqExtStepPage::init() {
  page_count = 8;
  DEBUG_PRINTLN("seq extstep init");
  curpage = SEQ_EXTSTEP_PAGE;
  trig_interface.on();
  note_interface.state = true;
  config_encoders();
  midi_events.setup_callbacks();
  seq_menu_page.menu.enable_entry(SEQ_MENU_TRACK, true);
}

void SeqExtStepPage::cleanup() {
  SeqPage::cleanup();
  midi_events.remove_callbacks();
}

#define MAX_FOV_W 96

uint8_t SeqExtStepPage::find_note_off(int8_t note_val, uint8_t step) {
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  uint8_t match = 255;
  // Scan for matching note off;
  for (uint8_t j = step + 1; j < active_track.length && match == 255; j++) {
    for (uint8_t b = 0; b < NUM_EXT_NOTES; b++) {
      if (active_track.notes[b][j] == -1 * note_val) {
        match = j;
      }
    }
  }
  // Wrap around
  for (uint8_t j = 0; j < step && match == 255; j++) {
    for (uint8_t b = 0; b < NUM_EXT_NOTES; b++) {
      if (active_track.notes[b][j] == -1 * note_val) {
        match = j;
      }
    }
  }
  if (match == 255) {
    return step;
  }
  return match;
}

void SeqExtStepPage::draw_pianoroll() {
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  uint8_t timing_mid = active_track.get_timing_mid();

  // Absolute piano roll dimensions
  roll_length = active_track.length * timing_mid; // in ticks
  uint8_t roll_height = 127;                      // 127, Notes.

  // FOV offsets

  if (seq_param4.cur > active_track.length) {
    seq_param4.cur = active_track.length;
  }
  uint8_t fov_zoom = seq_param4.cur;

  DEBUG_DUMP(fov_zoom);
  fov_length = fov_zoom * timing_mid; // how many ticks to display on screen.

  fov_pixels_per_tick = (float)fov_w / (float)fov_length;

  // if ((1.00 / fov_pixels_per_tick) < 1) {
  //    fov_pixels_per_tick = 1;
  //   fov_length = fov_w;
  // }
  DEBUG_DUMP(fov_zoom);
  DEBUG_DUMP(fov_length);
  DEBUG_DUMP(fov_pixels_per_tick);
  uint16_t cur_tick_x =
      active_track.step_count * timing_mid + active_track.mod12_counter;

  // Draw vertical keyboard
  const uint16_t chromatic = 0b0000010101001010;
  for (uint8_t k = 0; k < fov_notes; k++) {
    uint8_t scale_pos =
        (fov_y + fov_notes - k) - (((fov_y + fov_notes - k) / 12) * 12);
    if (!IS_BIT_SET16(chromatic, scale_pos)) {
      oled_display.fillRect(draw_x - keyboard_w,
                            draw_y + k * (fov_h / fov_notes), keyboard_w,
                            (fov_h / fov_notes), WHITE);
    }
  }
  oled_display.drawLine(draw_x-1 , 0, draw_x-1 , fov_h, WHITE);

  // Draw sequencer position..
  if (is_within_fov(cur_tick_x)) {

    uint8_t cur_tick_fov_x =
        min(127, draw_x + fov_pixels_per_tick * (cur_tick_x - fov_offset));
    oled_display.drawLine(cur_tick_fov_x, 0, cur_tick_fov_x, fov_h - 1, WHITE);
  }

  uint16_t pattern_end_x = active_track.length * timing_mid;
  uint8_t pattern_end_fov_x = fov_w;

  if (is_within_fov(pattern_end_x)) {
    pattern_end_fov_x =
        min(fov_w, fov_pixels_per_tick * (pattern_end_x - fov_offset));
  }

  for (int i = 0; i < active_track.length; i++) {
    // Draw grid.
    uint16_t grid_tick_x = i * timing_mid;
    if (is_within_fov(grid_tick_x)) {
      uint8_t grid_fov_x =
          draw_x + fov_pixels_per_tick * (grid_tick_x - fov_offset);

      for (uint8_t k = 0; k < fov_notes; k += 1) {
        // draw crisscross
        // if ((fov_y + k + i) % 2 == 0) { oled_display.drawPixel( grid_fov_x,
        // (k * (fov_h / fov_notes)), WHITE); }
        oled_display.drawPixel(grid_fov_x, draw_y + (k * (fov_h / fov_notes)),
                               WHITE);
      }
    }

    for (uint8_t a = 0; a < NUM_EXT_NOTES; a++) {
      int8_t note_val = active_track.notes[a][i];
      // Check if note is note_on (positive) and is visible within fov vertical
      // range.
      if (note_val > 0) {
        uint8_t j = find_note_off(abs(note_val), i);
        uint16_t note_start =
            i * timing_mid + active_track.timing[i] - timing_mid;
        uint16_t note_end =
            j * timing_mid + active_track.timing[j] - timing_mid;

        if (is_within_fov(note_start) || is_within_fov(note_end) ||
            ((note_start < fov_offset) &&
             (note_end >= fov_offset + fov_length))) {
          uint8_t note_fov_start, note_fov_end;

          if (note_start < fov_offset) {
            note_fov_start = 0;
          } else {
            note_fov_start =
                (float)(note_start - fov_offset) * fov_pixels_per_tick;
          }

          if (note_end >= fov_offset + fov_length) {
            note_fov_end = fov_w;

          } else {
            note_fov_end = (float)(note_end - fov_offset) * fov_pixels_per_tick;
          }

          uint8_t note_fov_y =
              fov_h - ((note_val - fov_y) * (fov_h / fov_notes));
          /*
                    DEBUG_DUMP("Found note");
                    DEBUG_DUMP(note_start);
                    DEBUG_DUMP(note_end);
                    DEBUG_DUMP(note_fov_start);
                    DEBUG_DUMP(note_fov_end);
                    DEBUG_DUMP(note_fov_y);
                    DEBUG_DUMP(fov_pixels_per_tick);
          */

          // Draw vertical projection
          uint8_t proj_y = 255;
          if ((note_val >= fov_y + fov_notes) && (cur_y == fov_y + fov_notes - 1)) {
            proj_y = 0;
          }
          if ((note_val < fov_y) && (cur_y == fov_y)) {
            proj_y = draw_y + fov_h + 1;
          }
          if (proj_y != 255) {
            if (note_end < note_start) {
              // Wrap around note

              if (note_start < fov_offset + fov_length) {
                oled_display.drawRect(note_fov_start + draw_x, proj_y,
                                      pattern_end_fov_x - note_fov_start, 1,
                                      WHITE);
              }

              if (note_end > fov_offset) {
                oled_display.drawRect(draw_x, proj_y, note_fov_end, 1, WHITE);
              }

            } else {
              // Standard note.
              oled_display.drawRect(note_fov_start + draw_x, proj_y,
                                    note_fov_end - note_fov_start, 1, WHITE);
            }
          }
          // Draw notes
          if ((note_val >= fov_y) && (note_val < fov_y + fov_notes)) {
            if (note_end < note_start) {
              // Wrap around note
              if (note_start < fov_offset + fov_length) {
                oled_display.drawRect(note_fov_start + draw_x,
                                      draw_y + note_fov_y,
                                      pattern_end_fov_x - note_fov_start,
                                      (fov_h / fov_notes), WHITE);
              }

              if (note_end > fov_offset) {
                oled_display.drawRect(draw_x, draw_y + note_fov_y, note_fov_end,
                                      (fov_h / fov_notes), WHITE);
              }

            } else {
              // Standard note.
              oled_display.drawRect(
                  note_fov_start + draw_x, draw_y + note_fov_y,
                  note_fov_end - note_fov_start, (fov_h / fov_notes), WHITE);
            }
          }
        }
      }
    }
  }
  // Draw interactive cursor
  DEBUG_DUMP(cur_w);
  uint8_t fov_cur_y = fov_h - ((cur_y - fov_y + 1) * (fov_h / fov_notes));
  int16_t fov_cur_x = (float)(cur_x - fov_offset) * fov_pixels_per_tick;
  uint8_t fov_cur_w = (float)(cur_w)*fov_pixels_per_tick;
  if (fov_cur_x < 0) {
    fov_cur_x = 0;
  }
  if (fov_cur_x + fov_cur_w > fov_w) {
    fov_cur_w = fov_w - fov_cur_x;
  }
  oled_display.fillRect(draw_x + fov_cur_x, draw_y + fov_cur_y, fov_cur_w,
                        (fov_h / fov_notes), WHITE);
}

void SeqExtStepPage::draw_viewport_minimap() {
#ifdef OLED_DISPLAY
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  uint8_t timing_mid = active_track.get_timing_mid();
  constexpr uint16_t width = pidx_w*4+3;
  uint16_t pattern_end = active_track.length * timing_mid;
  uint16_t cur_tick_x = active_track.step_count * timing_mid + active_track.mod12_counter;

  oled_display.drawRect(pidx_x0, pidx_y, width, pidx_h, WHITE);

  // viewport is [fov_offset, fov_offset+fov_length] out of [0, pattern_end]

  uint16_t s = fov_offset * (width-1) / pattern_end;
  uint16_t w = fov_length * (width-2) / pattern_end;
  uint16_t p = cur_tick_x * (width-1) / pattern_end;
  oled_display.drawFastHLine(pidx_x0 + 1 + s, pidx_y + 1, w, WHITE);
  oled_display.drawPixel(pidx_x0 + 1 + p, pidx_y + 1, INVERT);
#endif
}


void SeqExtStepPage::draw_note(uint8_t note_val, uint16_t note_start,
                               uint16_t note_end) {}

#ifndef OLED_DISPLAY
void SeqExtStepPage::display() { SeqPage::display(); }
#else

void SeqExtStepPage::loop() {
  SeqPage::loop();
  if (seq_param1.hasChanged()) {
    // Vertical translation
    int16_t diff = seq_param1.cur - seq_param1.old;

    DEBUG_DUMP(diff);
    if (diff < 0) {
      if (cur_x <= fov_offset) {
        fov_offset += diff;
        // / fov_pixels_per_tick;
        if (fov_offset < 0) {
          fov_offset = 0;
        }
        cur_x = fov_offset;
      } else {
        cur_x += diff;
        if (cur_x < fov_offset) {
          cur_x = fov_offset;
        }
      }

    } else {
      if (cur_x >= fov_offset + fov_length - cur_w) {
        if (fov_offset + fov_length + diff < roll_length) {
          fov_offset += diff;
          cur_x = fov_offset + fov_length - cur_w;
        }
      } else {
        cur_x += diff;
        if (cur_x > fov_offset + fov_length - cur_w) {
          cur_x = fov_offset + fov_length - cur_w;
        }
      }
    }

    seq_param1.cur = 64;
    seq_param1.old = 64;
  }

  if (seq_param2.hasChanged()) {
    // Horizontal translation
    int16_t diff = seq_param2.old - seq_param2.cur; // reverse dir for sanity.

    DEBUG_DUMP(diff);
    if (diff < 0) {
      if (cur_y <= fov_y) {
        fov_y += diff;
        if (fov_y < 0) {
          fov_y = 0;
        }
        cur_y = fov_y;
      } else {
        cur_y += diff;
        if (cur_y < fov_y) {
          cur_y = fov_y;
        }
      }
    } else {
      if (cur_y >= fov_y + fov_notes - 1) {
        fov_y += diff;
        if (fov_y + fov_notes > 127) {
          fov_y = 127 - fov_notes;
        }
        cur_y = fov_y + fov_notes - 1;
      } else {
        cur_y += diff;
        if (cur_y > fov_y + fov_notes - 1) {
          cur_y = fov_y + fov_notes - 1;
        }
      }
    }
    seq_param2.cur = 64;
    seq_param2.old = 64;
  }

  if (seq_param3.hasChanged()) {

    int16_t diff = seq_param3.cur - seq_param3.old;

    if (diff < 0) {
      cur_w += diff;
      if (cur_w < cur_w_min) {
        cur_w = cur_w_min;
      }
    } else {
      if (cur_x >= fov_offset + fov_length - cur_w - diff) {
        if (fov_offset + fov_length + diff < roll_length) {
          cur_w += diff;
          fov_offset += diff;
        }
      } else {
        cur_w += diff;
      }
    }
    seq_param3.cur = 64;
    seq_param3.old = 64;
  }
}

void SeqExtStepPage::display() {

#ifdef EXT_TRACKS
  oled_display.clearDisplay();

  auto &active_track = mcl_seq.ext_tracks[last_ext_track];

  draw_viewport_minimap();
  draw_pianoroll();

  // draw_knob_conditional(seq_param1.getValue());
  // draw_knob_timing(seq_param2.getValue(),timing_mid);

  /*
  MusicalNotes number_to_note;
  uint8_t notes_held = 0;
  uint8_t i, j;
  for (i = 0; i < 16; i++) {
    if (note_interface.notes[i] == 1) {
      notes_held += 1;
    }
  }
  char K[4];
  itoa(seq_param3.getValue(), K, 10);
  draw_knob(2, "LEN", K);

  if (notes_held > 0) {
    uint8_t x = mcl_gui.knob_x0 + mcl_gui.knob_w * 3 + 2;
    auto *oldfont = oled_display.getFont();
    oled_display.setFont(&TomThumb);
    uint8_t note_idx = 0;
    for (i = 0; i < 2; i++) {
      for (j = 0; j < 2; j++) {
        oled_display.setCursor(x + j * 11, 6 + i * 8);
        const int8_t &c_note =
            active_track
                .notes[note_idx][note_interface.last_note + page_select * 16];
        if (c_note != 0) {
          uint8_t note = abs(c_note);
          DEBUG_DUMP(c_note);
          DEBUG_DUMP(note);
          note = note - 1;
          uint8_t oct = note / 12;
          note = note - 12 * oct;
          DEBUG_DUMP(note);
          DEBUG_DUMP(oct);
          if (c_note > 0) {
            oled_display.print(number_to_note.notes_upper[note]);
          } else {
            oled_display.print(number_to_note.notes_lower[note]);
          }

          oled_display.print(oct);
        }

        ++note_idx;
      }
    }
    oled_display.setFont(oldfont);
  }
  */
  if (mcl_gui.show_encoder_value(&seq_param2) &&
      (note_interface.notes_count_on() > 0) && (!show_seq_menu) &&
      (!show_step_menu)) {
    mcl_gui.draw_microtiming(
        get_ext_speed(mcl_seq.ext_tracks[last_ext_track].speed),
        seq_param2.cur);
  }

  SeqPage::display();
  oled_display.display();
#endif
}
#endif

bool SeqExtStepPage::handleEvent(gui_event_t *event) {

#ifdef EXT_TRACKS
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  if (note_interface.is_event(event)) {
    uint8_t mask = event->mask;
    uint8_t port = event->port;
    uint8_t device = midi_active_peering.get_device(port);
    uint8_t track = event->source - 128;

    if (device == DEVICE_A4) {
      track -= 16;
    }

    if (mask == EVENT_BUTTON_PRESSED) {
      DEBUG_PRINTLN(track);
      if (device == DEVICE_MD) {

        if ((track + (page_select * 16)) >= active_track.length) {
          DEBUG_PRINTLN("setting to 0");
          DEBUG_PRINTLN(last_ext_track);
          DEBUG_PRINTLN(page_select);
          note_interface.notes[track] = 0;
          return true;
        }

        int8_t utiming =
            active_track.timing[(track + (page_select * 16))]; // upper
        uint8_t condition =
            active_track.conditional[(track + (page_select * 16))]; // lower
        // seq_param1.cur = translate_to_knob_conditional(condition);
        // Micro
        if (utiming == 0) {
          utiming = mcl_seq.ext_tracks[last_ext_track].get_timing_mid();
        }
        seq_param2.cur = utiming;

        note_interface.last_note = track;
      }
    }
    if (mask == EVENT_BUTTON_RELEASED) {
      if (device == DEVICE_MD) {

        uint8_t utiming = (seq_param2.cur + 0);
        uint8_t condition = translate_to_step_conditional(seq_param1.cur);
        if ((track + (page_select * 16)) >= active_track.length) {
          return true;
        }

        //  timing = 3;
        // condition = 3;
        if (clock_diff(note_interface.note_hold, slowclock) < TRIG_HOLD_TIME) {
          for (uint8_t c = 0; c < NUM_EXT_NOTES; c++) {
            if (active_track.notes[c][track + page_select * 16] > 0) {
              MidiUart2.sendNoteOff(
                  last_ext_track,
                  abs(active_track.notes[c][track + page_select * 16]) - 1, 0);
            }
            active_track.notes[c][track + page_select * 16] = 0;
          }
          active_track.timing[(track + (page_select * 16))] = 0;
          active_track.conditional[(track + (page_select * 16))] = 0;
        }

        else {
          active_track.timing[(track + (page_select * 16))] = utiming; // upper
          active_track.conditional[(track + (page_select * 16))] = condition; // upper
        }
      }
      return true;
    }
    return true;
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON1)) {
    GUI.setPage(&seq_step_page);
    return true;
  }

#endif

  return SeqPage::handleEvent(event);
}

void SeqExtStepMidiEvents::onNoteOnCallback_Midi2(uint8_t *msg) {
  // Step edit for ExtSeq
  // For each incoming note, check to see if note interface has any steps
  // selected For selected steps record notes.
#ifdef EXT_TRACKS
  uint8_t channel = MIDI_VOICE_CHANNEL(msg[0]);
  DEBUG_PRINT("note on midi2 ext, ");
  DEBUG_DUMP(channel);

  if (channel < mcl_seq.num_ext_tracks) {
    last_ext_track = channel;
    seq_extstep_page.config_encoders();

    if (MidiClock.state != 2) {
      mcl_seq.ext_tracks[channel].note_on(msg[1]);
    }

    for (uint8_t i = 0; i < 16; i++) {
      if (note_interface.notes[i] == 1) {
        mcl_seq.ext_tracks[channel].set_ext_track_step(
            seq_extstep_page.page_select * 16 + i, msg[1], msg[2]);
      }
    }
  }
#endif
}

void SeqExtStepMidiEvents::onNoteOffCallback_Midi2(uint8_t *msg) {
#ifdef EXT_TRACKS
  uint8_t channel = MIDI_VOICE_CHANNEL(msg[0]);
  if (channel < mcl_seq.num_ext_tracks && MidiClock.state != 2) {
    mcl_seq.ext_tracks[channel].note_off(msg[1]);
  }
#endif
}

void SeqExtStepMidiEvents::setup_callbacks() {
  if (state) {
    return;
  }
  Midi2.addOnNoteOnCallback(
      this, (midi_callback_ptr_t)&SeqExtStepMidiEvents::onNoteOnCallback_Midi2);
  Midi2.addOnNoteOffCallback(
      this,
      (midi_callback_ptr_t)&SeqExtStepMidiEvents::onNoteOffCallback_Midi2);

  state = true;
}

void SeqExtStepMidiEvents::remove_callbacks() {

  if (!state) {
    return;
  }
  Midi2.removeOnNoteOnCallback(
      this, (midi_callback_ptr_t)&SeqExtStepMidiEvents::onNoteOnCallback_Midi2);
  Midi2.removeOnNoteOffCallback(
      this,
      (midi_callback_ptr_t)&SeqExtStepMidiEvents::onNoteOffCallback_Midi2);
  state = false;
}
