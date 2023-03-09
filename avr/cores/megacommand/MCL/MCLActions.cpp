//* Copyright 2018, Justin Mammarella jmamma@gmail.com */
#include "MCL_impl.h"

#define MD_KIT_LENGTH 0x4D0

// No STL, no closure, no std::function, cannot make this generic...
// void __attribute__ ((noinline)) FOREACH_GRID_TRACK(void(*fn)(uint8_t,
// uint8_t, uint8_t, MidiDevice*, ElektronDevice*)) { uint8_t grid; uint8_t
// track_idx; MidiDevice *devs[2] = {
// midi_active_peering.get_device(UART1_PORT),
// midi_active_peering.get_device(UART2_PORT),
//};
// ElektronDevice *elektron_devs[2] = {
// devs[0]->asElektronDevice(),
// devs[1]->asElektronDevice(),
//};
// for (uint8_t i = 0; i < NUM_SLOTS; ++i) {
// if (i < GRID_WIDTH) {
// grid = 0;
// track_idx = i;
//} else {
// grid = 1;
// track_idx = i - GRID_WIDTH;
//}
// fn(i, grid, track_idx, devs[grid], elektron_devs[grid]);
//}
//}

void MCLActions::setup() {
  // DEBUG_PRINTLN(F("mcl actions setup"));
  mcl_actions_callbacks.setup_callbacks();
  mcl_actions_midievents.setup_callbacks();
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {
    next_transitions[i] = 0;
    transition_offsets[i] = 0;
    send_machine[i] = 0;
    transition_level[i] = 0;
  }
  memset(dev_sync_slot, 255, NUM_DEVS);
}

void MCLActions::init_chains() {
  for (uint8_t n = 0; n < NUM_SLOTS; n++) {
    mcl_actions.chains[n].init();
  }
}

void MCLActions::kit_reload(uint8_t pattern) {
  // DEBUG_PRINT_FN();
  /*
    if (mcl_actions.do_kit_reload != 255) {
      if (mcl_actions.writepattern == pattern) {
        auto dev1 =
            midi_active_peering.get_device(UART1_PORT)->asElektronDevice();
        auto dev2 =
            midi_active_peering.get_device(UART2_PORT)->asElektronDevice();
        if (dev1 != nullptr) {
          dev1->loadKit(mcl_actions.do_kit_reload);
        }
        if (dev2 != nullptr) {
          dev2->loadKit(mcl_actions.do_kit_reload);
        }
      }
      mcl_actions.do_kit_reload = 255;
    }
  */
}

uint8_t MCLActions::get_grid_idx(uint8_t slot_number) {
  return slot_number / GRID_WIDTH;
}

GridDeviceTrack *MCLActions::get_grid_dev_track(uint8_t slot_number,
                                                uint8_t *track_idx,
                                                uint8_t *dev_idx) {
  uint8_t grid_idx = get_grid_idx(slot_number);
  MidiDevice *devs[2] = {
      midi_active_peering.get_device(UART1_PORT),
      midi_active_peering.get_device(UART2_PORT),
  };
  // Find first device that is hosting this slot_number.
  for (uint8_t n = 0; n < 2; n++) {
    auto *p = &(devs[n]->grid_devices[grid_idx]);
    for (uint8_t i = 0; i < GRID_WIDTH; i++) {
      if (slot_number == p->tracks[i].get_slot_number()) {
        *track_idx = i;
        *dev_idx = n;
        return &(p->tracks[i]);
      }
    }
  }
  *track_idx = 255;
  *dev_idx = 255;
  return nullptr;
}

