/* Copyright 2018, Justin Mammarella jmamma@gmail.com */
#include "MDExploit.h"

void MDExploit::setup() {}
void MDExploit::setup_global(uint8_t global_num) {
  /** Original position of the global inside the MD (0 to 7). **/
  if (global_num == 0) {
    global_one.origPosition = 6;
  } else {
    global_one.origPosition = 7;
  }
  /** Stores the audio output for each track. **/

  for (uint8_t track_n = 0; track_n < 16; track_n++) {
    if (IS_BIT_SET32(cfg.cues, track_n)) {
      global_one.drumRouting[track_n] = 5;
    } else {
      global_one.drumRouting[track_n] = 6;
    }
  }

  // baseChannel
  // -- 0
  // 1-4 1
  // 5-8 2
  // 9-12 3
  // 13-16 4

  /** The MIDI base channel of the MachineDrum. **/
  if (global_num == 0) {
    global_one.baseChannel = 3;
  } else {
    global_one.baseChannel = 9;
  }

  global_one.extendedMode = true;
  if (MidiClock.mode == MidiClock.EXTERNAL_MIDI) {
    global_one.clockIn = false;
    global_one.clockOut = true;
  } else {
    global_one.clockIn = true;
    global_one.clockOut = false;
  }
  global_one.transportIn = true;
  // some bug
  global_one.transportOut = true;
  global_one.localOn = 1;
  global_one.programChange = 2;
}
void MDExploit::send_globals() {
  if (rec_global) {
    ElektronDataToSysexEncoder encoder(&MidiUart);
    ElektronDataToSysexEncoder encoder2(&MidiUart);
    setup_global(0);
    in_sysex = 1;
    global_one.toSysex(encoder);
    setup_global(1);
    global_one.toSysex(encoder2);
    in_sysex = 0;
  }
}

void MDExploit::switch_global(uint8_t global_page) {

  uint8_t data[] = {0x56, (uint8_t)global_page & 0x7F};
  in_sysex = 1;
  MD.sendSysex(data, countof(data));
  in_sysex = 0;
}
bool MDExploit::on() {
  // DEBUG_PRINTLN("Exploit on");
  //  DEBUG_PRINTLN(slowclock);

  // in_sysex = 1;
  noteinterface.note_proceed = false;
  if (state) {
    start_clock = slowclock;
    return false;
  }
  state = true;
  // last_md_track = MD.getCurrentTrack(CALLBACK_TIMEOUT);
  last_md_track = MD.currentTrack;
  MD.setStatus(0x22, 15);
  // MD.getBlockingGlobal(0);
  noteinterface.init_notes();

  /*if (MidiClock.state == 2) {

    div16th_last = MidiClock.div16th_counter;
    noteproceed = 0;
    }
    else {
    noteproceed = 1;
    }
  */
  noteinterface.notecount = 0;
  //   global_new.baseChannel = 9;
  //  ElektronDataToSysexEncoder encoder(&MidiUart);
  //   global_new.toSysex(encoder);
  //    MD.setTempo(MidiClock.tempo);

  int flag = 0;

  //     if ((MidiClock.state == 2) && (MidiClock.mode ==
  //     MidiClock.EXTERNAL_UART2)) {
  if ((MidiClock.state == 2) && (MidiClock.mode == MidiClock.EXTERNAL_UART2)) {
    flag = 1;
  }

  if (flag == 1) {
    MidiUart.m_putc_immediate(MIDI_STOP);
  }
  //   }

  MD.global.baseChannel = 4;

  switch_global(6);

  //     if ((MidiClock.state == 2) &&  (MidiClock.mode ==
  //     MidiClock.EXTERNAL_UART2)) {
  if (flag == 1) {
    MidiUart.m_putc_immediate(MIDI_CONTINUE);
  }
  //    }
  //  MD.getBlockingStatus(MD_CURRENT_GLOBAL_SLOT_REQUEST,200);
  start_clock = slowclock;
  return true;
  // note_interface.state = true;
  // in_sysex = 0;
}
bool MDExploit::off() {
  noteinterface.note_proceed = false;
  // DEBUG_PRINTLN("Exploit off");
  //     DEBUG_PRINTLN(slowclock);
  // in_sysex = 1;
  if (!state) {
    return false;
  }
  note_interface.state = false;

  //
  //  global_new.tempo = MidiClock.tempo;
  //   global_new.baseChannel = 3;
  //    ElektronDataToSysexEncoder encoder(&MidiUart);
  //   global_new.toSysex(encoder);
  int flag = 0;
  if ((MidiClock.state == 2) && (MidiClock.mode == MidiClock.EXTERNAL_UART2)) {
    flag = 1;
  }
  if (flag == 1) {
    MidiUart.m_putc_immediate(MIDI_STOP);
  }
  //   }
  MD.global.baseChannel = 9;

  switch_global(7);
  //    if ((MidiClock.state == 2) && (MidiClock.mode ==
  //    MidiClock.EXTERNAL_UART2)) {
  if (flag == 1) {
    MidiUart.m_putc_immediate(MIDI_CONTINUE);
  }
  //   }
  if (grid.cur_col < 16) {
    MD.setStatus(0x22, grid.cur_col);

  } else {
    MD.setStatus(0x22, last_md_track);
  }
  state = false;
  return true;
  // in_sysex = 0;
}

