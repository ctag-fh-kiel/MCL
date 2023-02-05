#include "MCL_impl.h"
#include "ResourceManager.h"

void GridPage::init() {
  DEBUG_PRINTLN("Grid page init");
  encoders[0] = &param1;
  encoders[1] = &param2;
  ((MCLEncoder *)encoders[0])->max = getWidth() - 1;
  show_slot_menu = false;
  reload_slot_models = false;
  // Edge case, prevent R.Clear being called if we're outside of GridPage
  if (GUI.currentPage() != &grid_page) {
    return;
  }
  trig_interface.off();
  load_slot_models();
  oled_display.clearDisplay();
  R.Clear();
  R.use_machine_names_short();
}

void GridPage::setup() {
  encoders[0]->cur = encoders[0]->old = mcl_cfg.col;
  encoders[1]->cur = encoders[1]->old = mcl_cfg.row;
  cur_col = mcl_cfg.cur_col;
  cur_row = mcl_cfg.cur_row;
  memset(active_slots, SLOT_DISABLED, sizeof(active_slots));
  /*
  cur_col = 0;
  if (mcl_cfg.row < MAX_VISIBLE_ROWS) { cur_row = mcl_cfg.row; }
  else { cur_row = MAX_VISIBLE_ROWS - 1; }
  */
}

void GridPage::cleanup() {
  oled_display.setFont();
  oled_display.setTextColor(WHITE, BLACK);
  bank_popup = 0;
}

void GridPage::load_row(uint8_t n, uint8_t row) {
  if (row >= GRID_LENGTH) { return; }
  if (IS_BIT_CLEAR16(grid_page.bank_popup_loadmask, n)) {
    grid_load_page.group_load(row);
    SET_BIT16(grid_page.bank_popup_loadmask, n);
  }
}

void GridPage::jump_to_row(uint8_t row) {
  //  uint8_t y = (row / MAX_VISIBLE_ROWS) * MAX_VISIBLE_ROWS;
  //  uint8_t r = row - y;
  uint8_t y = row;
  uint8_t r = row - (row / MAX_VISIBLE_ROWS) * MAX_VISIBLE_ROWS;
  param2.cur = y;
  param2.old = y;
  cur_row = r;
  reload_slot_models = false;
  grid_lastclock = slowclock;
  write_cfg = true;
}

void GridPage::set_active_row(uint8_t row) {
  grid_task.last_active_row = row;
  if (bank_popup) {
    send_row_led();
  }
}

void GridPage::send_row_led() {
  uint64_t rows[2] = {0};
  if (row >= GRID_LENGTH) { return; }
  SET_BIT128_P(&rows, grid_task.last_active_row);
  uint16_t *blink_mask = (uint16_t *)&rows[0];
  DEBUG_PRINTLN("Sending row led");
  DEBUG_PRINTLN(blink_mask[grid_page.bank]);
  MD.set_trigleds(blink_mask[grid_page.bank], TRIGLED_EXCLUSIVENDYNAMIC, 1);
}
void GridPage::close_bank_popup() {
  if (bank_popup == 2) {
    MD.draw_close_bank();
  }
  trig_interface.off();
  if (last_page != nullptr) {
    DEBUG_PRINTLN("setting page");
    GUI.setPage(last_page);
  }
  last_page = nullptr;
  bank_popup = 0;
  note_interface.init_notes();
  // Clear blink leds
  MD.set_trigleds(0, TRIGLED_EXCLUSIVENDYNAMIC, 1);
}