void md_import() {

  if (!mcl_gui.wait_for_confirm("Import:", "Overwrite rows?")) {
    return;
  }

  uint8_t track_select_array[NUM_SLOTS] = {0};
  uint8_t track_idx, dev_idx;

  MidiDevice *devs[2] = {
      midi_active_peering.get_device(UART1_PORT),
      midi_active_peering.get_device(UART2_PORT),
  };

  ElektronDevice *elektron_devs[2] = {
      devs[0]->asElektronDevice(),
      devs[1]->asElektronDevice(),
  };

  for (uint8_t n = 0; n < NUM_SLOTS; n++) {
    GridDeviceTrack *gdt =
        mcl_actions.get_grid_dev_track(n, &track_idx, &dev_idx);

    if (gdt == nullptr || track_idx == 255)
      continue;

    if (devs[dev_idx] == &MD) {
      track_select_array[n] = 1;
    }
  }

  for (uint8_t n = opt_import_src;
       n < opt_import_src + opt_import_count && n < 128; n++) {
    uint8_t count = n - opt_import_src;
    mcl_gui.draw_progress("IMPORTING", count, opt_import_count);
    mcl_actions.save_tracks(opt_import_dest + count, track_select_array,
                            SAVE_MD, n);
  }

  grid_page.row_scan = GRID_LENGTH;
  grid_page.reload_slot_models = false;
  GUI.setPage(&grid_page);
}

void MCLActions::save_tracks(int row, uint8_t *slot_select_array, uint8_t merge,
                             uint8_t readpattern) {
  // DEBUG_PRINT_FN();

  EmptyTrack empty_track;

  if (readpattern == 255) {
    readpattern = MD.currentPattern;
  }

  uint8_t old_grid = proj.get_grid();

  bool save_dev_tracks[2] = {false, false};
  MidiDevice *devs[2] = {
      midi_active_peering.get_device(UART1_PORT),
      midi_active_peering.get_device(UART2_PORT),
  };
  ElektronDevice *elektron_devs[2] = {
      devs[0]->asElektronDevice(),
      devs[1]->asElektronDevice(),
  };

  uint8_t i = 0;

  uint8_t track_idx, dev_idx;

  for (i = 0; i < NUM_SLOTS; i++) {
    if (slot_select_array[i] > 0) {
      GridDeviceTrack *gdt = get_grid_dev_track(i, &track_idx, &dev_idx);
      if (gdt != nullptr) {
        save_dev_tracks[dev_idx] = true;
      }
    }
  }

  if (MidiClock.state == 2) {
    merge = 0;
  }

  for (i = 0; i < NUM_DEVS; ++i) {
    if (elektron_devs[i] != nullptr) {
      if (save_dev_tracks[i]) {
        if (merge > 0) {
          // DEBUG_PRINTLN(F("fetching pattern"));
          // DEBUG_PRINTLN(readpattern);
          if (!elektron_devs[i]->getBlockingPattern(readpattern)) {
            // DEBUG_PRINTLN(F("could not receive pattern"));
            save_dev_tracks[i] = false;
            continue;
          }
          ElektronPattern *p = elektron_devs[i]->getPattern();
          if (p->isEmpty()) {
            save_dev_tracks[i] = false;
            continue;
          }
          if (!elektron_devs[i]->getBlockingKit(p->getKit())) {
            // DEBUG_PRINTLN(F("could not receive kit"));
            continue;
          }
        } else {
          if (elektron_devs[i]->canReadWorkspaceKit()) {
            if (!elektron_devs[i]->getWorkSpaceKit()) {
              // DEBUG_PRINTLN(F("could not receive kit"));
              save_dev_tracks[i] = false;
              continue;
            }
          } else if (elektron_devs[i]->canReadKit()) {
            auto kit = elektron_devs[i]->getCurrentKit();
            elektron_devs[i]->saveCurrentKit(kit);
            if (!elektron_devs[i]->getBlockingKit(kit)) {
              // DEBUG_PRINTLN(F("could not receive kit"));
              save_dev_tracks[i] = false;
              continue;
            }
          }
        }
        if (MidiClock.state == 2) {
          elektron_devs[i]->updateKitParams();
        } else {
          elektron_devs[i]->undokit_sync();
        }
      }
    }
  }

  GridRowHeader row_headers[NUM_GRIDS];
  GridTrack grid_track;

  for (uint8_t n = 0; n < NUM_GRIDS; n++) {
    proj.select_grid(n);
    proj.read_grid_row_header(&row_headers[n], row);
  }

  for (i = 0; i < NUM_SLOTS; i++) {
    if (slot_select_array[i] > 0) {

      GridDeviceTrack *gdt = get_grid_dev_track(i, &track_idx, &dev_idx);
      uint8_t grid_idx = get_grid_idx(i);

      bool online = (devs[dev_idx] != nullptr);
      // If save_dev_tracks[dev_idx] turns false, it means getBlockingKit
      // has failed, so we just skip this device.

      if (!save_dev_tracks[dev_idx]) {
        continue;
      }

      if (gdt != nullptr) {
        proj.select_grid(grid_idx);

        // Preserve existing link settings before save.

        if (row_headers[grid_idx].track_type[track_idx] != EMPTY_TRACK_TYPE) {
          // DEBUG_PRINTLN(F("tl"));
          if (!grid_track.load_from_grid(track_idx, row))
            continue;
          memcpy(&empty_track.link, &grid_track.link, sizeof(GridLink));
        } else {
          empty_track.link.init(row);
        }
        auto pdevice_track =
            ((DeviceTrack *)&empty_track)->init_track_type(gdt->track_type);
        pdevice_track->store_in_grid(track_idx, row, gdt->seq_track, merge,
                                     online);
        row_headers[grid_idx].update_model(
            track_idx, pdevice_track->get_model(), gdt->track_type);
      }
    }
  }

  // Only copy row name from kit if, the current row is not active.
  for (uint8_t n = 0; n < NUM_GRIDS; n++) {
    if (!row_headers[n].active && devs[0] == &MD && save_dev_tracks[n]) {
      for (uint8_t c = 0; c < 17; c++) {
        row_headers[n].name[c] = MD.kit.name[c];
      }
      row_headers[n].active = true;
    }
    proj.write_grid_row_header(&row_headers[n], row, n);
    proj.sync_grid(n);
  }

  proj.select_grid(old_grid);
}

