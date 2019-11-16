
#include "A4Messages.hh"
#include "A4Params.hh"
#include "Elektron.hh"
#include "helpers.h"

#include "A4.h"

bool A4Global::fromSysex(uint8_t *data, uint16_t len) {
  if (len != 0xC4 - 6) {
    //		printf("wrong length\n");
    // wrong length
    return false;
  }

  if (!ElektronHelper::checkSysexChecksum(data, len)) {
    //		printf("wrong checksum\n");
    return false;
  }

  //	origPosition = data[3];
  ElektronSysexDecoder decoder(DATA_ENCODER_INIT(data + 4, len - 4));

  return true;
}

uint16_t A4Global::toSysex(uint8_t *data, uint16_t len) {
  ElektronDataToSysexEncoder encoder(DATA_ENCODER_INIT(data, len));
  return toSysex(encoder);
  if (len < 0xC5)
    return 0;
}

uint16_t A4Global::toSysex(ElektronDataToSysexEncoder &encoder) {
  encoder.stop7Bit();
  encoder.begin();
  encoder.pack(a4_sysex_hdr, sizeof(a4_sysex_hdr));
  encoder.pack8(A4_GLOBAL_MESSAGE_ID);
  encoder.pack8(0x05); // version
  encoder.pack8(0x01); // revision

  encoder.startChecksum();
  // encoder.pack8(origPosition);

  uint16_t enclen = encoder.finish();
  encoder.finishChecksum();

  return enclen + 5;
}

/**
 * 0xF0 [8B prologue] [1B ORIGPOS] 
 * <begin checksum> [8B header] <begin 7bit enc> 
 * {1B BEGIN SOUND} {4B 32-TAGS} {15B + nul NAME}
 * <begin parameters>
 */

static constexpr uint8_t a4sound_prologue[8] =  { 0x00, 0x20, 0x3c, 0x06, 0x00, 0x53, 0x01, 0x01 };
static constexpr uint8_t a4soundx_prologue[8] = { 0x00, 0x20, 0x3c, 0x06, 0x00, 0x59, 0x01, 0x01 };
static constexpr uint8_t a4sound_header[8] = { 0x78, 0x3e, 0x6f, 0x3a, 0x3a, 0x00, 0x00, 0x00 };
static constexpr uint8_t a4sound_footer[4] =  { 0x3a, 0x3e, 0x7a, 0x4e };

static constexpr size_t a4sound_sysex_len = 415 - 2; // 2 for sysex frame
static constexpr size_t a4sound_origpos_idx = sizeof(a4sound_prologue);
static constexpr size_t a4sound_checksum_startidx = a4sound_origpos_idx + 1;
static constexpr size_t a4sound_encoding_startidx = a4sound_checksum_startidx + sizeof(a4sound_header);

// caller guarantees: 1. in checksum; 2. not in 7bit enc.
// when this routine exits, condition 1) and 2) hold.
bool A4Sound::fromSysex_impl(ElektronSysexDecoder &decoder) {
  decoder.start7Bit();
  decoder.skip(1); // skip sound header 0x05
  decoder.get(tags);
  decoder.get(name);
  decoder.get(sound);
  decoder.skip(sizeof(a4sound_footer));
  decoder.stop7Bit();
}

// caller guarantees: 1. in checksum; 2. not in 7bit enc.
// when this routine exits, condition 1) and 2) hold.
void A4Sound::toSysex_impl(ElektronDataToSysexEncoder &encoder)
{
  encoder.pack(a4sound_header);
  encoder.start7Bit();
  encoder.pack8(0x05);
  encoder.pack(tags);
  encoder.pack(name);
  encoder.pack(sound);
  encoder.pack(a4sound_footer);
  encoder.stop7Bit();
}

///  !Note, sysex frame wrappers are not included in [data..data+len)
///  !Note, first effective payload byte is origPosition @ 0x08.
///  !Note, checksum starts at 0x09 without origPosition.
bool A4Sound::fromSysex(uint8_t *data, uint16_t len) {
  if (len != a4sound_sysex_len) {
    GUI.setLine(GUI.LINE1);
    GUI.flash_strings_fill("WRONG LEN", "");
    GUI.setLine(GUI.LINE2);

    GUI.put_value16(0, len);
    //  GUI.put_value16(8, 384 - 10 - 2 -2 -1);

    return false;
  }

  if (!ElektronHelper::checkSysexChecksumAnalog(
    data + a4sound_checksum_startidx,
    len - a4sound_checksum_startidx)) {
    GUI.flash_strings_fill("WRONG CKSUM", "");
    return false;
  }

  origPosition = data[a4sound_origpos_idx];
  ElektronSysexDecoder decoder(
    DATA_ENCODER_INIT(
      data + a4sound_encoding_startidx,
      len - a4sound_encoding_startidx - 4));

  return fromSysex_impl(decoder);
}