void GridPage::loop() {
  int8_t diff, new_val;
  if (show_slot_menu) {

    if (encoders[3]->hasChanged()) {
      if (cur_row + encoders[3]->cur > MAX_VISIBLE_ROWS - 1) {
        load_slot_models();
        reload_slot_models = true;
      }
    }
    grid_slot_page.loop();
    return;
  } else {
  }
  /*
   if (encoders[2]->hasChanged()) {
    diff = encoders[2]->cur - encoders[2]->old;
    if (cur_col + encoders[2]->cur > MAX_VISIBLE_COLS - 1) {
      encoders[0]->cur += diff;
      encoders[0]->old = encoders[0]->cur;
    }
   }
*/
  if (encoders[0]->hasChanged()) {
    diff = encoders[0]->cur - encoders[0]->old;
    new_val = cur_col + diff;
    if (new_val > MAX_VISIBLE_COLS - 1) {
      new_val = MAX_VISIBLE_COLS - 1;
    }
    if (new_val < 0) {
      new_val = 0;
    }
    cur_col = new_val;
  }

  if (encoders[1]->hasChanged()) {
    diff = encoders[1]->cur - encoders[1]->old;
    new_val = cur_row + diff;

    if (new_val > MAX_VISIBLE_ROWS - 1) {
      new_val = MAX_VISIBLE_ROWS - 1;
    }
    if (new_val < 0) {
      new_val = 0;
    }
    // MD.assignMachine(0, encoders[1]->cur);
    cur_row = new_val;
    if ((cur_row == MAX_VISIBLE_ROWS - 1) || (cur_row == 0)) {
      load_slot_models();
    }
    reload_slot_models = true;
    grid_lastclock = slowclock;

    volatile uint8_t *ptr;
    write_cfg = true;
  }
  encoders[2]->cur = 1;
  encoders[3]->cur = 1;
  ((MCLEncoder *)encoders[2])->max = getWidth() - getCol();
  ((MCLEncoder *)encoders[3])->max = GRID_LENGTH - getRow();

  if (!reload_slot_models) {
    load_slot_models();
    reload_slot_models = true;
  }

  if (slowclock < grid_lastclock) {
    grid_lastclock = slowclock + GUI_NAME_TIMEOUT;
  }

  if (clock_diff(grid_lastclock, slowclock) > GUI_NAME_TIMEOUT) {
    ///   DEBUG_DUMP(grid_lastclock);
    //   DEBUG_DUMP(slowclock);
    //   display_name = 1;
    if ((write_cfg) && (MidiClock.state != 2)) {
      mcl_cfg.cur_col = cur_col;
      mcl_cfg.cur_row = cur_row;

      mcl_cfg.col = encoders[0]->cur;
      mcl_cfg.row = encoders[1]->cur;

      mcl_cfg.tempo = MidiClock.get_tempo();
      DEBUG_PRINTLN(F("write cfg"));
      if (MidiClock.state != 2) { mcl_cfg.write_cfg(); }
      grid_lastclock = slowclock;
      write_cfg = false;
      // }
    }
    // display_name = 0;
    row_state_scan();
  }

  if (bank_popup == 2 && clock_diff(bank_popup_lastclock, slowclock) > 800) {
    close_bank_popup();
    return;
  }

}

void GridPage::row_state_scan() {
  if (row_scan) {
    uint8_t old_grid = proj.get_grid();
    GridRowHeader header_tmp;
    row_scan--;

    uint8_t row = GRID_LENGTH - row_scan - 1;
    proj.select_grid(0);
    proj.read_grid_row_header(&header_tmp, row);
    bool state = header_tmp.is_empty();

    proj.select_grid(1);
    proj.read_grid_row_header(&header_tmp, row);
    state |= header_tmp.is_empty();

    update_row_state(row, !state);
    proj.select_grid(old_grid);
  }
}

void GridPage::update_row_state(uint8_t row, bool state) {
  // DEBUG_PRINTLN("updating row state");
  // DEBUG_PRINTLN(row);
  if (state) {
    SET_BIT128_P(row_states, row);
  } else {
    CLEAR_BIT128_P(row_states, row);
  }
}

uint8_t GridPage::getRow() { return param2.cur; }

uint8_t GridPage::getCol() { return param1.cur; }

uint8_t GridPage::getWidth() { return GRID_WIDTH; }

void GridPage::load_slot_models() {
  DEBUG_PRINT_FN();
  uint8_t row_shift = 0;
  if ((cur_row + encoders[3]->cur > MAX_VISIBLE_ROWS - 1)) {
    row_shift = cur_row + encoders[3]->cur - MAX_VISIBLE_ROWS;
  }

  for (uint8_t n = 0; n < MAX_VISIBLE_ROWS; n++) {
    uint8_t row = getRow() - cur_row + n + row_shift;
    if (row >= GRID_LENGTH) { return; }
    proj.read_grid_row_header(&row_headers[n], row);
    update_row_state(row, row_headers[n].active);
  }
}