void MCLActions::load_tracks(int row, uint8_t *slot_select_array,
                             uint8_t *_row_array, uint8_t load_mode) {
  // DEBUG_PRINT_FN();
  ElektronDevice *elektron_devs[2] = {
      midi_active_peering.get_device(UART1_PORT)->asElektronDevice(),
      midi_active_peering.get_device(UART2_PORT)->asElektronDevice(),
  };
  if (load_mode == 255) {
    load_mode = mcl_cfg.load_mode;
  }
  uint8_t row_array[NUM_SLOTS] = {};
  uint8_t cache_track_array[NUM_SLOTS] = {};
  uint8_t track_idx, dev_idx;
  bool recache = false;
  DEBUG_PRINTLN("load tracks");
  DEBUG_PRINTLN(load_mode);
  for (uint8_t n = 0; n < NUM_SLOTS; ++n) {
    GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
    //    DEBUG_PRINTLN(slot_select_array[n]);
    if ((slot_select_array[n] == 0) || (gdt == nullptr)) {
      continue;
    }
    if (_row_array == nullptr) {
      row_array[n] = row;
    } else {
      row_array[n] = _row_array[n];
    }

    if (load_mode == LOAD_QUEUE) {
      chains[n].add(row_array[n], get_chain_length());
      DEBUG_PRINTLN("adding link");
      if (chains[n].num_of_links > 1) {
        slot_select_array[n] = 0;
        if (chains[n].num_of_links == 2) {
          cache_track_array[n] = 1;
          recache = true;
        }
      }
    } else {
      chains[n].init();
    }
    chains[n].mode = load_mode;
  }

  if (MidiClock.state == 2) {
    manual_transition(slot_select_array, row_array);
    return;
  }

  for (uint8_t i = 0; i < NUM_DEVS; ++i) {
    if (elektron_devs[i] != nullptr &&
        elektron_devs[i]->canReadWorkspaceKit()) {
      elektron_devs[i]->getBlockingKit(0x7F);
    }
  }
  if (recache) {
    bool gui_update = false;
    cache_next_tracks(cache_track_array, gui_update);
  } else {
    send_tracks_to_devices(slot_select_array, row_array);
  }
}

