#include "MCL_impl.h"

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
  seq_param3.min = 1;

  fov_offset = 0;
  cur_x = 0;
  fov_y = 64;
  cur_y = fov_y + 1;
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
  DEBUG_PRINTLN(F("seq extstep init"));
  SeqPage::init();
  param_select = 128;
  trig_interface.on();
  note_interface.state = true;
  x_notes_down = 0;
  config_encoders();
  midi_events.setup_callbacks();
  seq_menu_page.menu.enable_entry(SEQ_MENU_TRACK, true);
  seq_menu_page.menu.enable_entry(SEQ_MENU_LENGTH, true);
  seq_menu_page.menu.enable_entry(SEQ_MENU_CHANNEL, true);
  seq_menu_page.menu.enable_entry(SEQ_MENU_PIANOROLL, true);
}

void SeqExtStepPage::cleanup() {
  SeqPage::cleanup();
  midi_events.remove_callbacks();
}

#define MAX_FOV_W 96

void SeqExtStepPage::draw_lockeditor() {
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  uint8_t timing_mid = active_track.get_timing_mid();

  // Absolute piano roll dimensions
  roll_length = active_track.length * timing_mid; // in ticks

  if (seq_param4.cur > 32) {
    seq_param4.cur = 32;
  }
  if (seq_param4.cur > active_track.length) {
    seq_param4.cur = active_track.length;
  }

  uint8_t fov_zoom = seq_param4.cur;

  fov_length = fov_zoom * timing_mid; // how many ticks to display on screen.

  fov_pixels_per_tick = (float)fov_w / (float)fov_length;

  uint16_t cur_tick_x =
      active_track.step_count * timing_mid + active_track.mod12_counter;

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

  uint8_t h = fov_h / fov_notes;
  uint16_t ev_idx = 0, ev_end = 0, ev_j_end;
  uint8_t j = 0;

  for (uint8_t i = 0; i < active_track.length; i++) {
    // Update bucket index range
    if (j > i) {
      i = j;
      ev_end = ev_j_end;
      // active_track.locate(i, ev_idx, ev_end);
    } else {
      ev_end += active_track.timing_buckets.get(i);
    }
    uint16_t grid_tick_x = i * timing_mid;
    if (is_within_fov(grid_tick_x)) {
      uint8_t grid_fov_x =
          draw_x + fov_pixels_per_tick * (grid_tick_x - fov_offset);

      for (uint8_t k = 0; k < fov_notes; k += 1) {
        // draw crisscross
        // if ((fov_y + k + i) % 2 == 0) { oled_display.drawPixel(
        // grid_fov_x, (k * (fov_h / fov_notes)), WHITE); }
        oled_display.drawPixel(grid_fov_x, draw_y + (k * (h)), WHITE);

        if (i % 16 == 0) {
          oled_display.drawPixel(grid_fov_x, draw_y + (k * h) + (h / 2), WHITE);
        }
      }
    }

    for (; ev_idx != ev_end; ++ev_idx) {
      auto &ev = active_track.events[ev_idx];
      int lock_val = ev.event_value;
      if (!ev.is_lock || (ev.lock_idx != pianoroll_mode - 1)) {
        continue;
      }
      uint16_t next_lock_ev = ev_idx;
      ev_j_end = ev_end;
      j = active_track.search_lock_idx(pianoroll_mode - 1, i, next_lock_ev,
                                       ev_j_end);
      if (next_lock_ev == 0xFFFF) {
        next_lock_ev = ev_idx;
      }
      auto &ev_j = active_track.events[next_lock_ev];

      uint16_t lock_start = i * timing_mid + ev.micro_timing - timing_mid;
      uint16_t lock_end = j * timing_mid + ev_j.micro_timing - timing_mid;

      if (is_within_fov(lock_start) || is_within_fov(lock_end) ||
          ((lock_start < fov_offset) &&
           (lock_end >= fov_offset + fov_length))) {
        uint8_t lock_fov_start, lock_fov_end;

        if (lock_start < fov_offset) {
          lock_fov_start = 0;
        } else {
          lock_fov_start =
              (float)(lock_start - fov_offset) * fov_pixels_per_tick;
        }

        if (lock_end >= fov_offset + fov_length) {
          lock_fov_end = fov_w;

        } else {
          lock_fov_end = (float)(lock_end - fov_offset) * fov_pixels_per_tick;
        }
        uint8_t lock_fov_start_y =
            fov_h - (((float)ev.event_value / 128.0) * (float)fov_h);
        uint8_t lock_fov_end_y =
            fov_h - (((float)ev_j.event_value / 128.0) * (float)fov_h);

        if (lock_end < lock_start) {
          float gradient = float(lock_fov_end_y - lock_fov_start_y) /
                           (float)(lock_fov_end - lock_fov_start);
          // Wrap around note
          /*
                    if (lock_start < fov_offset + fov_length) {
                oled_display.drawLine(lock_fov_start + draw_x,
                                      draw_y + note_fov_y,
                                      pattern_end_fov_x - note_fov_start,
                                      (fov_h / fov_notes), WHITE);
              }

              if (note_end > fov_offset) {
                oled_display.drawRect(draw_x, draw_y + note_fov_y, note_fov_end,
                                      (fov_h / fov_notes), WHITE);
              }
  */
        } else {
          // Standard note.
          oled_display.drawLine(lock_fov_start + draw_x, lock_fov_start_y,
                                draw_x + lock_fov_end, lock_fov_end_y, WHITE);
          oled_display.drawLine(lock_fov_start + draw_x, lock_fov_start_y + 1,
                                draw_x + lock_fov_end, lock_fov_end_y + 1,
                                WHITE);
        }
      }
    }
    if (j < i) {
      break;
    }
  }
  // Draw interactive cursor
  int16_t fov_cur_x = (float)(cur_x - fov_offset) * fov_pixels_per_tick;
  uint8_t fov_cur_w = (float)(cur_w)*fov_pixels_per_tick;
  uint8_t fov_cur_y = fov_h - ((float)lock_cur_y / 128.0 * (float)fov_h);

  oled_display.drawPixel(draw_x + fov_cur_x - 1, draw_y + fov_cur_y, WHITE);
  oled_display.drawPixel(draw_x + fov_cur_x + 1, draw_y + fov_cur_y, WHITE);
  oled_display.drawPixel(draw_x + fov_cur_x, draw_y + fov_cur_y - 1, WHITE);
  oled_display.drawPixel(draw_x + fov_cur_x, draw_y + fov_cur_y + 1, WHITE);
}