void GridPage::display_counters() {
  uint8_t y_offset = 8;
  uint8_t x_offset = 20;

  char val[3] = "";
  val[2] = '\0';

  mcl_gui.put_value_at2(MidiClock.bar_counter, val);

  if (val[0] == '0') {
    val[0] = (char)0x60;
  }

  oled_display.setFont(&TomThumb);
  oled_display.setCursor(24, y_offset);
  oled_display.print(val);

  oled_display.print(":");
  oled_display.print(MidiClock.beat_counter);

  if ((mcl_actions.next_transition != (uint16_t)-1) &&
      (MidiClock.bar_counter <= mcl_actions.nearest_bar) &&
      (mcl_actions.nearest_beat > MidiClock.beat_counter ||
       mcl_actions.nearest_bar != MidiClock.bar_counter)) {
    mcl_gui.put_value_at2(mcl_actions.nearest_bar, val);

    if (val[0] == '0') {
      val[0] = (char)0x60;
      if (val[1] == '0') {
        val[1] = (char)0x60;
      }
    }
    oled_display.setCursor(24, y_offset + 8);
    oled_display.print(val);
    oled_display.print(":");
    oled_display.print(mcl_actions.nearest_beat);
  }
}

void GridPage::display_grid_info() {
  uint8_t x_offset = 43;
  uint8_t y_offset = 8;

  oled_display.setFont(&Elektrothic);
  oled_display.setCursor(0, 10);
  oled_display.print(round(MidiClock.get_tempo()));

  display_counters();
  oled_display.setFont(&TomThumb);
  //  oled_display.print(":");
  // oled_display.print(MidiClock.step_counter);

  oled_display.setCursor(22, y_offset + 1 * 8);

  uint8_t tri_x = 10, tri_y = 12;
  if (MidiClock.state == 2) {

    oled_display.drawLine(tri_x, tri_y, tri_x, tri_y + 4, WHITE);
    oled_display.fillTriangle_3px(tri_x + 1, tri_y, WHITE);
  }
  if (MidiClock.state == 0) {
    oled_display.fillRect(tri_x - 1, tri_y, 2, 5, WHITE);
    oled_display.fillRect(tri_x + 2, tri_y, 2, 5, WHITE);
  }

  oled_display.setCursor(0, y_offset + 1 + 1 * 8);
  char dev[3] = "  ";

  MidiUart.device.get_name(dev);
  dev[2] = '\0';
  oled_display.print(dev);

  oled_display.setCursor(0, y_offset + 3 * 8);
  char dev2[3] = "  ";
  MidiUart2.device.get_name(dev2);
  dev2[2] = '\0';
  oled_display.print(dev2);

  oled_display.setCursor(10, y_offset + (MAX_VISIBLE_ROWS - 1) * 8);
  oled_display.print((char)('X' + proj.get_grid()));
  oled_display.print(':');

  char val[4];
  mcl_gui.put_value_at2(encoders[0]->cur + 1, val);
  val[2] = '\0';
  oled_display.print(val);
  oled_display.print(" ");
  uint8_t b = encoders[1]->cur / 16;
  oled_display.print((char)('A' + b));
  mcl_gui.put_value_at2(encoders[1]->cur - b * 16 + 1, val);
  oled_display.print(val);

  oled_display.setCursor(1, y_offset + 2 * 8);
  oled_display.fillRect(oled_display.getCursorX() - 1,
                        oled_display.getCursorY() - 6, 37, 7, WHITE);

  oled_display.setTextColor(BLACK, WHITE);
  if (row_headers[cur_row].active) {
    char rowname[10];
    strncpy(rowname, row_headers[cur_row].name, 9);
    rowname[9] = '\0';

    oled_display.print(rowname);
  }

  oled_display.setTextColor(WHITE, BLACK);
}
bool GridPage::is_slot_queue(uint8_t x, uint8_t y) {
  uint8_t slot = proj.get_grid() * GRID_WIDTH + x;
  if (mcl_actions.chains[slot].is_mode_queue() && !show_slot_menu) {
    for (uint8_t n = 0; n < mcl_actions.chains[slot].num_of_links; n++) {
      if (mcl_actions.chains[slot].rows[n] == y) {
        return true;
      }
    }
  }
  return false;
}