void MCLActions::collect_tracks(uint8_t *slot_select_array,
                                uint8_t *row_array) {

  uint8_t old_grid = proj.get_grid();
  memset(dev_sync_slot, 255, NUM_DEVS);

  uint8_t track_idx, dev_idx;
  for (uint8_t n = 0; n < NUM_SLOTS; ++n) {

    GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
    uint8_t grid_idx = get_grid_idx(n);
    proj.select_grid(grid_idx);

    if ((slot_select_array[n] == 0) || (gdt == nullptr)) {
      // Ignore slots that are not device supported.
      slot_select_array[n] = 0;
      continue;
    }
    uint8_t row = row_array[n];
    EmptyTrack empty_track;
    // DEBUG_PRINTLN(F("cl"));
    auto *device_track = empty_track.load_from_grid(track_idx, row);

    if (device_track == nullptr || device_track->active != gdt->track_type) {
      empty_track.clear();
      device_track = device_track->init_track_type(gdt->track_type);
      if (device_track) {
        device_track->init(track_idx, gdt->seq_track);
      }
      send_machine[n] = 1;
    } else {
      send_machine[n] = device_track->transition_cache(track_idx, n);
      dev_sync_slot[dev_idx] = n;
    }
    if (device_track) {
      device_track->store_in_mem(gdt->mem_slot_idx);
    }
  }

  proj.select_grid(old_grid);
}

void MCLActions::manual_transition(uint8_t *slot_select_array,
                                   uint8_t *row_array) {
  // DEBUG_PRINT_FN();
  uint8_t q = get_quant();

  // DEBUG_CHECK_STACK();

  collect_tracks(slot_select_array, row_array);

  uint16_t next_step = (MidiClock.div16th_counter / q) * q + q;
  uint8_t loops = 1;

  uint8_t track_idx, dev_idx;

  bool recalc_latency = true;
  ////DEBUG_PRINTLN("manual trans");
again:
  uint8_t row = grid_task.next_active_row;
  uint16_t div16th_counter = MidiClock.div16th_counter;
  for (uint8_t n = 0; n < NUM_SLOTS; n++) {

    if (slot_select_array[n] > 0) {
      row = row_array[n];
      if (q == 255) {
        GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
        if (gdt != nullptr) {
          transition_level[n] = 0;
          next_transitions[n] =
              div16th_counter - ((float)gdt->seq_track->step_count *
                                 gdt->seq_track->get_speed_multiplier());
          links[n].speed = gdt->seq_track->speed;
          links[n].length = gdt->seq_track->length;
          links[n].row = row;
          links[n].loops = loops;
          bool ignore_chain_settings = true;
          calc_next_slot_transition(n, ignore_chain_settings);
          grid_page.active_slots[n] = SLOT_PENDING;
        }
      } else {
        // transition_level[n] = gridio_param3.getValue();
        transition_level[n] = 0;
        next_transitions[n] = next_step;
        links[n].row = row;
        links[n].loops = 1;
        // if (grid_page.active_slots[n] < 0) {
        DEBUG_PRINT("slot man trans "); DEBUG_PRINT(n); DEBUG_PRINT(" "); DEBUG_PRINTLN(next_transitions[n]);
        grid_page.active_slots[n] = SLOT_PENDING;
        // }
      }
    } else {
      //calc_next_slot_transition(n);
    }
  }

  calc_next_transition();

  grid_task.next_active_row = row;
  grid_task.chain_behaviour = false;
  grid_task.gui_update();

  if (recalc_latency) {
    calc_latency();
  }

  // //DEBUG_PRINTLN("NEXT STEP");
  // //DEBUG_PRINTLN(next_step);
  // //DEBUG_PRINTLN(next_transition);
  // //DEBUG_PRINTLN(MidiClock.div16th_counter);

  // int32_t pos = next_transition - (div192th_total_latency / 12) -
  // MidiClock.div16th_counter; next transition should always be at least 2
  // steps away.
  if (next_transition - (div192th_total_latency / 12) - 2 <
      MidiClock.div16th_counter) {

    if (q == 255) {
      loops += 1;
    } else {
      // DEBUG_PRINTLN("try again");
      next_step += q;
    }
    recalc_latency = false;
    goto again;
  }
}

