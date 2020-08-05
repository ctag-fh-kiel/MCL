/* Justin Mammarella jmamma@gmail.com 2018 */
#ifndef GRIDTRACK_H__
#define GRIDTRACK_H__

#define A4_TRACK_TYPE_270 2
#define MD_TRACK_TYPE_270 1
#define EXT_TRACK_TYPE_270 3

#define MD_TRACK_TYPE 4
#define A4_TRACK_TYPE 5
#define EXT_TRACK_TYPE 6

#define EMPTY_TRACK_TYPE 0

#include "GridChain.h"

class GridTrack {
public:
  uint8_t active = EMPTY_TRACK_TYPE;
  char trackName[17];
  GridChain chain;
//  bool get_track_from_sysex(int tracknumber, uint8_t column);
//  void place_track_in_sysex(int tracknumber, uint8_t column);

  uint16_t get_track_size();
  bool load_track_from_grid(uint8_t column, uint16_t row);
  bool store_track_in_grid(uint8_t column, uint16_t row);
};

#endif /* GRIDTRACK_H__ */