void GridPage::display_grid() {
  uint8_t x_offset = 43;
  uint8_t y_offset = 8;

  oled_display.setFont(&TomThumb);

  char str[3];
  PGM_P tmp;
  encoders[1]->handler = NULL;
  uint8_t row_shift = 0;
  uint8_t col_shift = 0;
  auto grid_id = proj.get_grid();
  auto *device = midi_active_peering.get_device(grid_id + 1);
  if (show_slot_menu) {
    if (cur_col + encoders[2]->cur > MAX_VISIBLE_COLS - 1) {

      col_shift = cur_col + encoders[2]->cur - MAX_VISIBLE_COLS;
    }

    if (cur_row + encoders[3]->cur > MAX_VISIBLE_ROWS - 1) {
      row_shift = cur_row + encoders[3]->cur - MAX_VISIBLE_ROWS;
    }
  }
  for (uint8_t y = 0; y < MAX_VISIBLE_ROWS; y++) {

    auto cur_posx = x_offset;
    auto cur_posy = y_offset + y * 8;
    auto w = getWidth();
    for (uint8_t x = col_shift; x < MAX_VISIBLE_COLS + col_shift && x < w;
         x++) {
      oled_display.setCursor(cur_posx, cur_posy);

      auto track_idx = x + getCol() - cur_col;
      auto row_idx = y + getRow() - cur_row;
      uint8_t track_type = row_headers[y].track_type[track_idx];
      uint8_t model = row_headers[y].model[track_idx];

      bool blink = false;
      auto active_cue_color = WHITE;

      str[0] = str[1] = '-';
      str[2] = 0;
      //  Set cell label
      switch (track_type) {
      case MD_TRACK_TYPE: {
        auto tmp = getMDMachineNameShort(model, 2);
        copyMachineNameShort(tmp, str);
        break;
      }
      case A4_TRACK_TYPE:
        str[0] = 'A';
        str[1] = (x + getCol() - cur_col) + '1';
        break;
      case EXT_TRACK_TYPE:
        str[0] = 'M';
        str[1] = (x + getCol() - cur_col) + '1';
        break;
      case MDFX_TRACK_TYPE:
        str[0] = 'F';
        str[1] = 'X';
        break;
      case MDROUTE_TRACK_TYPE:
        str[0] = 'R';
        str[1] = 'T';
        break;
      case MDTEMPO_TRACK_TYPE:
        str[0] = 'T';
        str[1] = 'P';
        break;
      case MDLFO_TRACK_TYPE:
        str[0] = 'L';
        str[1] = 'F';
        break;
      case GRIDCHAIN_TRACK_TYPE:
        str[0] = 'C';
        str[1] = 'N';
        break;
      case MNM_TRACK_TYPE:
        tmp = getMNMMachineNameShort(model, 2);
        if (tmp) {
          copyMachineNameShort(tmp, str);
        }
        break;
      }
      //  Highlight the current cursor position + slot menu apply range
      bool a = in_area(x, y + row_shift, cur_col, cur_row, encoders[2]->cur - 1,
                       encoders[3]->cur - 1);
      bool b = is_slot_queue(track_idx, row_idx);

      if (a ^ b) {
        oled_display.fillRect(cur_posx - 1, cur_posy - 6, 9, 7, WHITE);
        oled_display.setTextColor(BLACK, WHITE);
        active_cue_color = BLACK;
      } else {
        oled_display.setTextColor(WHITE, BLACK);
      }

      uint8_t track_grid_idx = track_idx + GRID_WIDTH * proj.get_grid();
      if (MidiClock.getBlinkHint(false) &&
          row_idx == active_slots[track_grid_idx]) {
        // blink, don't print
        blink = true;
      } else {
        oled_display.print(str);
      }
      if (row_idx == active_slots[track_grid_idx] && !blink) {
        // a gentle visual cue for active slots
        oled_display.drawPixel(cur_posx - 1, cur_posy - 6, active_cue_color);
      }

      // tomThumb is 4x6
      if (track_idx % 4 == 3 && w >= 8) {
        if (y == 0) {
          // draw vertical separator
          mcl_gui.draw_vertical_dashline(cur_posx + 9, 3);
        }
        cur_posx += 12;
      } else {
        cur_posx += 10;
      }
    }
  }

  // optionally, draw the first separator
  if ((getCol() - cur_col + col_shift) % 4 == 0) {
    mcl_gui.draw_vertical_dashline(x_offset - 3, 3);
  }
  oled_display.setTextColor(WHITE, BLACK);
}
void GridPage::display_slot_menu() {
  uint8_t y_offset = 8;
  grid_slot_page.draw_menu(1, y_offset, 39);
  // grid_slot_page.draw_scrollbar(36);
}

#ifdef FPS
int frames;
int frameclock;
#endif
void GridPage::display() {

  #ifdef FPS
  if (clock_diff(frameclock, slowclock) >= 1000) {
  DEBUG_PRINT("FPS: "); DEBUG_PRINTLN(frames);
  frames = 0;
  frameclock = slowclock;
  }
  frames++;
  #endif
  oled_display.clearDisplay();
  oled_display.setFont(&TomThumb);
  oled_display.setTextColor(WHITE, BLACK);
  if (!show_slot_menu) {
    display_grid_info();
  } else {
    display_slot_menu();
  }
  display_grid();
  /*
  if (row_scan) {
    mcl_gui.draw_progress_bar(8, 8, false, 18, 2, 9, 7, false);
  }
  */
  oled_display.display();
}

