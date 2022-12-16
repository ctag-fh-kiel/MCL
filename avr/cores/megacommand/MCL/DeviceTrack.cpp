#include "MCL_impl.h"

DeviceTrack *DeviceTrack::init_track_type(uint8_t track_type) {
  switch (track_type) {
  default:
  case EMPTY_TRACK_TYPE:
    ::new (this) EmptyTrack;
    break;
  case MD_TRACK_TYPE:
    ::new (this) MDTrack;
    break;
  case A4_TRACK_TYPE:
    ::new (this) A4Track;
    break;
  case EXT_TRACK_TYPE:
    ::new (this) ExtTrack;
    break;
  case MDFX_TRACK_TYPE:
    ::new (this) MDFXTrack;
    break;
  case MDTEMPO_TRACK_TYPE:
    ::new (this) MDTempoTrack;
    break;
  case MDROUTE_TRACK_TYPE:
    ::new (this) MDRouteTrack;
    break;
  case MDLFO_TRACK_TYPE:
    ::new (this) MDLFOTrack;
    break;
  case MNM_TRACK_TYPE:
    ::new (this) MNMTrack;
    break;
  case GRIDCHAIN_TRACK_TYPE:
    ::new (this) GridChainTrack;
    break;
  case NULL_TRACK_TYPE:
    // Dummy init
    break;
  }
  return this;
}

DeviceTrack *DeviceTrack::load_from_grid(uint8_t column, uint16_t row) {
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

  if (!proj.read_grid(ptrack, len, column, row)) {
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

bool DeviceTrackChunk::load_from_mem_chunk(uint8_t column, uint8_t chunk) {
  size_t chunk_size = sizeof(seq_data_chunk);
  uint32_t offset = (uint32_t)seq_data_chunk - (uint32_t)this;
  uint32_t pos = get_region() + get_track_size() * (uint32_t)(column) + offset +
                 chunk_size * chunk;
  volatile uint8_t *ptr = reinterpret_cast<uint8_t *>(pos);
  memcpy_bank1(seq_data_chunk, ptr, chunk_size);
  return true;
}
bool DeviceTrackChunk::load_chunk(volatile void *ptr, uint8_t chunk) {
  size_t chunk_size = sizeof(seq_data_chunk);
  if (chunk == get_chunk_count() - 1) {
    chunk_size = get_seq_data_size() - sizeof(seq_data_chunk) * chunk;
  }
  memcpy(ptr + sizeof(seq_data_chunk) * chunk, seq_data_chunk, chunk_size);
  return true;
}

bool DeviceTrackChunk::load_link_from_mem(uint8_t column) {
  uint32_t pos = get_region() + get_track_size() * (uint32_t)(column) + (uint32_t) &this->link - (uint32_t) this;
  volatile uint8_t *ptr = reinterpret_cast<uint8_t *>(pos);
  memcpy_bank1(&this->link, ptr, sizeof(GridTrack));
  return true;
}
