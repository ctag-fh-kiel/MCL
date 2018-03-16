#include "GridPage.h"

void GridPage::displayScroll(uint8_t i) {
  if (encoders[i] != NULL) {

    if (((encoders[0]->getValue() + i + 1) % 4) == 0) {
      char strn[2] = "I";
      strn[0] = (char)001;
      //           strn[0] = (char) 219;
      GUI.setLine(GUI.LINE1);

      GUI.put_string_at_noterminator((2 + (i * 3)), strn);

      GUI.setLine(GUI.LINE2);
      GUI.put_string_at_noterminator((2 + (i * 3)), strn);
    }

    else {
      char strn_scroll[2] = "|";
      GUI.setLine(GUI.LINE1);

      GUI.put_string_at_noterminator((2 + (i * 3)), strn_scroll);

      GUI.setLine(GUI.LINE2);
      GUI.put_string_at_noterminator((2 + (i * 3)), strn_scroll);
    }
  }
}

void GridPage::encoder_fx_handle(Encoder *enc) {
  GridEncoder *mdEnc = (GridEncoder *)enc;

  /*Scale delay feedback for safe ranges*/

  if (mdEnc->fxparam == MD_ECHO_FB) {
    if (mdEnc->getValue() > 68) {
      mdEnc->setValue(68);
    }

  }
  USE_LOCK();
  SET_LOCK();
  MD.sendFXParam(mdEnc->fxparam, mdEnc->getValue(), mdEnc->effect);
  CLEAR_LOCK();
}
void GridPage::load_slot_models() {

  DEBUG_PRINT_FN(x);

  DEBUG_PRINT("Row: ");
  DEBUG_PRINTLN(encoders[2]->getValue());
  for (uint8_t i = 0; i < 22; i++) {
    grid_models[i] =
       grid.getGridModel(i, encoders[2]->getValue(), true, (A4Track *)&track_bufx);
    DEBUG_PRINT("Slot: ");
    DEBUG_PRINT(i);
    DEBUG_PRINT(" Model: ");
    DEBUG_PRINTLN(grid_models[i]);
    if (i == 0) {
      if (temptrack.active != EMPTY_TRACK_TYPE) {
        for (uint8_t c = 0; c < 17; c++) {
          currentkitName[c] = temptrack.kitName[c];
        }
      } else {
        for (uint8_t c = 0; c < 17; c++) {
          currentkitName[c] = ' ';
        }
      }
    }
  }
}

void GridPage::tick_frames() {
  uint16_t current_clock = slowclock;

  frames += 1;
  if (clock_diff(frames_startclock, current_clock) >= 400) {
    frames_startclock = slowclock;
    frames = 0;
  }
  if (clock_diff(frames_startclock, current_clock) >= 250) {
    frames_fps = frames;
    // frames_fps = ((frames + frames_fps)/ 2);
    frames = 0;
    frames_startclock = slowclock;
  }
}

void toggle_fx1() {
  dispeffect = 1;
  if (encoders[3]->effect == MD_FX_REV) {
    fx_dc = encoders[3]->getValue();
    encoders[3]->setValue(fx_tm);

    encoders[3]->effect = MD_FX_ECHO;
    encoders[3]->fxparam = MD_ECHO_TIME;
  }
  else {
    fx_tm = encoders[3]->getValue();
    encoders[3]->setValue(fx_dc);
    encoders[3]->effect = MD_FX_REV;
    encoders[3]->fxparam = MD_REV_DEC;
  }
}

void toggle_fx2() {
 dispeffect = 1;

  if (encoder[4]->.effect == MD_FX_REV) {
    fx_lv = encoder[4]->.getValue();
    encoder[4]->.setValue(fx_fb);
    encoder[4]->.effect = MD_FX_ECHO;
    encoder[4]->.fxparam = MD_ECHO_FB;
  }

  else {
    fx_fb = encoder[4]->.getValue();
    encoder[4]->.setValue(fx_lv);
    encoder[4]->.effect = MD_FX_REV;
    encoder[4]->.fxparam = MD_REV_LEV;
  }
}