void GridPage::prepare() {
  if (MD.connected) {
    MD.getCurrentTrack(CALLBACK_TIMEOUT);
    MD.currentKit = MD.getCurrentKit(CALLBACK_TIMEOUT);
    MD.getBlockingKit(0x7F);
    if (MidiClock.state == 2) {
      mcl_seq.update_kit_params();
    }
  }
}

void rename_row() {
  const char *my_title = "Row Name:";
  uint8_t old_grid = proj.get_grid();
  GridRowHeader row_headers[NUM_GRIDS];

  for (uint8_t n = 0; n < NUM_GRIDS; n++) {
    proj.select_grid(n);
    proj.read_grid_row_header(&row_headers[n], grid_page.getRow());
  }

  if (row_headers[0].active) {
    if (mcl_gui.wait_for_input(row_headers[0].name, my_title, 8)) {
      strcpy(row_headers[1].name, row_headers[0].name);
      for (uint8_t n = 0; n < NUM_GRIDS; n++) {
        proj.select_grid(n);
        proj.write_grid_row_header(&(row_headers[n]), grid_page.getRow());
        proj.sync_grid();
      }
    }
  } else {
    gfx.alert("Error", "Row not active");
  }
  proj.select_grid(old_grid);
  grid_page.load_slot_models();
  grid_slot_param2.cur = 0;
  grid_slot_page.cur_row = 0;
}

void apply_slot_changes_cb() { grid_page.apply_slot_changes(); }

void GridPage::swap_grids() {
  if (grid_select_apply != proj.grid_select) {
    proj.grid_select = grid_select_apply;
    ((MCLEncoder *)encoders[0])->max = getWidth() - 1;
    // load_slot_models();
    return;
  }
}