bool MCLActions::load_track(uint8_t track_idx, uint8_t row, uint8_t pos,
                            GridDeviceTrack *gdt, uint8_t *send_masks) {
  EmptyTrack empty_track;
  auto *ptrack = empty_track.load_from_grid_512(track_idx, row);

  if (ptrack == nullptr) {
    // DEBUG_PRINTLN("bad read");
    return false;
  } // read failure

  ptrack->link.store_in_mem(pos, &(links[0]));

  if (ptrack->active != gdt->track_type) {
    empty_track.clear();
     DEBUG_PRINTLN("Clearing track");
     DEBUG_PRINTLN(pos);
    ptrack->init_track_type(gdt->track_type);
    ptrack->init(track_idx, gdt->seq_track);
    ptrack->load_seq_data(gdt->seq_track);
  } else {
   DEBUG_PRINTLN("load immediate track");
    ptrack->load_immediate(track_idx, gdt->seq_track);
    ptrack->store_in_mem(track_idx);
    send_masks[pos] = 1;
  }
  return true;
}

void MCLActions::send_tracks_to_devices(uint8_t *slot_select_array,
                                        uint8_t *row_array) {
  // DEBUG_PRINT_FN();
  DEBUG_PRINTLN("send tracks to devices");

  uint8_t select_array[NUM_SLOTS];
  // Take a copy, because we call GUI.loop later.
  memcpy(select_array, slot_select_array, NUM_SLOTS);

  MidiDevice *devs[2] = {
      midi_active_peering.get_device(UART1_PORT),
      midi_active_peering.get_device(UART2_PORT),
  };

  uint8_t mute_states[NUM_SLOTS];
  uint8_t send_masks[NUM_SLOTS] = {0};
  uint8_t row = 0;
  uint8_t old_grid = proj.get_grid();

  uint8_t track_idx, dev_idx;

  // DEBUG_PRINTLN("send tracks 1");
  // DEBUG_PRINTLN((int)SP);
  // DEBUG_CHECK_STACK();

  uint8_t last_slot = 255;
  for (uint8_t i = 0; i < NUM_SLOTS; i++) {

    GridDeviceTrack *gdt = get_grid_dev_track(i, &track_idx, &dev_idx);
    uint8_t grid_idx = get_grid_idx(i);
    proj.select_grid(grid_idx);

    if (gdt == nullptr) {
      goto cont;
    }

    mute_states[i] = gdt->seq_track->mute_state;
    gdt->seq_track->mute_state = SEQ_MUTE_ON;

    if ((select_array[i] == 0)) {
    cont:
      select_array[i] = 0;
      continue;
    }

    row = grid_page.getRow();
    if (row_array) {
      row = row_array[i];
    }
    last_slot = i;

    grid_page.active_slots[i] = row;

    // DEBUG_DUMP("here");
    // DEBUG_DUMP(row);

    if (!load_track(track_idx, row, i, gdt, send_masks)) {
      select_array[i] = 0;
    }
  }
  /*Send the encoded kit to the devices via sysex*/
  uint16_t myclock = slowclock;
  uint16_t latency_ms = 0;

  GridRowHeader row_header;

  proj.select_grid(old_grid);
  proj.read_grid_row_header(&row_header, row);

  if (mcl_cfg.uart2_prg_out > 0) {
    MidiUart2.sendProgramChange(mcl_cfg.uart2_prg_out - 1, row);
  }

  for (uint8_t i = 0; i < NUM_DEVS; ++i) {
    auto elektron_dev = devs[i]->asElektronDevice();
    if (elektron_dev != nullptr) {

      char *dst = devs[i]->asElektronDevice()->getKitName();
      if (dst != nullptr) {
        if (row_header.active) {
          uint8_t len = elektron_dev->sysex_protocol.kitname_length;
          memcpy(dst, row_header.name, len);
          dst[len - 1] = '\0';
        } else {
          strcpy(dst, "NEW KIT");
        }
        // DEBUG_PRINTLN("SEND NAME");
        // DEBUG_PRINTLN(dst);
      }
      latency_ms += elektron_dev->sendKitParams(send_masks + i * GRID_WIDTH);
    }
  }

  // switch back to old grid before driving the GUI loop
  // note, do not re-enter grid_task -- stackoverflow

  GUI.removeTask(&grid_task);
  while (clock_diff(myclock, slowclock) < latency_ms) {
    //  GUI.loop();
  }
  GUI.addTask(&grid_task);
  for (uint8_t i = 0; i < NUM_SLOTS; ++i) {
    GridDeviceTrack *gdt = get_grid_dev_track(i, &track_idx, &dev_idx);
    if (gdt != nullptr) {
      gdt->seq_track->mute_state = mute_states[i];
    }
  }
  /*All the tracks have been sent so clear the write queue*/
  write_original = 0;

  // if ((mcl_cfg.load_mode == 0) || (mcl_cfg.load_mode == LOAD_MANUAL)) {
  //   next_transition = (uint16_t)-1;
  //   return;
  // }

  // Cache
  // DEBUG_CHECK_STACK();
  bool gui_update = false;
  cache_next_tracks(select_array, gui_update);

  // in_sysex = 0;

  for (uint8_t n = 0; n < NUM_SLOTS; n++) {
    if ((select_array[n] > 0) && (grid_page.active_slots[n] != SLOT_DISABLED)) {
      GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
      if (gdt != nullptr) {
        transition_level[n] = 0;
        next_transitions[n] = MidiClock.div16th_counter -
                              (gdt->seq_track->step_count *
                               gdt->seq_track->get_speed_multiplier());
        calc_next_slot_transition(n);
      }
    }
  }
  grid_task.last_active_row = row;
  grid_task.next_active_row = row;
  grid_task.chain_behaviour = false;

  if (last_slot != 255) {
    grid_task.last_active_row = grid_task.last_active_row;
    grid_task.next_active_row = links[last_slot].row;
    grid_task.chain_behaviour = chains[last_slot].mode > 1;
  }
  grid_task.gui_update();

  calc_next_transition();
  calc_latency();
}