void GridPage::display() {
  tick_frames();
  // GUI.put_value16_at(0, MidiClock.div192th_counter);
  //  GUI.put_value16_at(5, MidiClock.div96th_counter);
  //   GUI.put_value_at(12, (uint8_t)MidiClock.div192th_time);
  // if (MidiClock.mod12_counter > 10) { GUI.put_value16_at(0,
  // MidiClock.mod12_counter); } GUI.put_value16_at(5, MidiClock.mod6_counter);

  //  return;

  row_name_offset += (float)1 / frames_fps * 1.5;

  if (BUTTON_DOWN(Buttons.BUTTON3) && (encoders[3]->hasChanged())) {
    toggle_fx1();
  }

  if (BUTTON_DOWN(Buttons.BUTTON3) && (encoders[4]->hasChanged())) {
    toggle_fx2();
  }
  uint8_t display_name = 0;
  if (slowclock < grid_lastclock) {
    grid_lastclock = 0xFFFF - grid_lastclock;
  }
  if (!reload_slot_models) {
    load_slot_models();
    reload_slot_models = true;
  }

  if (clock_diff(grid_lastclock, slowclock) < GUI_NAME_TIMEOUT) {
    display_name = 1;
    if (clock_diff(cfg_save_lastclock, slowclock) > GUI_NAME_TIMEOUT) {
      cfg.cur_col = encoders[1]->cur;
      cfg.cur_row = encoders[2]->cur;
      cfg.write_cfg();
    }
  } else {

    /*For each of the 4 encoder objects, ie 4 Grids to be displayed on screen*/
    for (uint8_t i = 0; i < 4; i++) {

      /*Display the encoder, ie the Grid i*/
      encoders[i]->displayAt(i);
      /*Display the scroll animation. (Scroll animation draws a || at every 4
       * Grids in a row, making it easier to separate and visualise the track
       * Grids)*/
      displayScroll(i);
      /*value = the grid position of the left most Grid (displayed on screen).*/
    }
  }
  // int value = encoders[0]->getValue() + (encoders[1]->getValue() * 16);

  /*If the effect encoders have been changed, then set a switch to indicate that
   * the effects values should be displayed*/

  if (encoders[3]->hasChanged() || encoders[4]->hasChanged()) {
    dispeffect = 1;
  }

  /*If the grid encoders have been changed, set a switch to indicate that the
   * row/col values should be displayed*/
  if (encoders[1]->hasChanged() || encoders[2]->hasChanged()) {
    dispeffect = 0;
  }

  if (dispeffect == 1) {
    GUI.setLine(GUI.LINE1);
    /*Displays the kit name of the left most Grid on the first line at position
     * 12*/
    if (encoders[3]->effect == MD_FX_ECHO) {
      GUI.put_string_at(12, "TM");
    } else {
      GUI.put_string_at(12, "DC");
    }

    if (encoders[4]->effect == MD_FX_ECHO) {
      GUI.put_string_at(14, "FB");
    } else {
      GUI.put_string_at(14, "LV");
    }

    GUI.setLine(GUI.LINE2);
    /*Displays the value of the current Row on the screen.*/
    GUI.put_value_at2(12, (encoders[2]->getValue()));
    GUI.put_value_at2(14, (encoders[3]->getValue()));
    // mdEnc1->dispnow = 0;
    //  mdEnc2->dispnow = 0;

  } else {
    GUI.setLine(GUI.LINE1);
    /*Displays the kit name of the left most Grid on the first line at position
     * 12*/
    if (display_name == 1) {
      GUI.put_string_at(0, "                ");

      GUI.put_string_at(0, currentkitName);
      GUI.setLine(GUI.LINE2);

      GUI.put_string_at(0, "                ");
      // temptrack.patternOrigPosition;
      char str[5];

      // if (gridio_encoders[1]->getValue() < 8) {
      // if (temptrack.active != EMPTY_TRACK_TYPE) {
      //   MD.getPatternName(temptrack.patternOrigPosition , str);
      // }
      // }
    } else {

      GUI.put_string_at(12, getTrackKit(encoders[0]->getValue(),
                                        encoders[1]->getValue(), false, true));
    }
    GUI.setLine(GUI.LINE2);

    /*Displays the value of the current Row on the screen.*/
    GUI.put_value_at2(12, (encoders[0]->getValue()));
    /*Displays the value of the current Column on the screen.*/
    GUI.put_value_at2(14, (encoders[1]->getValue()));
  }
}