void MDExploitMidiEvents::setup_callbacks() {
  if (state) {
    return;
  }
  Midi.addOnNoteOnCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onNoteOnCallback);
  Midi.addOnNoteOffCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onNoteOffCallback);
  Midi.addOnControlChangeCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onControlChangeCallback);

  state = true;
}

void MDExploitMidiEvents::remove_callbacks() {

  if (!state) {
    return;
  }
  Midi.removeOnNoteOnCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onNoteOnCallback);
  Midi.removeOnNoteOffCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onNoteOffCallback);
  Midi.removeOnControlChangeCallback(
      this, (midi_callback_ptr_t)&MDExploitMidiEvents::onControlChangeCallback);

  state = false;
}
void MDExploitCallbacks::setup_callbacks() {
  if (state) {
    return;
  }
  MidiClock.addOnMidiStartCallback(
      this, (midi_clock_callback_ptr_t)&TrigInterface::onMidiStartCallback);
  MidiClock.addOnMidiContinueCallback(
      this, (midi_clock_callback_ptr_t)&TrigInterface::onMidiStartCallback);

  state = true;
};
void MDExploitCallbacks::remove_callbacks() {
  if (!state) {
    return;
  }
  MidiClock.removeOnMidiStartCallback(
      this, (midi_clock_callback_ptr_t)&TrigInterface::onMidiStartCallback);
  MidiClock.removeOnMidiContinueCallback(
      this, (midi_clock_callback_ptr_t)&TrigInterface::onMidiStartCallback);

  state = false;
};

uint8_t MDExploitMidiEvents::note_to_trig(uint8_t note_num) {

  uint8_t trig_num = 0;
  for (uint8_t i = 0; i < sizeof(MD.global.drumMapping); i++) {
    if (note_num == MD.global.drumMapping[i]) {
      trig_num = i;
    }
  }
  return trig_num;
}
void MDExploitMidiEvents::onNoteOnCallback(uint8_t *msg) {
  if (!md_exploit.note_proceed) {
    uint16_t current_clock = slowclock;

    if (clock_diff(md_exploit.start_clock, current_clock) >
        EXPLOIT_DELAY_TIME) {
      md_exploit.note_proceed = true;
    }
  }

  if (md_exploit.note_proceed) {
    if ((msg[0] == 153)) {
      note_inteface.note_on_event(note_to_trig(msg[1]));
    }
  }
}
void MDExploitMidiEvents::onNoteOffCallback(uint8_t *msg) {
  if (md_exploit.note_proceed) {
    if ((msg[0] == 153)) {
      note_inteface.note_off_event(note_to_trig(msg[1]));
    }
  }
}


void MDExploitCallbacks::onNoteOffCallback(uint8_t *msg) {}

void MDExploitCallbacks::onControlChangeCallback(uint8_t *msg) {
 md_exploit.off();
        GUI.setPage(&grid_page);
}

void MDExploitCallbacks::onMidiStartCallback() {
  //     if ((curpage == S_PAGE) || (curpage == W_PAGE) || (curpage == CUE_PAGE)
  //     || (curpage == MIXER_PAGE)) {
  exploit_start_clock = slowclock;
  md_exploit.note_proceed = false;
  // }
  pattern_start_clock32th = 0;
}

MDExploit md_exploit;