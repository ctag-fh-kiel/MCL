#include "MCL_impl.h"

DeviceTrack* DeviceTrack::init_track_type(uint8_t track_type) {
  switch (track_type) {
  default:
  case EMPTY_TRACK_TYPE:
    ::new(this) EmptyTrack;
    break;
  case MD_TRACK_TYPE:
    ::new(this) MDTrack;
    break;
  case A4_TRACK_TYPE:
    ::new(this) A4Track;
    break;
  case EXT_TRACK_TYPE:
    ::new(this) ExtTrack;
    break;
  case MDFX_TRACK_TYPE:
    ::new(this) MDFXTrack;
    break;
  case MDTEMPO_TRACK_TYPE:
    ::new(this) MDTempoTrack;
    break;
  case MDROUTE_TRACK_TYPE:
    ::new(this) MDRouteTrack;
    break;
  case MDLFO_TRACK_TYPE:
    ::new(this) MDLFOTrack;
    break;
  case MNM_TRACK_TYPE:
    ::new(this) MNMTrack;
    break;
  case GRIDCHAIN_TRACK_TYPE:
    ::new(this) GridChainTrack;
    break;
  case NULL_TRACK_TYPE:
    //Dummy init
    break;
  }
  return this;
}

DeviceTrack* DeviceTrack::load_from_grid(uint8_t column, uint16_t row) {
  DEBUG_PRINTLN(F("lfg"));
  if (!GridTrack::load_from_grid(column, row)) {
    return nullptr;
  }

  // header read successfully. now reconstruct the object.
  auto ptrack = init_track_type(active);

  if (ptrack == nullptr) {
    DEBUG_PRINTLN("unrecognized track type");
    return nullptr;
  }

  // virtual functions are ready
  uint32_t len = ptrack->get_track_size();

  if(!proj.read_grid(ptrack, len, column, row)) {
    DEBUG_PRINTLN(F("read failed"));
    return nullptr;
  }

  auto ptrack2 = ptrack->init_track_type(active);
  if (ptrack2 == nullptr) {
    DEBUG_PRINTLN("unrecognized track type 2");
    return nullptr;
  }

  return ptrack2;
}

