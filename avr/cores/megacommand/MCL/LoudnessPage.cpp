#include "LoudnessPage.h"
#include "MCL.h"
#include "MidiSDS.h"

void LoudnessPage::setup() { DEBUG_PRINT_FN(); }

void LoudnessPage::init() {
  DEBUG_PRINT_FN();

  trig_interface.off();
#ifdef OLED_DISPLAY
  // classic_display = false;
  oled_display.clearDisplay();
  oled_display.setFont();
#endif
  encoders[0]->cur = 100;
}

void LoudnessPage::cleanup() {
  //  md_exploit.off();
#ifdef OLED_DISPLAY
  oled_display.clearDisplay();
#endif
}
void LoudnessPage::scale_vol(float inc) {
  DEBUG_PRINT_FN();
  EmptyTrack empty_track;

  MDTrack *md_track = (MDTrack *)&empty_track;

  grid_page.prepare();

  uint8_t seq_mute_states[NUM_MD_TRACKS];
  for (uint8_t a = 0; a < NUM_MD_TRACKS; a++) {
    seq_mute_states[a] = mcl_seq.md_tracks[a].mute_state;
  }
  for (uint8_t n = 0; n < 16; n++) {
    md_track->get_machine_from_kit(n);
    memcpy(&(md_track->seq_data), &mcl_seq.md_tracks[n],
           sizeof(md_track->seq_data));
    md_track->scale_vol(inc);
    memcpy(&mcl_seq.md_tracks[n], &(md_track->seq_data),
           sizeof(md_track->seq_data));
    mcl_seq.md_tracks[n].mute_state = SEQ_MUTE_ON;
    //    md_track->place_track_in_pattern(n, n, &MD.pattern);
    md_track->place_track_in_kit(n, n, &MD.kit);
    MD.setMachine(n, &(md_track->machine));
  }

  for (uint8_t a = 0; a < NUM_MD_TRACKS; a++) {
    mcl_seq.md_tracks[a].mute_state = seq_mute_states[a];
  }
}

void LoudnessPage::display() {

#ifdef OLED_DISPLAY
    oled_display.clearDisplay();
#endif
  GUI.setLine(GUI.LINE1);
  uint8_t x;
  // GUI.put_string_at(12,"Loudness");
  GUI.put_string_at(0, "LOUDNESS ");
  uint8_t msb = encoders[0]->cur / 100;
  uint8_t mantissa = encoders[0]->cur % 100;
  GUI.setLine(GUI.LINE2);
  GUI.put_string_at(0, "Gain:");
  GUI.put_value_at(6, encoders[0]->cur);
  GUI.put_string_at(9, "%");
  /*
  GUI.put_value_at1(8,msb);
  GUI.put_string_at(9,".");
  GUI.put_value_at1(10,mantissa / 10);
  GUI.put_value_at1(11,mantissa % 10);
*/
#ifdef OLED_DISPLAY
#endif
}

bool LoudnessPage::handleEvent(gui_event_t *event) {
  if (note_interface.is_event(event)) {
    uint8_t track = event->source - 128;
    if (midi_active_peering.get_device(event->port) != DEVICE_MD) {
      return true;
    }
  }
  if (event->mask == EVENT_BUTTON_RELEASED) {
    return true;
  }
  if (EVENT_PRESSED(event, Buttons.BUTTON4)) {

    scale_vol((float)encoders[0]->cur / (float)100);
    encoders[0]->cur = 100;
    return true;
  }
//  if (EVENT_PRESSED(event, Buttons.BUTTON4)) {
//    float inc = check_loudness();
 //   encoders[0]->cur = inc * 100;
//  }

  if (EVENT_PRESSED(event, Buttons.BUTTON2)) {
    GUI.setPage(&page_select_page);
    return true;
  }

  return false;
}

MCLEncoder loudness_param1(1, 255, 2);
LoudnessPage loudness_page(&loudness_param1);