void MCLActions::cache_track(uint8_t n, uint8_t track_idx, uint8_t dev_idx,
                             GridDeviceTrack *gdt) {}

void MCLActions::cache_next_tracks(uint8_t *slot_select_array,
                                   bool gui_update) {
  // //DEBUG_PRINT_FN();

  //  //DEBUG_PRINTLN("cache next");
  //  //DEBUG_PRINTLN((int)SP);
  // DEBUG_CHECK_STACK();
  /*
    while (SeqTrack::in_countdown && (MidiClock.state == 2)) {
      gui_loop();
    }
  */
  uint8_t old_grid = proj.get_grid();

  uint8_t track_idx, dev_idx;

  memset(dev_sync_slot, 255, sizeof(dev_sync_slot));

  const uint8_t div32th_margin = 1;
  uint32_t diff = 0;

  float tempo = MidiClock.get_tempo();
  //  div32th_per_second: tempo / 60.0f * 4.0f * 2.0f = tempo * 8 / 60
  float div32th_per_second = tempo * 0.133333333333f;
  //  div32th_per_second: tempo / 60.0f * 4.0f * 2.0f * 6.0f = tempo * 8 / 10
  // float div192th_per_second = tempo * 0.8f;
  // float div192th_time = 1.0 / div192th_per_second;
  float div192th_time = 1.25 / tempo;

  // float div192th_time = 1.25 / tempo;
  // diff * div19th_time > 80ms equivalent to diff > (0.08/1.25) * tempo
  float ms = (0.80 / 1.25) * tempo;

  for (uint8_t n = 0; n < NUM_SLOTS; n++) {

    if (slot_select_array[n] == 0)
      continue;

    GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
    uint8_t grid_idx = get_grid_idx(n);

    if (gdt == nullptr)
      continue;

    uint32_t diff = MidiClock.clock_diff_div192(
        MidiClock.div192th_counter, (uint32_t)next_transition * 12 + 4 * 12);
    while ((gdt->seq_track->count_down && (MidiClock.state == 2))) {
      proj.select_grid(old_grid);
      handleIncomingMidi();
      if (((float)diff > 0.08 * 0.8 * tempo) && gui_update) {
        GUI.loop();
      }
    }

    proj.select_grid(grid_idx);

    if (chains[n].is_mode_queue()) {
      if (chains[n].get_length() == QUANT_LEN) {
        if (links[n].loops == 0) {
          links[n].loops = 1;
        }
      } else if (chains[n].get_length() != QUANT_LEN) {
        links[n].loops = 1;
        links[n].length = (float)chains[n].get_length() /
                          (float)gdt->seq_track->get_speed_multiplier();
      }
      chains[n].inc();
      links[n].row = chains[n].get_row();
      if (links[n].row == 255) {
        setLed2();
      }
    }

    // if (links[n].row >= GRID_LENGTH)
    if (links[n].row >= GRID_LENGTH ||
        links[n].row == grid_page.active_slots[n] || links[n].loops == 0)
      continue;

    EmptyTrack empty_track;

    auto *ptrack = empty_track.load_from_grid_512(track_idx, links[n].row);
    send_machine[n] = 1;

    if (ptrack == nullptr || ptrack->active != gdt->track_type) {
      // EMPTY_TRACK_TYPE
      ////DEBUG_PRINTLN(F("clear track"));
      empty_track.clear();
      ptrack = empty_track.init_track_type(gdt->track_type);
      ptrack->init(track_idx, gdt->seq_track);
    } else {
      if (ptrack->get_sound_data_ptr() && ptrack->get_sound_data_size()) {
        if (ptrack->memcmp_sound(gdt->mem_slot_idx) != 0) {
          ptrack->transition_cache(track_idx, n);
          send_machine[n] = 0;
          dev_sync_slot[dev_idx] = n;
        }
      }
    }
    if (ptrack == nullptr) {
      continue;
    }
    ptrack->store_in_mem(gdt->mem_slot_idx);
  }
  //  //DEBUG_PRINTLN("cache finished");
  proj.select_grid(old_grid);
}

