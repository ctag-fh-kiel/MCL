#include "SeqRlckPage.h"

void SeqRlckPage::setup() {
  SeqPage::setup();
  collect_trigs = false;

  encoders[1]->max = 4;
  encoders[2]->max = 64;
  encoders[3]->max = 64;
  encoders[4]->max = 11;
  encoders[3]->cur = PatternLengths[last_md_track];

  curpage = SEQ_RTRK_PAGE;
}

bool SeqRlckPage::display() {
  GUI.setLine(GUI.LINE1);
  GUI.put_value_at1(15, seq_page_select + 1);

  GUI.put_string_at(0, "RLCK");

  const char *str1 = getMachineNameShort(MD.kit.models[cur_col], 1);
  const char *str2 = getMachineNameShort(MD.kit.models[cur_col], 2);
  if (cur_col < 16) {
    GUI.put_p_string_at(9, str1);
    GUI.put_p_string_at(11, str2);
    GUI.put_value_at(5, encoders[3]->getValue());
  } else {
    GUI.put_value_at(5, (encoders[3]->getValue() /
                         (2 / ExtPatternResolution[last_Ext_track])));
    if (Analog4.connected) {
      GUI.put_string_at(9, "A4T");
    } else {
      GUI.put_string_at(9, "MID");
    }
    GUI.put_value_at1(12, cur_col - 16 + 1);
  }

  draw_lockmask(seq_page_select * 16);
}
bool SeqRlckPage::handleEvent(gui_event_t *event) {

  if (note_interface.is_event(event)) {
    return true;
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON1)) {
    md_exploit.on();
    collect_trigs = false;
    curpage = SEQ_RTRK_PAGE;
    GUI.setPage(&seq_rtrk_page);
    return true;
  }

  if ((EVENT_PRESSED(evnt, Buttons.BUTTON1) && BUTTON_DOWN(Buttons.BUTTON4)) ||
      (EVENT_PRESSED(evnt, Buttons.BUTTON4) && BUTTON_DOWN(Buttons.BUTTON3))) {

    for (uint8_t n = 0; n < 16; n++) {
      clear_seq_locks(n);
    }
    return true;
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON4)) {
    clear_seq_locks(cur_col);
    return true;
  }

  if (SeqPage::handleEvent(event)) {
    return true;
  }

  return false;
}