void GridPage::apply_slot_changes(bool ignore_undo, bool ignore_func) {
  uint8_t width;
  uint8_t height;

  GridTrack temp_slot;

  temp_slot.load_from_grid(getCol(), getRow());

  uint8_t load_mode_old = mcl_cfg.load_mode;
  uint8_t undo = slot_undo && !ignore_undo && slot_undo_x == getCol() &&
                 slot_undo_y == getRow();
  DEBUG_PRINTLN("apply slot");
  swap_grids();
  if (!ignore_func) {
    void (*row_func)() =
        grid_slot_page.menu.get_row_function(grid_slot_page.encoders[1]->cur);
    if (row_func != NULL) {
      DEBUG_PRINTLN("calling menu func");
      (*row_func)();
      return;
    }
  }
  width = encoders[2]->cur;
  height = encoders[3]->cur;

  uint8_t slot_update = 0;

  if (insert_rows) {
    int8_t row_count = GRID_LENGTH - getRow() - insert_rows;
    uint8_t row_dst = getRow() + insert_rows;
    if (row_dst > GRID_LENGTH || row_count < 1) {
      return;
    }

    if (mcl_gui.wait_for_confirm("Insert", "Rows?")) {
      for (uint8_t g = 0; g < 2; g++) {
        for (uint8_t n = 0; n < row_count; n++) {
          if (n % 8) mcl_gui.draw_progress("INSERT ROWS", n + g * row_count, row_count * 2);
          mcl_clipboard.copy(0, getRow() + row_count - n - insert_rows,
                             GRID_WIDTH, 1, g);
          mcl_clipboard.paste(0, getRow() + row_count - n, g);
        }
        for (uint8_t n = 0; n < insert_rows; n++) {
          proj.clear_row_grid(getRow() + n, g);
        }
      }
    }
    goto end;
  }

  if (slot_copy + slot_paste + slot_clear + slot_load + undo == 0) {
    if ((temp_slot.link.row != slot.link.row) ||
        (temp_slot.link.loops != slot.link.loops) ||
        (temp_slot.link.length != slot.link.length)) {
      slot_update = 1;
      DEBUG_PRINTLN("Slot update");
    }
    height = 1;
  }
  if (undo == 1) {
    slot_paste = 1;
  }

  if (slot_copy == 1 || (slot_clear == 1 && !undo)) {
    if (slot_clear == 1) {
      slot_undo_x = getCol();
      slot_undo_y = getRow();
      if (width > 0) {
        oled_display.textbox("CLEAR ", "SLOTS");
      } else {
        oled_display.textbox("CLEAR ", "SLOT");
      }
      slot_undo = 1;
    } else {
      slot_undo = 0;
      if (width > 0) {
        oled_display.textbox("COPY ", "SLOTS");
      } else {
        oled_display.textbox("COPY ", "SLOT");
      }
    }
    mcl_clipboard.copy(getCol(), getRow(), width, height, proj.get_grid());
    if (slot_clear) {
      goto run;
    }
  }

  else if (slot_paste == 1) {
    if (undo) {
      oled_display.textbox("UNDO", "");
    } else {
      oled_display.textbox("PASTE", "");
    }
    slot_undo = 0;
    mcl_clipboard.paste(getCol(), getRow(), proj.get_grid());
  } else {
    if (slot_update == 1) {
      oled_display.textbox("SLOT ", "UPDATE");
    }

    if (slot_load) {
      if (height > 1) {
        mcl_cfg.load_mode = LOAD_QUEUE;
      }
      grid_load_page.display_load();
    }
  run:

    oled_display.display();

    bool activate_header = false;

    uint8_t track_select_array[GRID_LENGTH] = {0};
    GridRowHeader header;

    SeqTrack seq_track;
    uint16_t target_length = slot.link.length * seq_track.get_speed_multiplier(slot.link.speed) * slot.link.loops;

    for (uint8_t y = 0; y < height && y + getRow() < GRID_LENGTH; y++) {
      uint8_t ypos = y + getRow();
      proj.read_grid_row_header(&header, y + getRow());

      memset(track_select_array, 0, sizeof(track_select_array));

      if (slot_clear && height > 8) {
        mcl_gui.draw_progress("", y, height);
      }
      for (uint8_t x = 0; x < width && x + getCol() < getWidth(); x++) {
        uint8_t xpos = x + getCol();
        if (slot_clear == 1) {
          // Delete slot(s)
          proj.clear_slot_grid(xpos, ypos);
          header.update_model(xpos, 0, EMPTY_TRACK_TYPE);
        } else if (slot_update == 1) {
          // Save slot link data
          activate_header = true;
          if (x == 0) {
            //slot.active = header.track_type[xpos];
            slot.store_in_grid(xpos, ypos);
          }
          else {
            temp_slot.load_from_grid(xpos,ypos);
            uint16_t temp_slot_length = temp_slot.link.length * seq_track.get_speed_multiplier(temp_slot.link.speed);
            //check if length is an even multiple, otherwise skip.
            if (temp_slot_length && !(target_length % temp_slot_length) && temp_slot_length <= target_length) {
              temp_slot.link.loops = target_length / temp_slot_length;
              temp_slot.link.row = slot.link.row;
              temp_slot.store_in_grid(xpos, ypos);
            }
            else if (temp_slot.link.speed == slot.link.speed) {
              temp_slot.link.length = slot.link.length;
              temp_slot.store_in_grid(xpos, ypos);
            }
          }
        } else if (slot_load == 1) {
          // if (height > 1 && y == 0) {
          //   mcl_actions.chains[xpos].init();
          // }
          track_select_array[xpos + proj.get_grid() * 16] = 1;
        }
      }
      if (slot_load == 1) {
        mcl_actions.load_tracks(ypos, track_select_array);
      }
      // If all slots are deleted then clear the row name
      else if ((header.is_empty() && (slot_clear == 1)) || (activate_header)) {
        header.active = activate_header;
        strcpy(header.name, "\0");
        proj.write_grid_row_header(&header, ypos);
      }
    }
  }
end:
  if ((slot_clear == 1) || (slot_paste == 1) || (slot_update == 1) ||
      (insert_rows == 1)) {
    proj.sync_grid();
    load_slot_models();
  }
  mcl_cfg.load_mode = load_mode_old;
  slot_apply = 0;
  slot_load = 0;
  slot_clear = 0;
  slot_copy = 0;
  slot_paste = 0;
  insert_rows = 0;
  slot.load_from_grid(getCol(), getRow());
}