bool GridPage::handleEvent(gui_event_t *event) {
  if (note_interface.is_event(event)) {

    return true;
  }

  if (BUTTON_RELEASED(Buttons.BUTTON1) && BUTTON_DOWN(Buttons.BUTTON3)) {
    clear_row(encoders[2]->getValue());
    reload_slot_models = 0;
    return true;
  }
  // TRACK READ PAGE

  if (EVENT_RELEASED(event, Buttons.BUTTON1)) {
    grid_save_page->setup = false;
    GUI.setPage(&grid_save_page);

    return true;
  }

  // TRACK WRITE PAGE

  if (EVENT_RELEASED(event, Buttons.BUTTON4)) {
    grid_write_page->setup = false;
    GUI.setPage(&grid_write_page);

    return true;
  }

  if (BUTTON_DOWN(Buttons.BUTTON2) && BUTTON_DOWN(Buttons.BUTTON4)) {
    setLed();
    int curtrack = MD.getCurrentTrack(CALLBACK_TIMEOUT);

    encoders[1]->cur = curtrack;

    clearLed();
    return true;
  }

  if (EVENT_PRESSED(event, Buttons.BUTTON2)) {
    mixer_page->setup = false;
    GUI.setPage(&mixer_page);
    //   draw_levels();
  }
  /*IF button1 and encoder buttons are pressed, store current track selected on
   * MD into the corresponding Grid*/

  //  if (BUTTON_PRESSED(Buttons.BUTTON3)) {
  //      MD.getBlockingGlobal(1);
  //          MD.global.baseChannel = 9;
  //        //global_new.baseChannel;
  //          for (int i=0; i < 16; i++) {
  //            MD.muteTrack(i,true);
  //          }
  //           setLevel(8,100);
  //  }

  if (BUTTON_PRESSED(Buttons.ENCODER1)) {
    if (BUTTON_DOWN(Buttons.BUTTON3)) {
      seq_euc_page->setup = false;
      GUI.setPage(&seq_euc_page);
    } else {
      seq_step_page->setup = false;
      GUI.setPage(&seq_step_page);
    }

    return true;
  }
  if (BUTTON_PRESSED(Buttons.ENCODER2)) {
    seq_rtrk_page->setup = false;
    GUI.setPage(&seq_rtrk_page);

    return true;
  }
  if (BUTTON_PRESSED(Buttons.ENCODER3)) {
    seq_param_a_page->setup = false;
    GUI.setPage(&seq_param_a_page);

    return true;
  }
  if (BUTTON_PRESSED(Buttons.ENCODER4)) {
    seq_ptc_page->setup = false;
    GUI.setPage(&seq_ptc_page);

    return true;
  }
}

if ((EVENT_PRESSED(event, Buttons.BUTTON1) && BUTTON_DOWN(Buttons.BUTTON4)) ||
    (EVENT_PRESSED(event, Buttons.BUTTON4) && BUTTON_DOWN(Buttons.BUTTON1))) {

  system_page->setup = false;
  GUI.setPage(&system_page);

  curpage = SYSTEM_PAGE;

  return true;
}

return false;
}

void GridPage::loop() { midi_active_peering.check(); }

bool GridPage::setup() {
  frames_startclock = slowclock;

  encoders[2]->handler = encoder_encoders[2]->handle;
  encoders[3]->handler = encoder_fx_handle;
  encoders[3]->effect = MD_FX_ECHO;
  encoders[3]->fxparam = MD_ECHO_TIME;
  encoders[4]->handler = encoder_fx_handle;
  encoders[4]->effect = MD_FX_ECHO;
  encoders[4]->fxparam = MD_ECHO_FB;
}
