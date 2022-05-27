#include "MCL_impl.h"

void mclsys_apply_config() {
  DEBUG_PRINT_FN();
#ifndef DEBUGMODE
#ifdef MEGACOMMAND
  Serial.end();
  if (mcl_cfg.usb_mode == USB_DFU) {
    mcl_cfg.usb_mode = USB_SERIAL;
    if (mcl_gui.wait_for_confirm("USB", "Enter DFU and reboot?")) {
      SET_USB_MODE(USB_DFU);
      while (1);
    }
  }

  if (mcl_cfg.usb_mode <= USB_SERIAL) {
    SET_USB_MODE(mcl_cfg.usb_mode);
  }
  if (mcl_cfg.usb_mode == USB_SERIAL) {
    if ((!Serial) && (mcl_cfg.display_mirror == 1)) {
      GUI.display_mirror = true;
      MidiUartUSB.mode = UART_SERIAL;
      Serial.begin(SERIAL_SPEED);
    }
    if ((Serial) && (mcl_cfg.display_mirror == 0)) {
      GUI.display_mirror = false;
    }
  }
#endif
#endif

  mcl_cfg.write_cfg();
}

void mclsys_apply_config_midi() {
  mclsys_apply_config();
  midi_setup.cfg_ports();
}

bool MCLSysConfig::write_cfg() {
  bool ret;
  int b;

  DEBUG_PRINT_FN();
  DEBUG_PRINTLN(F("Writing cfg"));
  cfgfile.close();
  ret = cfgfile.open("/config.mcls", O_RDWR);
  if (!ret) {
    DEBUG_PRINTLN(F("Open cfg file failed"));
    return false;
  }

  ret = mcl_sd.write_data((uint8_t *)this, sizeof(MCLSysConfigData), &cfgfile);
  if (!ret) {
    DEBUG_PRINTLN(F("Write cfg failed"));
  }
  DEBUG_PRINTLN(F("Write cfg okay"));
  cfgfile.close();
  cfg_save_lastclock = slowclock;
  return true;
}

bool MCLSysConfig::cfg_init() {
  bool ret;
  int b;

  DEBUG_PRINT_FN();
  DEBUG_PRINTLN(F("Initialising cfgfile"));

  // DEBUG_PRINTLN(F("conf ext"));
  cfgfile.remove();
  ret = cfgfile.createContiguous("/config.mcls", (uint32_t)GRID_SLOT_BYTES);
  if (ret) {
    DEBUG_PRINTLN(F("Created new cfgfile"));
  } else {
    DEBUG_PRINTLN(F("Failed to create new cfgfile"));
    return false;
  }

  char my_string[16] = "/project000.mcl";

  version = CONFIG_VERSION;
  number_projects = 0;
  strncpy(project, my_string, 16);
  clock_send = 0;
  clock_rec = 0;
  uart1_turbo = 3;
  uart2_turbo = 3;
  col = 0;
  row = 0;
  cur_row = 0;
  cur_col = 0;
  memset(&routing, 6, sizeof(routing));
  poly_mask = 0;
  uart2_ctrl_chan = MIDI_LOCAL_MODE;
  uart2_poly_chan = MIDI_LOCAL_MODE;
  uart2_prg_in = MIDI_LOCAL_MODE;
  uart2_prg_out = MIDI_LOCAL_MODE;
  mutes = 0;
  display_mirror = 0;
  rec_quant = 0;
  tempo = 125;
  midi_forward = 0;
  auto_save = 1;
  auto_normalize = 1;
  load_mode = LOAD_MANUAL;
  chain_queue_length = 1;
  chain_load_quant = 4;
  ram_page_mode = 0;
  track_select = 1;
  track_type_select = 0b00000101;
  uart2_device = 0;
  uart_cc_loopback = 0;
  uart2_prg_mode = 0;
  usb_mode = USB_SERIAL;
  cfgfile.close();

  ret = write_cfg();
  if (!ret) {
    return false;
  }
  return true;
}

MCLSysConfig mcl_cfg;