bool GridPage::handleEvent(gui_event_t *event) {
  if (note_interface.is_event(event)) {
    uint8_t mask = event->mask;
    uint8_t port = event->port;
    MidiDevice *device = midi_active_peering.get_device(port);

    uint8_t track = event->source - 128;
    if (device != &MD) {
      return true;
    }

    uint8_t row = grid_page.bank * 16 + track;
    if (event->mask == EVENT_BUTTON_RELEASED) {
      if (grid_page.bank_popup > 0) {
        if (note_interface.notes_all_off()) {
          note_interface.init_notes();
          grid_page.bank_popup_loadmask = 0;
        }
        return true;
      }
    }

    if (event->mask == EVENT_BUTTON_PRESSED) {
      if (grid_page.bank_popup > 0) {

        uint8_t load_mode_old = mcl_cfg.load_mode;
        uint8_t load_count = popcount16(grid_page.bank_popup_loadmask);

        if (load_count == 0) {
          grid_page.jump_to_row(row);
          if (load_mode_old != LOAD_AUTO) {
            mcl_cfg.load_mode = LOAD_MANUAL;
          }
          mcl_actions.init_chains();
        }
        if (load_count > 0) {
          mcl_cfg.load_mode = LOAD_QUEUE;
        }

        if (load_count == 1) {
          for (uint8_t n = 0; n < 16; n++) {
            if (IS_BIT_SET16(grid_page.bank_popup_loadmask, n)) {
              uint8_t r = grid_page.bank * 16 + n;
              CLEAR_BIT16(grid_page.bank_popup_loadmask, n);
              // Reload as queue.
              grid_page.load_row(n, r);
              break;
            }
          }
        }

        grid_page.load_row(track, row);

        if (!trig_interface.is_key_down(MDX_KEY_BANKA) &&
            !trig_interface.is_key_down(MDX_KEY_BANKB) &&
            !trig_interface.is_key_down(MDX_KEY_BANKC) &&
            !trig_interface.is_key_down(MDX_KEY_BANKD)) {
          grid_page.close_bank_popup();
        }
        mcl_cfg.load_mode = load_mode_old;
        return true;
      }
    }
  }
  if (EVENT_CMD(event)) {

    uint8_t key = event->source - 64;
    if (trig_interface.is_key_down(MDX_KEY_PATSONG)) {
       if (key == MDX_KEY_PATSONGKIT && event->mask == EVENT_BUTTON_PRESSED) {
          grid_page.grid_select_apply = !grid_page.grid_select_apply;
          swap_grids();
          init();
          return true;
       }
       if (show_slot_menu) {
        if (event->mask == EVENT_BUTTON_PRESSED) {
          switch (key) {
          case MDX_KEY_BANKA:
          case MDX_KEY_BANKB:
          case MDX_KEY_BANKC: {
            mcl_cfg.load_mode = key - MDX_KEY_BANKA + 1;
            return true;
          }
          case MDX_KEY_NO: {
            goto next;
          }
          }
        }
        return grid_slot_page.handleEvent(event);
      }
    }
    next:
    if (event->mask == EVENT_BUTTON_PRESSED) {
      uint8_t inc = 1;
      if (trig_interface.is_key_down(MDX_KEY_FUNC)) {
        inc = 16;
      }
      if (show_slot_menu) {
        switch (key) {
        // case MDX_KEY_NO: {
        //  goto slot_menu_off;
        //}
        case MDX_KEY_YES: {
          slot_load = 1;
          goto slot_menu_off;
        }
        case MDX_KEY_COPY: {
          slot_copy = 1;
          apply_slot_changes(false, true);
          init();
          // if (trig_interface.is_key_down(MDX_KEY_NO)) {
          //  goto slot_menu_on;
          //}
          return true;
        }
        case MDX_KEY_CLEAR: {
          slot_clear = 1;
          apply_slot_changes(false, true);
          init();
          // if (trig_interface.is_key_down(MDX_KEY_NO)) {
          //  goto slot_menu_on;
          //}
          return true;
        }
        case MDX_KEY_PASTE: {
          slot_paste = 1;
          apply_slot_changes(false, true);
          init();
          // if (trig_interface.is_key_down(MDX_KEY_NO)) {
          //  goto slot_menu_on;
          //}
          return true;
        }
        case MDX_KEY_UP: {
          param4.cur -= inc;
          return true;
        }
        case MDX_KEY_DOWN: {
          param4.cur += inc;
          return true;
        }
        case MDX_KEY_LEFT: {
          if (inc > 1) {
            inc = 4;
          }
          param3.cur = max(0, param3.cur - inc);
          return true;
        }
        case MDX_KEY_RIGHT: {
          if (inc > 1) {
            inc = 4;
          }

          param3.cur += inc;
          return true;
        }
        case MDX_KEY_BANKA:
        case MDX_KEY_BANKB:
        case MDX_KEY_BANKC: {
          if (!trig_interface.is_key_down(MDX_KEY_FUNC)) {
            mcl_cfg.load_mode = key - MDX_KEY_BANKA + 1;
            return true;
          }
        }
        }
      }
      switch (key) {
      case MDX_KEY_UP: {
      up:
        param2.cur -= inc;
        reset_undo();
        return true;
      }
      case MDX_KEY_DOWN: {
      down:
        param2.cur += inc;
        reset_undo();
        return true;
      }
      case MDX_KEY_LEFT: {
      left:
        if (inc > 1) {
          inc = 4;
        }
        param1.cur = max(0, param1.cur - inc);
        reset_undo();
        return true;
      }
      case MDX_KEY_RIGHT: {
      right:
        if (inc > 1) {
          inc = 4;
        }
        param1.cur += inc;
        reset_undo();
        return true;
      }
      case MDX_KEY_YES: {
        trig_interface.ignoreNextEvent(MDX_KEY_YES);
        if (!trig_interface.is_key_down(MDX_KEY_FUNC)) {
          goto load;
        }
        goto save;
      }
      case MDX_KEY_NO: {
        if (!show_slot_menu) {
          goto slot_menu_on;
        }
        break;
      }
      }
    }
    if (event->mask == EVENT_BUTTON_RELEASED) {
      if (bank_popup) {
        switch (key) {
        case MDX_KEY_BANKA:
        case MDX_KEY_BANKB:
        case MDX_KEY_BANKC:
        case MDX_KEY_BANKD: {
          uint8_t bank = key - MDX_KEY_BANKA;
          if (bank_popup == 1) {
            bank_popup = 2;
            bank_popup_lastclock = slowclock;
            MD.draw_bank(bank);
          }
          return true;
        }
        }
      }
      switch (key) {
      case MDX_KEY_NO: {
        if (!trig_interface.is_key_down(MDX_KEY_PATSONG)) {
          goto slot_menu_off;
        }
        return true;
      }
      }
    }
  }

  if (!show_slot_menu) {
    if (EVENT_RELEASED(event, Buttons.BUTTON1)) {
    save:
      GUI.setPage(&grid_save_page);

      return true;
    }

    if (EVENT_RELEASED(event, Buttons.BUTTON4)) {
    load:
      GUI.setPage(&grid_load_page);

      return true;
    }
  }

  if (EVENT_PRESSED(event, Buttons.BUTTON3)) {
  slot_menu_on:
    DEBUG_DUMP(getCol());
    DEBUG_DUMP(getRow());
    slot.load_from_grid(getCol(), getRow());
    DEBUG_PRINTLN(F("what's in the slot"));
    DEBUG_DUMP(slot.link.loops);
    DEBUG_DUMP(slot.link.row);
    encoders[0] = &grid_slot_param1;
    encoders[1] = &grid_slot_param2;
    encoders[2]->cur = 1;
    encoders[3]->cur = 1;
    slot_apply = 0;
    if (!slot.is_ext_track()) {
      grid_slot_page.menu.enable_entry(2, true);
      grid_slot_page.menu.enable_entry(3, false);
    } else {
      grid_slot_page.menu.enable_entry(2, false);
      grid_slot_page.menu.enable_entry(3, true);
    }
    show_slot_menu = true;
    grid_slot_page.init();
    return true;
  }

  if (EVENT_RELEASED(event, Buttons.BUTTON3)) {
  slot_menu_off:
    if (show_slot_menu && !trig_interface.is_key_down(MDX_KEY_NO)) {
      uint8_t old_undo = slot_undo;
      bool restore_undo = false;
      // Prevent undo from occuring when re-entering shift menu. Want to keep
      // undo flag in case user decides to undo with MD GUI.
      if (slot_copy + slot_paste + slot_clear + slot_load == 0) {
        slot_undo = 0;
        restore_undo = true;
      }

      apply_slot_changes();

      if (restore_undo) {
        slot_undo = old_undo;
      }

      DEBUG_PRINTLN("here");
      init();
    }
    return true;
  }
  if ((EVENT_PRESSED(event, Buttons.BUTTON1) && BUTTON_DOWN(Buttons.BUTTON4)) ||
      (EVENT_PRESSED(event, Buttons.BUTTON4) && BUTTON_DOWN(Buttons.BUTTON1))) {
  system:
    system_page.isSetup = false;
    GUI.pushPage(&system_page);
    return true;
  }
  if (EVENT_PRESSED(event, Buttons.BUTTON2)) {
    GUI.setPage(&page_select_page);
    return true;
  }

  return false;
}