bool A4Sound::fromSysex(MidiClass *midi) {
  const auto &reclen = midi->midiSysex.recordLen;

  // len / offset: checksum'ed part
  uint16_t len = reclen - a4sound_checksum_startidx;
  uint16_t offset = a4sound_checksum_startidx;
  if (reclen != a4sound_sysex_len) {
    GUI.setLine(GUI.LINE1);
    GUI.flash_strings_fill("WRONG LEN", "");
    GUI.setLine(GUI.LINE2);

    GUI.put_value16(0, len);
    //  GUI.put_value16(8, 384 - 10 - 2 -2 -1);

    return false;
  }

  if (!ElektronHelper::checkSysexChecksumAnalog(midi, a4sound_checksum_startidx, len)) {
    GUI.flash_strings_fill("WRONG CKSUM", "");
    return false;
  }

  origPosition = midi->midiSysex.getByte(a4sound_origpos_idx);
  ElektronSysexDecoder decoder(DATA_ENCODER_INIT(midi, offset, len - 4));

  return fromSysex_impl(decoder);
}

uint16_t A4Sound::toSysex() {
  ElektronDataToSysexEncoder encoder(&MidiUart2);
  return toSysex(encoder);
}

uint16_t A4Sound::toSysex(uint8_t *data, uint16_t len) {
  ElektronDataToSysexEncoder encoder(DATA_ENCODER_INIT(data, len));
  if (len < 0xC5) // what is 0xC5?
    return 0;
  return toSysex(encoder);
}

uint16_t A4Sound::toSysex(ElektronDataToSysexEncoder &encoder) {
  encoder.stop7Bit();
  encoder.begin();
  if (!soundpool) {
    encoder.pack(a4soundx_prologue);
  } else {
    encoder.pack(a4sound_prologue);
  }
  encoder.pack8(origPosition);
  encoder.startChecksum();
  toSysex_impl(encoder);
  uint16_t enclen = encoder.finish();
  encoder.finishChecksum();
  return enclen + 5;
}

bool A4Kit::fromSysex(uint8_t *data, uint16_t len) {
  if (len != (2679 - 10 - 0)) {
    GUI.setLine(GUI.LINE1);
    GUI.flash_strings_fill("WRONG LEN", "");
    GUI.setLine(GUI.LINE2);

    GUI.put_value16(0, len);
    //  GUI.put_value16(8, 384 - 10 - 2 -2 -1);

    return false;
  }

  if (!ElektronHelper::checkSysexChecksumAnalog(data + 1, len - 1)) {
    GUI.flash_strings_fill("WRONG CKSUM", "");
    return false;
  }
  ElektronSysexDecoder decoder(DATA_ENCODER_INIT(data, len - 4));
  decoder.stop7Bit();
  decoder.get8(&origPosition);
  //    decoder.skip(2);
  decoder.get((uint8_t *)&payload_start, sizeof(payload_start));

  for (uint8_t i = 0; i < 4; i++) {
    sounds[i].fromSysex_impl(decoder);
    sounds[i].origPosition = i;
  }
  decoder.get((uint8_t *)&payload_end, sizeof(payload_end));

  return true;
}

uint16_t A4Kit::toSysex() {
  ElektronDataToSysexEncoder encoder(&MidiUart2);
  return toSysex(encoder);
}
uint16_t A4Kit::toSysex(uint8_t *data, uint16_t len) {
  ElektronDataToSysexEncoder encoder(DATA_ENCODER_INIT(data, len));
  if (len < 0xC5)
    return 0;
  return toSysex(encoder);
}

uint16_t A4Kit::toSysex(ElektronDataToSysexEncoder &encoder) {
  encoder.stop7Bit();
  encoder.begin();
  encoder.pack(a4_sysex_hdr, sizeof(a4_sysex_hdr));
  if (workSpace) {
    encoder.pack8(A4_KITX_MESSAGE_ID);
  } else {
    encoder.pack8(A4_KIT_MESSAGE_ID);
  }
  encoder.pack(a4_sysex_proto_version, sizeof(a4_sysex_proto_version));
  encoder.pack8(origPosition);

  encoder.startChecksum();
  encoder.pack(payload_start, sizeof(payload_start)); // unknow
  for (uint8_t i = 0; i < 4; i++) {
    sounds[i].toSysex_impl(encoder);
  }
  encoder.pack(payload_end, sizeof(payload_end));
  uint16_t enclen = encoder.finish();
  encoder.finishChecksum();

  return enclen + 5;
}