void MCLActions::calc_next_slot_transition(uint8_t n,
                                           bool ignore_chain_settings, bool auto_check) {


  // DEBUG_PRINT_FN();

  if (auto_check) {
   switch (chains[n].mode) {
    case LOAD_AUTO: {
      if (links[n].loops == 0) {
        next_transitions[n] = -1;
        return;
      }
      break;
    }
   }
  }

  if (!ignore_chain_settings) {
    switch (chains[n].mode) {
    case LOAD_QUEUE: {
      break;
    }
    case LOAD_AUTO: {
      if (links[n].loops == 0) {
        next_transitions[n] = -1;
        return;
      }
      break;
    }
    case LOAD_MANUAL: {
      next_transitions[n] = -1;
      return;
    }
    }
  }

    // next transition[n] already valid, use this.
  if (next_transitions[n] != -1 && next_transitions[n] > next_transition) {
    return;
  }

  uint8_t track_idx, dev_idx;

  GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
  if (gdt == nullptr) {
    return;
  }
  uint16_t next_transitions_old = next_transitions[n];

  float len;

  float l = links[n].length;
  len =
      (float)links[n].loops * l * (float)gdt->seq_track->get_speed_multiplier();
  while (len < 4) {
    if (len < 1) {
      len = 4;
      transition_offsets[n] = 0;
    } else {
      len = len * 2;
    }
  }

  // Last offset must be carried over to new offset.
  transition_offsets[n] += (float)(len - (uint16_t)(len)) * 12;
  if (transition_offsets[n] >= 12) {
    transition_offsets[n] = transition_offsets[n] - 12;
    len++;
  }

  // DEBUG_DUMP(len - (uint16_t)(len));
  // DEBUG_DUMP(transition_offsets[n]);
  next_transitions[n] += (uint16_t)len;

  // check for overflow and make sure next nearest step is greater than
  // midiclock counter
  while ((next_transitions[n] >= next_transitions_old) &&
         (next_transitions[n] < MidiClock.div16th_counter)) {
    next_transitions[n] += (uint16_t)len;
  }

  DEBUG_PRINT("slot ");
  DEBUG_PRINT(n);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(next_transitions[n]);
}

void MCLActions::calc_next_transition() {
  next_transition = (uint16_t)-1;
  // DEBUG_PRINT_FN();
  int8_t slot = -1;
  for (uint8_t n = 0; n < NUM_SLOTS; n++) {
    if (grid_page.active_slots[n] != SLOT_DISABLED) {
      if ((links[n].loops > 0)) {
        // && (links[n].row != grid_page.active_slots[n])) || links[n].length) {
        if (MidiClock.clock_less_than(next_transitions[n], next_transition)) {
          next_transition = next_transitions[n];
          slot = n;
        }
      }
    }
  }

  nearest_bar = next_transition / 16 + 1;
  nearest_bar = nearest_bar - (nearest_bar / 100) * 100;
  nearest_beat = next_transition % 4 + 1;
  // next_transition = next_transition % 16;

     DEBUG_PRINTLN(F("current_step"));
     DEBUG_PRINTLN(MidiClock.div16th_counter);
   DEBUG_PRINTLN(F("nearest step"));
    DEBUG_PRINTLN(next_transition);
}

