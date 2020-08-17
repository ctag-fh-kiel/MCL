#include "A4Track.h"
#include "MCL.h"
#include "MCLSeq.h"
//#include "MCLSd.h"

bool A4Track::get_track_from_sysex(uint8_t tracknumber) {

  active = A4_TRACK_TYPE;
}

bool A4Track::store_track_in_grid(uint8_t tracknumber, uint16_t row,
                                  bool online) {
  active = A4_TRACK_TYPE;

  bool ret;
  int b = 0;
  DEBUG_PRINT_FN();
  DEBUG_PRINTLN("storing a4 track");
  uint32_t len;

  /*analog 4 tracks*/
#ifdef EXT_TRACKS
  if (online) {
    if (Analog4.connected) {
      if (tracknumber != 255) {
        get_track_from_sysex(tracknumber);
      }
    }
    memcpy(&seq_data, &mcl_seq.ext_tracks[tracknumber], sizeof(seq_data));

    chain.length = mcl_seq.ext_tracks[tracknumber].length;
    chain.speed = mcl_seq.ext_tracks[tracknumber].speed;
  }
#endif
  ret = proj.write_grid((uint8_t *)this, A4_TRACK_LEN, tracknumber, row);
  if (!ret) {
    return false;
  }
  return true;
}