void SeqExtStepPage::draw_pianoroll() {
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  uint8_t timing_mid = active_track.get_timing_mid();

  // Absolute piano roll dimensions
  roll_length = active_track.length * timing_mid; // in ticks
  uint8_t roll_height = 127;                      // 127, Notes.

  // FOV offsets

  if (seq_param4.cur > 32) {
    seq_param4.cur = 32;
  }
  if (seq_param4.cur > active_track.length) {
    seq_param4.cur = active_track.length;
  }

  uint8_t fov_zoom = seq_param4.cur;

  fov_length = fov_zoom * timing_mid; // how many ticks to display on screen.

  fov_pixels_per_tick = (float)fov_w / (float)fov_length;

  // if ((1.00 / fov_pixels_per_tick) < 1) {
  //    fov_pixels_per_tick = 1;
  //   fov_length = fov_w;
  // }
  uint16_t cur_tick_x =
      active_track.step_count * timing_mid + active_track.mod12_counter;

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

  uint16_t ev_idx = 0, ev_end = 0;
  uint8_t h = fov_h / fov_notes;
  for (int i = 0; i < active_track.length; i++) {
    // Update bucket index range
    ev_end += active_track.timing_buckets.get(i);
    // Draw grid
    uint16_t grid_tick_x = i * timing_mid;
    if (is_within_fov(grid_tick_x)) {
      uint8_t grid_fov_x =
          draw_x + fov_pixels_per_tick * (grid_tick_x - fov_offset);

      for (uint8_t k = 0; k < fov_notes; k += 1) {
        // draw crisscross
        // if ((fov_y + k + i) % 2 == 0) { oled_display.drawPixel(
        // grid_fov_x, (k * (fov_h / fov_notes)), WHITE); }
        oled_display.drawPixel(grid_fov_x, draw_y + (k * (h)), WHITE);

        if (i % 16 == 0) {
          oled_display.drawPixel(grid_fov_x, draw_y + (k * h) + (h / 2), WHITE);
        }
      }
    }

    for (; ev_idx != ev_end; ++ev_idx) {
      auto &ev = active_track.events[ev_idx];
      int note_val = ev.event_value;
      // Check if note is note_on and is visible within fov vertical
      // range.
      if (ev.is_lock || !ev.event_on) {
        continue;
      }

      uint16_t note_off_idx = ev_idx;
      uint8_t j =
          active_track.search_note_off(note_val, i, note_off_idx, ev_end);
      if (note_off_idx == 0xFFFF) {
        note_off_idx = ev_idx;
      }
      auto &ev_j = active_track.events[note_off_idx];

      uint16_t note_start = i * timing_mid + ev.micro_timing - timing_mid;
      uint16_t note_end = j * timing_mid + ev_j.micro_timing - timing_mid;

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

        uint8_t note_fov_y = fov_h - ((note_val - fov_y) * (fov_h / fov_notes));
        // Draw vertical projection
        uint8_t proj_y = 255;
        if (note_val > fov_y + fov_notes) {
          //&&     (cur_y == fov_y + fov_notes - 1)) {
          proj_y = 0;
        } else if (note_val <= fov_y) {
          // && (cur_y == fov_y)) {
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
        if ((note_val > fov_y) && (note_val <= fov_y + fov_notes)) {
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
            oled_display.drawRect(note_fov_start + draw_x, draw_y + note_fov_y,
                                  note_fov_end - note_fov_start,
                                  (fov_h / fov_notes), WHITE);
          }
        }
      }
    }
  }
  // Draw interactive cursor
  uint8_t fov_cur_y = fov_h - ((cur_y - fov_y) * ((fov_h) / fov_notes));
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
  constexpr uint16_t width = pidx_w * 4 + 3;
  uint16_t pattern_end = active_track.length * timing_mid;
  uint16_t cur_tick_x =
      active_track.step_count * timing_mid + active_track.mod12_counter;

  oled_display.drawRect(pidx_x0, pidx_y, width, pidx_h, WHITE);

  // viewport is [fov_offset, fov_offset+fov_length] out of [0,
  // pattern_end]

  uint16_t s = fov_offset * (width - 1) / pattern_end;
  uint16_t w = fov_length * (width - 2) / pattern_end;
  uint16_t p = cur_tick_x * (width - 1) / pattern_end;
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
  if (pianoroll_mode == 0) {
    seq_menu_page.menu.enable_entry(SEQ_MENU_VEL, true);
    seq_menu_page.menu.enable_entry(SEQ_MENU_PARAMSELECT, false);
  } else {
    seq_menu_page.menu.enable_entry(SEQ_MENU_VEL, false);
    seq_menu_page.menu.enable_entry(SEQ_MENU_PARAMSELECT, true);
  }
  SeqPage::loop();

  if (seq_param1.hasChanged()) {
    // Horizontal translation
    int16_t diff = seq_param1.cur - seq_param1.old;

    uint8_t w = cur_w;
    if (pianoroll_mode >= 1) {
      w = 3;
    }
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
      if (cur_x >= fov_offset + fov_length - w) {
        if (fov_offset + fov_length + diff < roll_length) {
          fov_offset += diff;
          cur_x = fov_offset + fov_length - w;
        }
      } else {
        cur_x += diff;
        if (cur_x > fov_offset + fov_length - w) {
          cur_x = fov_offset + fov_length - w;
        }
      }
    }

    seq_param1.cur = 64;
    seq_param1.old = 64;
  }

  if (seq_param2.hasChanged()) {
    // Vertical translation
    int16_t diff = seq_param2.old - seq_param2.cur; // reverse dir for sanity.
    if (pianoroll_mode >= 1) {
      lock_cur_y = limit_value(lock_cur_y, diff, 0, 127);
    }

    else {
      if (diff < 0) {
        scroll_dir = false;
        if (cur_y <= fov_y + 1) {
          fov_y += diff;
          if (fov_y < 1) {
            fov_y = 1;
          }
          cur_y = fov_y + 1;
        } else {
          cur_y += diff;
          if (cur_y < fov_y + 1) {
            cur_y = fov_y + 1;
          }
        }
      } else {
        scroll_dir = true;
        if (cur_y >= fov_y + fov_notes) {
          fov_y += diff;
          if (fov_y + fov_notes > 127) {
            fov_y = 127 - fov_notes;
          }
          cur_y = fov_y + fov_notes;
        } else {
          cur_y += diff;
          if (cur_y >= fov_y + fov_notes) {
            cur_y = fov_y + fov_notes;
          }
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
  if (recording && MidiClock.state == 2) {
    if (!redisplay) {
      return;
    }
  }

#ifdef EXT_TRACKS
  oled_display.clearDisplay();

  auto &active_track = mcl_seq.ext_tracks[last_ext_track];

  draw_viewport_minimap();
  if (pianoroll_mode == 0) {
    draw_pianoroll();
  } else {
    draw_lockeditor();
  }

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
                .notes[note_idx][note_interface.last_note + page_select *
  16]; if (c_note != 0) { uint8_t note = abs(c_note); DEBUG_DUMP(c_note);
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

  SeqPage::display();
  // Draw vertical keyboard
  oled_display.fillRect(draw_x - keyboard_w - 1, 0, keyboard_w + 1, fov_h,
                        BLACK);
  if (pianoroll_mode == 0) {
    const uint16_t chromatic = 0b0000010101001010;
    for (uint8_t k = 0; k < fov_notes; k++) {
      uint8_t scale_pos =
          (fov_y + fov_notes - k) - (((fov_y + fov_notes - k) / 12) * 12);
      if (!IS_BIT_SET16(chromatic, scale_pos)) {
        oled_display.fillRect(draw_x - keyboard_w,
                              draw_y + k * (fov_h / fov_notes) + 1,
                              keyboard_w + 1, (fov_h / fov_notes) - 1, WHITE);
      } else {
        //    oled_display.fillRect(draw_x - keyboard_w,
        //                            draw_y + k * (fov_h / fov_notes) + 1,
        //                            keyboard_w + 1, (fov_h / fov_notes) -
        //                            1, BLACK);
        oled_display.fillRect(draw_x, draw_y + k * (fov_h / fov_notes), 1,
                              (fov_h / fov_notes) + 1, WHITE);
      }
    }
  }
  // oled_display.fillRect(draw_x, 0, 1 , fov_h, WHITE);

  oled_display.display();
#endif
}
#endif

void SeqExtStepPage::enter_notes() {
  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  for (uint8_t n = 0; n < NUM_NOTES_ON; n++) {
    if (active_track.notes_on[n].value == 255)
      continue;
    if (!active_track.del_note(cur_x, cur_w, active_track.notes_on[n].value)) {
      active_track.add_note(cur_x, cur_w, active_track.notes_on[n].value,
                            velocity);
    }
  }
}

bool SeqExtStepPage::handleEvent(gui_event_t *event) {

  if (note_interface.is_event(event)) {
    uint8_t mask = event->mask;
    uint8_t port = event->port;
    uint8_t device = midi_active_peering.get_device(port)->id;
    uint8_t track = event->source - 128;

    if (device == DEVICE_A4) {
      return true;
    }

    trig_interface.send_md_leds(TRIGLED_EXCLUSIVE);

    if (mask == EVENT_BUTTON_PRESSED) {
      ++x_notes_down;
      if (x_notes_down == 1) {
        cur_x = fov_offset + (float)(fov_length / 16) * (float)track;
        note_interface.last_note = track;
      } else {
        cur_w = fov_offset + (float)(fov_length / 16) * (float)track - cur_x;
        if (cur_w < 0) {
          cur_x += cur_w;
          cur_w = -cur_w;
        } else if (cur_w < cur_w_min) {
          cur_w = cur_w_min;
        }
      }
    } else if (mask == EVENT_BUTTON_RELEASED) {
      --x_notes_down;
      if (x_notes_down == 0) {
        enter_notes();
      }
      return true;
    }
    return true;
  }

  auto &active_track = mcl_seq.ext_tracks[last_ext_track];
  if (EVENT_RELEASED(event, Buttons.BUTTON4)) {
    if (pianoroll_mode >= 1) {
      uint8_t timing_mid = active_track.get_timing_mid();

      uint8_t step = (cur_x / timing_mid);
      uint8_t utiming = timing_mid + cur_x - (step * timing_mid);

      active_track.set_track_locks(
          step, utiming, active_track.locks_params[pianoroll_mode - 1] - 1,
          lock_cur_y);
      return true;
    }
    if (!recording) {
      if (active_track.notes_on_count > 1) {
        enter_notes();
      } else {

        if (!active_track.del_note(cur_x, cur_w, cur_y)) {
          active_track.add_note(cur_x, cur_w, cur_y, velocity);
        }
      }
      return true;
    } else {
      switch (last_rec_event) {
      case REC_EVENT_TRIG:
        if (BUTTON_DOWN(Buttons.BUTTON3)) {
          oled_display.textbox("CLEAR ", "TRACKS");
          for (uint8_t n = 0; n < NUM_EXT_TRACKS; ++n) {
            mcl_seq.ext_tracks[n].clear_track();
          }
        } else {
          oled_display.textbox("CLEAR ", "TRACK");
          active_track.clear_track();
        }
        break;
      case REC_EVENT_CC:
        // TODO
        // oled_display.textbox("CLEAR ", "LOCK");
        break;
      }
      queue_redraw();
      return true;
    }
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON3)) {
    if (pianoroll_mode >= 1) {
      if (param_select == 128) {
        active_track.locks_params[SeqPage::pianoroll_mode - 1] = 0;
      } else {
        active_track.locks_params[SeqPage::pianoroll_mode - 1] =
            param_select + 1;
      }
    }
  }
  if (EVENT_RELEASED(event, Buttons.BUTTON1)) {
    recording = !recording;
    oled_display.textbox("REC", "");
    queue_redraw();
    return true;
  }

  if (!recording || MidiClock.state != 2 ||
      EVENT_PRESSED(event, Buttons.BUTTON2)) {
    if (pianoroll_mode >= 1) {
      if (active_track.locks_params[pianoroll_mode - 1] == 0) {
        param_select = 128;
      } else {
        param_select = active_track.locks_params[pianoroll_mode - 1] - 1;
      }
    }
    if (SeqPage::handleEvent(event)) {
      if (show_seq_menu) {
        redisplay = true;
        return true;
      }
    }
    return true;
  }
}

void SeqExtStepMidiEvents::onControlChangeCallback_Midi2(uint8_t *msg) {
  uint8_t channel = MIDI_VOICE_CHANNEL(msg[0]);
  uint8_t param = msg[1];
  uint8_t value = msg[2];
  DEBUG_DUMP("blah");
  if (SeqPage::recording) {
    DEBUG_DUMP("Record... cc");
    if (channel < NUM_EXT_TRACKS) {
      mcl_seq.ext_tracks[channel].record_track_locks(param, value);
      mcl_seq.ext_tracks[channel].update_param(param, value);
    }
  }
}

void SeqExtStepMidiEvents::onNoteOnCallback_Midi2(uint8_t *msg) {
  // Step edit for ExtSeq
  // For each incoming note, check to see if note interface has any steps
  // selected For selected steps record notes.
#ifdef EXT_TRACKS
  uint8_t channel = MIDI_VOICE_CHANNEL(msg[0]);
  uint8_t note_num = msg[1];
  DEBUG_PRINT(F("note on midi2 ext, "));
  DEBUG_DUMP(channel);

  if (channel < mcl_seq.num_ext_tracks) {

    auto fov_offset = seq_extstep_page.fov_offset;
    auto cur_x = seq_extstep_page.cur_x;
    auto fov_y = seq_extstep_page.fov_y;
    auto cur_y = seq_ptc_page.seq_ext_pitch(note_num);
    auto cur_w = seq_extstep_page.cur_w;

    if (fov_y >= cur_y && cur_y != 0) {
      fov_y = cur_y - 1;
    } else if (fov_y + SeqExtStepPage::fov_notes <= cur_y) {
      fov_y = cur_y - SeqExtStepPage::fov_notes;
    }

    last_ext_track = channel;
    seq_extstep_page.config_encoders();

    seq_extstep_page.fov_offset = fov_offset;
    seq_extstep_page.cur_x = cur_x;
    seq_extstep_page.fov_y = fov_y;
    seq_extstep_page.cur_y = cur_y;
    seq_extstep_page.cur_w = cur_w;

    mcl_seq.ext_tracks[channel].record_track_noteon(cur_y, msg[2]);
    if (seq_extstep_page.x_notes_down > 0) {
      seq_extstep_page.enter_notes();
    }
    if ((MidiClock.state != 2) || (seq_extstep_page.recording)) {
      seq_extstep_page.last_rec_event = REC_EVENT_TRIG;
      uint8_t vel = SeqPage::velocity;
      if (seq_extstep_page.recording) {
        vel = msg[2];
      }
      mcl_seq.ext_tracks[channel].note_on(cur_y, vel);
    }
  }
#endif
}

void SeqExtStepMidiEvents::onNoteOffCallback_Midi2(uint8_t *msg) {
#ifdef EXT_TRACKS
  uint8_t channel = MIDI_VOICE_CHANNEL(msg[0]);
  uint8_t note_num = msg[1];
  uint8_t pitch = seq_ptc_page.seq_ext_pitch(note_num);

  if (channel < mcl_seq.num_ext_tracks) {

    if (MidiClock.state != 2) {
      mcl_seq.ext_tracks[channel].note_off(pitch);
    }
    if ((seq_extstep_page.recording) && (MidiClock.state == 2)) {
      mcl_seq.ext_tracks[channel].note_off(pitch);
      mcl_seq.ext_tracks[channel].record_track_noteoff(pitch);
    } else {
      mcl_seq.ext_tracks[channel].remove_notes_on(pitch);
    }
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
  Midi2.addOnControlChangeCallback(this,
                                   (midi_callback_ptr_t)&SeqExtStepMidiEvents::
                                       onControlChangeCallback_Midi2);
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
  Midi2.removeOnControlChangeCallback(
      this, (midi_callback_ptr_t)&SeqExtStepMidiEvents::
                onControlChangeCallback_Midi2);
  state = false;
}
