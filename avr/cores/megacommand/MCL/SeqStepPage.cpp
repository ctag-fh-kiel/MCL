#include "SeqStepPage.h"

void SeqStepPage::setup() { SeqPage::setup(); }
void SeqStepPage::init() {
  collect_trigs = true;

  encoders[1]->max = 13;
  encoders[2]->max = 23;
  encoders[2]->min = 1;
  encoders[2]->cur = 12;
  encoders[3]->max = 64;
  encoders[4]->max = 16;
  encoders[3]->cur = PatternLengths[last_md_track];

  curpage = SEQ_STEP_PAGE;
}
bool SeqStepPage::display() {
  GUI.put_string_at(0, "                ");

  const char *str1 = getMachineNameShort(MD.kit.models[last_md_track], 1);
  const char *str2 = getMachineNameShort(MD.kit.models[last_md_track], 2);

  char c[3] = "--";

  if (encoders[1]->getValue() == 0) {
    GUI.put_string_at(0, "L1");

  } else if (encoders[1]->getValue() <= 8) {
    GUI.put_string_at(0, "L");

    GUI.put_value_at1(1, encoders[1]->getValue());

  } else {
    GUI.put_string_at(0, "P");
    uint8_t prob[5] = {1, 2, 5, 7, 9};
    GUI.put_value_at1(1, prob[encoders[1]->getValue() - 9]);
  }

  if (encoders[2]->getValue() == 0) {
    GUI.put_string_at(2, "--");
  } else if ((encoders[2]->getValue() < 12) && (encoders[2]->getValue() != 0)) {
    GUI.put_string_at(2, "-");
    GUI.put_value_at1(3, 12 - encoders[2]->getValue());

  } else {
    GUI.put_string_at(2, "+");
    GUI.put_value_at1(3, encoders[2]->getValue() - 12);
  }

  GUI.put_p_string_at(10, str1);
  GUI.put_p_string_at(12, str2);
  GUI.put_value_at(6, encoders[3]->getValue());
  GUI.put_value_at1(15, seq_page_select + 1);
  // GUI.put_value_at2(7, encoders[3]->getValue());
  draw_patternmask((seq_page_select * 16), DEVICE_MD);
}

bool SeqStepPage::handleEvent(gui_event_t *event) {
  if (note_interface.is_event(event)) {
    uint8_t mask = event->mask;
    uint8_t device = midi_active_peering.get_device(port);

    if (device == MD_DEVICE) {
      uint8_t track = event->source - 128;
    }
    if (device == A4_DEVICE) {
      uint8_t track = event->source - 128 - 16;
    }

    if (event->mask == EVENT_BUTTON_PRESSED) {

      if (device == A4_DEVICE) {
        last_Ext_Track = track;
        GUI.setPage(&seq_extstep_page);
      }
      if ((note_num + (seq_page_select * 16)) >= PatternLengths[grid.cur_col]) {
        notes[note_num] = 0;
        return;
      }

      encoders[2]->max = 23;
      int8_t utiming =
          timing[grid.cur_col][(note_num + (seq_page_select * 16))]; // upper
      uint8_t condition =
          conditional[grid.cur_col]
                     [(note_num + (seq_page_select * 16))]; // lower

      // Cond
      encoders[1]->cur = condition;
      // Micro
      if (utiming == 0) {
        utiming = 12;
      }
      encoders[2]->cur = utiming;
    }
    if (event->mask == EVENT_BUTTON_RELEASED) {
    }
    return true;
  }
  if (EVENT_PRESSED(event, Buttons.ENCODER1) {
    md_exploit.off();
    GUI.setPage(&grid_page);
    curpage = GRID_PAGE;
    return true;

  }

if ((EVENT_PRESSED(event, Buttons.BUTTON1) && BUTTON_DOWN(Buttons.BUTTON4)) ||
    (EVENT_PRESSED(event, Buttons.BUTTON4) && BUTTON_DOWN(Buttons.BUTTON3))) {
    for (uint8_t n = 0; n < 16; n++) {
      clear_seq_track(n);
    }
    return true;
}
  if (EVENT_RELEASED(event, Buttons.BUTTON4)) {
    clear_seq_track(last_md_track);
    return true;
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON1))  {
    load_seq_extstep_page(last_Ext_track);

    return true;
    /*

      return true;
    */


  }
  if (SeqStep::handleEvent(event)) {
    return true;
  }


return false;
}