void MCLActions::calc_latency() {
  EmptyTrack empty_track;

  MidiDevice *devs[2] = {
      midi_active_peering.get_device(UART1_PORT),
      midi_active_peering.get_device(UART2_PORT),
  };

  for (uint8_t a = 0; a < NUM_DEVS; a++) {
    dev_latency[a].latency = 0;
    dev_latency[a].div32th_latency = 0;
    dev_latency[a].div192th_latency = 0;

    if (dev_sync_slot[a] != 255) {
      dev_latency[a].latency += 2 + 7;
    }
    //  dev_latency[a].load_latency = 0;
  }

  bool send_dev[NUM_DEVS] = {0};

  uint8_t track_idx, dev_idx;

  uint8_t num_devices = 0;

  // DEBUG_PRINTLN("calc latency");
  // DEBUG_CHECK_STACK();
  for (uint8_t n = 0; n < NUM_SLOTS; n++) {
    if ((grid_page.active_slots[n] == SLOT_DISABLED))
      continue;
    if (next_transitions[n] == next_transition) {
      GridDeviceTrack *gdt = get_grid_dev_track(n, &track_idx, &dev_idx);
      if (gdt == nullptr) {
        continue;
      }
      if (send_machine[n] == 0) {
        // Optimised, assume we dont need to read the entire object to calculate
        // latency.
        auto *ptrack = empty_track.load_from_mem(
            gdt->mem_slot_idx, gdt->track_type, sizeof(GridTrack));
        //   uint16_t diff = clock_diff(old_clock, clock);
        if (ptrack == nullptr || !ptrack->is_active() ||
            gdt->track_type != ptrack->active) {
          continue;
        }
        // dev_latency[dev_idx].load_latency += diff;
        dev_latency[dev_idx].latency += ptrack->calc_latency(n);
      }
      if (send_dev[dev_idx] != true) {
        num_devices++;
      }
      send_dev[dev_idx] = true;
    }
  }

  float tempo = MidiClock.get_tempo();
  //  div32th_per_second: tempo / 60.0f * 4.0f * 2.0f = tempo * 8 / 60
  float div32th_per_second = tempo * 0.133333333333f;
  //  div32th_per_second: tempo / 60.0f * 4.0f * 2.0f * 6.0f = tempo * 8 / 10
  float div192th_per_second = tempo * 0.8f;

  div32th_total_latency = 0;
  div192th_total_latency = 0;

  if (mcl_cfg.uart2_prg_out > 0) {
    send_dev[1] = true;
  }
  for (uint8_t a = 0; a < NUM_DEVS; a++) {
    if (send_dev[a]) {
      float bytes_per_second_uart1 = devs[a]->uart->speed * 0.1f;
      float latency_in_seconds = (float)dev_latency[a].latency /
                                 bytes_per_second_uart1; // 25ms minimum.
      if (num_devices == 1) {
        latency_in_seconds += .10;
      } else if (a == 1) {
        latency_in_seconds += .20;
      }

      // latency_in_seconds += (float) dev_latency[a].load_latency * .0002;

      dev_latency[a].div32th_latency =
          floor(div32th_per_second * latency_in_seconds) + 1;
      dev_latency[a].div192th_latency =
          floor(div192th_per_second * latency_in_seconds) + 1;

      // Program change minimum delay = 1 x 16th.
      if (mcl_cfg.uart2_prg_out > 0 && a == 1) {
        if (dev_latency[a].div32th_latency < 2) {
          dev_latency[a].div32th_latency = 2;
          dev_latency[a].div192th_latency = 12;
        }
      }

      div32th_total_latency += dev_latency[a].div32th_latency;
      div192th_total_latency += dev_latency[a].div192th_latency;
    }
  }
  // DEBUG_PRINTLN("total latency");
  // DEBUG_PRINTLN(div32th_total_latency);
  // DEBUG_PRINTLN(div192th_total_latency);
}

MCLActions mcl_actions;
