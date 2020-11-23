/* Justin Mammarella jmamma@gmail.com 2018 */

#ifndef MDSEQTRACKDATA_H__
#define MDSEQTRACKDATA_H__

#include "CommonTools/helpers.h"

#define NUM_LOCKS_270 4
#define NUM_MD_STEPS_270 64

#define NUM_MD_LOCK_SLOTS 256
#define NUM_MD_STEPS 64

class MDSeqTrackData_270 {
public:
  uint8_t length;
  uint8_t speed;
  uint32_t slide_mask32; // to be increased to 64bits
  uint8_t locks[NUM_LOCKS_270][NUM_MD_STEPS_270];
  uint8_t locks_params[NUM_LOCKS_270];
  uint64_t pattern_mask;
  uint64_t lock_mask;
  uint8_t conditional[NUM_MD_STEPS_270];
  uint8_t timing[NUM_MD_STEPS_270];
};

class MDSeqStepDescriptor {
public:
  uint8_t locks; // <-- bitfield of 8 locks in the current step, first lock is lsb
  bool locks_enabled : 1; // true: locks are enabled. false: locks are disabled, but still occupy lock slots
  bool trig : 1;
  bool slide : 1;
  bool cond_plock : 1;
  uint8_t cond_id : 4;
  bool is_lock_bit(const uint8_t idx) const { return locks & (1 << idx); }
  bool is_lock(const uint8_t idx) const { return is_lock_bit(idx) && locks_enabled; }
};

class MDSeqStep {
public:
  bool active;
  uint8_t locks[NUM_LOCKS];
  uint8_t timing;
  MDSeqStepDescriptor data;
};

class MDSeqTrackData {
public:
  uint8_t locks[NUM_MD_LOCK_SLOTS];
  uint8_t locks_params[NUM_LOCKS];
  uint8_t timing[NUM_MD_STEPS];
  MDSeqStepDescriptor steps[NUM_MD_STEPS];

  // get the pointer to the data chunk.
  // useful to skip the vtable
  void* data() const { return (void*) &locks; }

  FORCED_INLINE() uint8_t find_param(uint8_t param_id) const {
    param_id += 1;
    if (!param_id)
      return 255;
    for (uint8_t c = 0; c < NUM_LOCKS; c++) {
      if (locks_params[c] == param_id) {
        return c;
      }
    }
    return 255;
  }

  FORCED_INLINE() uint16_t get_lockidx(uint8_t step) const {
    uint16_t idx = 0;
    for (uint8_t i = 0; i < step; ++i) {
      idx += popcount(steps[i].locks);
    }
    return idx;
  }

  FORCED_INLINE() uint16_t get_lockidx(uint8_t step, uint8_t lock_idx) const {
    uint8_t mask = 1 << lock_idx;
    uint8_t rmask = (mask - 1);
    if (steps[step].is_lock_bit(lock_idx)) {
      auto idx = get_lockidx(step) + popcount(steps[step].locks & rmask);
      return idx;
    } else {
      return NUM_MD_LOCK_SLOTS;
    }
  }

  void init() { memset(this, 0, sizeof(MDSeqTrackData)); }
  bool convert(MDSeqTrackData_270 *old) {
    // TODO
    return false;
  }
};

#endif /* MDSEQTRACKDATA_H__ */
