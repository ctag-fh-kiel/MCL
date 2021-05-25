#include "Elektron.h"

model_param_name_t md_model_param_names[] = {
    // 0 - gnd_sn_model_names
    {"PT1", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"PT2", 4},
    {"PT3", 5},
    {"PT4", 6},
    {"UNI", 7},
    {"", 127},
    // 9 - gnd_ns_model_names
    {"DEC", 0},
    {"", 127},
    // 11 - gnd_im_model_names
    {"UP", 0},
    {"UVL", 1},
    {"DWN", 2},
    {"DVL", 3},
    {"", 127},
    // 16 - gnd_sw_model_names
    {"PT1", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"PT2", 4},
    {"PT3", 5},
    {"SKE", 6},
    {"UNI", 7},
    {"", 127},
    // 25 - gnd_pu_model_names
    {"PT1", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"PT2", 4},
    {"PT3", 5},
    {"WID", 6},
    {"UNI", 7},
    {"", 127},
    // 34 - trx_bd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"STR", 4},
    {"NS", 5},
    {"HRM", 6},
    {"CLP", 7},
    {"", 127},
    // 43 - trx_b2_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"HLD", 3},
    {"TCK", 4},
    {"NS", 5},
    {"DRT", 6},
    {"DST", 7},
    {"", 127},
    // 52 - trx_s2_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"NS", 2},
    {"NDE", 3},
    {"PWR", 4},
    {"TUN", 5},
    {"NTU", 6},
    {"NTY", 7},
    {"", 127},
    // 61 - trx_sd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"BMP", 2},
    {"BNV", 3},
    {"SNP", 4},
    {"TON", 5},
    {"TUN", 6},
    {"CLP", 7},
    {"", 127},
    // 70 - trx_xt_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"DMP", 4},
    {"DST", 5},
    {"DTP", 6},
    {"", 127},
    // 78 - trx_cp_model_names
    {"CLP", 0},
    {"TON", 1},
    {"HRD", 2},
    {"RCH", 3},
    {"RAT", 4},
    {"ROM", 5},
    {"RSZ", 6},
    {"RTN", 7},
    {"", 127},
    // 87 - trx_rs_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"DST", 2},
    {"", 127},
    // 91 - trx_cb_model_names
    {"PTC", 0},
    {"DEC", 2},
    {"ENH", 3},
    {"DMP", 4},
    {"TON", 5},
    {"BMP", 6},
    {"", 127},
    // 98 - trx_ch_model_names
    {"GAP", 0},
    {"DEC", 1},
    {"HPF", 2},
    {"LPF", 3},
    {"MTL", 4},
    {"", 127},
    // 104 - trx_oh_model_names
    {"GAP", 0},
    {"DEC", 1},
    {"HPF", 2},
    {"LPF", 3},
    {"MTL", 4},
    {"", 127},
    // 110 - trx_cy_model_names
    {"RCH", 0},
    {"DEC", 1},
    {"TOP", 2},
    {"TTU", 3},
    {"SZ", 4},
    {"PK", 5},
    {"", 127},
    // 117 - trx_ma_model_names
    {"ATT", 0},
    {"SUS", 1},
    {"REV", 2},
    {"DMP", 3},
    {"RTL", 4},
    {"RTP", 5},
    {"TON", 6},
    {"HRD", 7},
    {"", 127},
    // 126 - trx_cl_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"DUA", 2},
    {"ENH", 3},
    {"TUN", 4},
    {"CLC", 5},
    {"", 127},
    // 133 - trx_xc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"DMP", 4},
    {"DST", 5},
    {"DTP", 6},
    {"", 127},
    // 141 - efm_bd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"MFB", 7},
    {"", 127},
    // 150 - efm_sd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"NS", 2},
    {"NDC", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"HPF", 7},
    {"", 127},
    // 159 - efm_xt_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"RMP", 2},
    {"RDC", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"CLC", 7},
    {"", 127},
    // 168 - efm_cp_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"CLP", 2},
    {"CDC", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"HPF", 7},
    {"", 127},
    // 177 - efm_rs_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"MOD", 2},
    {"HPF", 3},
    {"SNR", 4},
    {"SPT", 5},
    {"SDC", 6},
    {"SMD", 7},
    {"", 127},
    // 186 - efm_cb_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"SNP", 2},
    {"FB", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"", 127},
    // 194 - efm_hh_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"TRM", 2},
    {"TFQ", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"FB", 7},
    {"", 127},
    // 203 - efm_cy_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"FB", 2},
    {"HPF", 3},
    {"MOD", 4},
    {"MFQ", 5},
    {"MDC", 6},
    {"", 127},
    // 211 - e12_bd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"SNP", 2},
    {"SPL", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 220 - e12_sd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"RNG", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 229 - e12_ht_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 238 - e12_lt_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"RNG", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 247 - e12_cp_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 256 - e12_rs_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"RTL", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 265 - e12_cb_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 274 - e12_ch_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 283 - e12_oh_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"STP", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 292 - e12_rc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"BEL", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 301 - e12_cc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 310 - e12_br_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"REL", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 319 - e12_ta_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 328 - e12_tr_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"HPQ", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 337 - e12_sh_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"SLW", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 346 - e12_bc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HP", 2},
    {"BC", 3},
    {"STR", 4},
    {"RTG", 5},
    {"RTM", 6},
    {"BND", 7},
    {"", 127},
    // 355 - p_i_bd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"HMR", 3},
    {"TNS", 4},
    {"DMP", 5},
    {"", 127},
    // 362 - p_i_sd_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"TNS", 3},
    {"RVL", 4},
    {"RDC", 5},
    {"RNG", 6},
    {"", 127},
    // 370 - p_i_mt_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"HMR", 3},
    {"TUN", 4},
    {"DMP", 5},
    {"SZ", 6},
    {"POS", 7},
    {"", 127},
    // 379 - p_i_ml_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"TNS", 3},
    {"", 127},
    // 384 - p_i_ma_model_names
    {"GRN", 0},
    {"DEC", 1},
    {"GLN", 2},
    {"SZ", 4},
    {"HRD", 5},
    {"", 127},
    // 390 - p_i_rs_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"RNG", 3},
    {"RVL", 4},
    {"RDC", 5},
    {"", 127},
    // 397 - p_i_rc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"RNG", 3},
    {"AG", 4},
    {"AU", 5},
    {"BR", 6},
    {"GRB", 7},
    {"", 127},
    // 406 - p_i_cc_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HRD", 2},
    {"RNG", 3},
    {"AG", 4},
    {"AU", 5},
    {"BR", 6},
    {"GRB", 7},
    {"", 127},
    // 415 - p_i_hh_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"CLS", 2},
    {"RNG", 3},
    {"AG", 4},
    {"AU", 5},
    {"BR", 6},
    {"CLS", 7},
    {"", 127},
    // 424 - inp_ga_model_names
    {"VOL", 0},
    {"GAT", 1},
    {"ATK", 2},
    {"HLD", 3},
    {"DEC", 4},
    {"", 127},
    // 430 - inp_fa_model_names
    {"ALV", 0},
    {"GAT", 1},
    {"FAT", 2},
    {"FHL", 3},
    {"FDC", 4},
    {"FDP", 5},
    {"FFQ", 6},
    {"FQ", 7},
    {"", 127},
    // 439 - inp_ea_model_names
    {"AVL", 0},
    {"AHL", 1},
    {"ADC", 2},
    {"FQ", 3},
    {"FDP", 4},
    {"FHL", 5},
    {"FDC", 6},
    {"FFQ", 7},
    {"", 127},
    // 448 - mid_model_names
    {"NOT", 0},
    {"N2", 1},
    {"N3", 2},
    {"LEN", 3},
    {"VEL", 4},
    {"PB", 5},
    {"MW", 6},
    {"AT", 7},
    {"C1D", 8},
    {"C1V", 9},
    {"C2D", 10},
    {"C2V", 11},
    {"C3D", 12},
    {"C3V", 13},
    {"C4D", 14},
    {"C4V", 15},
    {"C5D", 16},
    {"C5V", 17},
    {"C6D", 18},
    {"C6V", 19},
    {"PCG", 20},
    {"LFS", 21},
    {"LFD", 22},
    {"LFM", 23},
    {"", 127},
    // 473 - ctr_al_model_names
    {"SN1", 0},
    {"SN2", 1},
    {"SN3", 2},
    {"SN4", 3},
    {"SN5", 4},
    {"SN6", 5},
    {"SN7", 6},
    {"SN8", 7},
    {"", 127},
    // 482 - ctr_8p_model_names
    {"P1", 0},
    {"P2", 1},
    {"P3", 2},
    {"P4", 3},
    {"P5", 4},
    {"P6", 5},
    {"P7", 6},
    {"P8", 7},
    {"P1T", 8},
    {"P1P", 9},
    {"P2T", 10},
    {"P2P", 11},
    {"P3T", 12},
    {"P3P", 13},
    {"P4T", 14},
    {"P4P", 15},
    {"P5T", 16},
    {"P5P", 17},
    {"P6T", 18},
    {"P6P", 19},
    {"P7T", 20},
    {"P7P", 21},
    {"P8T", 22},
    {"P8P", 23},
    {"", 127},
    // 507 - ctr_re_model_names
    {"TIM", 0},
    {"MOD", 1},
    {"MFQ", 2},
    {"FB", 3},
    {"FLF", 4},
    {"FLW", 5},
    {"MON", 6},
    {"LEV", 7},
    // 515 - ctr_gb_model_names
    {"DVL", 0},
    {"PRE", 1},
    {"DEC", 2},
    {"DMP", 3},
    {"HP", 4},
    {"LP", 5},
    {"GAT", 6},
    {"LEV", 7},
    // 523 - ctr_eq_model_names
    {"LF", 0},
    {"LG", 1},
    {"HF", 2},
    {"HG", 3},
    {"PF", 4},
    {"PG", 5},
    {"PQ", 6},
    {"GAI", 7},
    // 531 - ctr_dx_model_names
    {"ATK", 0},
    {"REL", 1},
    {"THR", 2},
    {"RAT", 3},
    {"KNE", 4},
    {"HP", 5},
    {"GAI", 6},
    {"MIX", 7},
    // 539 - rom_model_names
    {"PTC", 0},
    {"DEC", 1},
    {"HLD", 2},
    {"BRR", 3},
    {"STR", 4},
    {"END", 5},
    {"RTG", 6},
    {"RTM", 7},
    {"", 127},
    // 548 - ram_r_model_names
    {"MLV", 0},
    {"MBL", 1},
    {"ILV", 2},
    {"IBL", 3},
    {"CU1", 4},
    {"CU2", 5},
    {"LEN", 6},
    {"RAT", 7},
    {"", 127},
    // 557 - generic_param_names
    {"P1", 0},
    {"P2", 1},
    {"P3", 2},
    {"P4", 3},
    {"P5", 4},
    {"P6", 5},
    {"P7", 6},
    {"P8", 7},
    {"AMD", 8},
    {"AMF", 9},
    {"EQF", 10},
    {"EQG", 11},
    {"FLF", 12},
    {"FLW", 13},
    {"FLQ", 14},
    {"SRR", 15},
    {"DST", 16},
    {"VOL", 17},
    {"PAN", 18},
    {"DEL", 19},
    {"REV", 20},
    {"LFS", 21},
    {"LFD", 22},
    {"LFM", 23},
    {"", 127},
    // end
};

model_param_name_t mnm_model_param_names[] = {
    // 0 - mnm_gnd_sin_model_names
    {"TUN", 7},
    {"", 127},
    // 2 - mnm_gnd_nois_model_names
    {"ST", 0},
    {"RED", 1},
    {"TUN", 7},
    {"", 127},
    // 6 - mnm_swave_saw_model_names
    {"UNL", 0},
    {"UNW", 1},
    {"UNX", 2},
    {"SBX", 4},
    {"SB1", 5},
    {"SB2", 6},
    {"TUN", 7},
    {"", 127},
    // 14 - mnm_swave_puls_model_names
    {"UNL", 0},
    {"UNW", 1},
    {"SB1", 2},
    {"SB2", 3},
    {"PW", 4},
    {"PWD", 5},
    {"PWR", 6},
    {"TUN", 7},
    {"", 127},
    // 23 - mnm_swave_ens_model_names
    {"PC2", 0},
    {"PC3", 1},
    {"PC4", 2},
    {"WAV", 3},
    {"PW", 4},
    {"CHL", 5},
    {"CHW", 6},
    {"TUN", 7},
    {"", 127},
    // 32 - mnm_sid_6581_model_names
    {"PW", 0},
    {"PWD", 1},
    {"PWR", 2},
    {"WAV", 3},
    {"MOD", 4},
    {"MSR", 5},
    {"MFQ", 6},
    {"TUN", 7},
    {"", 127},
    // 41 - mnm_dpro_wave_model_names
    {"WAV", 0},
    {"WP", 1},
    {"WPM", 2},
    {"WPR", 3},
    {"SYN", 4},
    {"SFQ", 5},
    {"TUN", 7},
    {"", 127},
    // 49 - mnm_dpro_bbox_model_names
    {"PTC", 0},
    {"STR", 1},
    {"RTG", 4},
    {"RTM", 5},
    {"", 127},
    // 54 - mnm_dpro_ddrw_model_names
    {"WV1", 0},
    {"MIX", 1},
    {"WV2", 2},
    {"TIM", 3},
    {"BR1", 4},
    {"WID", 5},
    {"BR2", 6},
    {"TUN", 7},
    {"", 127},
    // 63 - mnm_dpro_dens_model_names
    {"PC2", 0},
    {"PC3", 1},
    {"PC4", 2},
    {"WAV", 3},
    {"CHL", 5},
    {"CHW", 6},
    {"TUN", 7},
    {"", 127},
    // 71 - mnm_fm_stat_model_names
    {"1FQ", 0},
    {"1FI", 1},
    {"1NV", 2},
    {"1FB", 3},
    {"2FQ", 4},
    {"2VL", 5},
    {"TON", 6},
    {"TUN", 7},
    {"", 127},
    // 80 - mnm_fm_par_model_names
    {"1FQ", 0},
    {"1NV", 1},
    {"2FQ", 2},
    {"2NV", 3},
    {"3FQ", 4},
    {"3NV", 5},
    {"TON", 6},
    {"TUN", 7},
    {"", 127},
    // 89 - mnm_fm_dyn_model_names
    {"1FQ", 0},
    {"1FN", 1},
    {"1VL", 2},
    {"1VN", 3},
    {"2FQ", 4},
    {"2NV", 5},
    {"2FB", 6},
    {"TUN", 7},
    {"", 127},
    // 98 - mnm_vo_vo6_model_names
    {"VC1", 0},
    {"VC2", 1},
    {"VSW", 2},
    {"VOI", 3},
    {"CNS", 4},
    {"CLN", 5},
    {"VCL", 6},
    {"TUN", 7},
    {"", 127},
    // 107 - mnm_fx_thru_model_names
    {"INP", 7},
    {"", 127},
    // 109 - mnm_fx_reverb_model_names
    {"DEC", 0},
    {"DMP", 1},
    {"GAT", 2},
    {"MIX", 3},
    {"HP", 4},
    {"LP", 5},
    {"INP", 7},
    {"", 127},
    // 117 - mnm_fx_chorus_model_names
    {"DEL", 0},
    {"DEP", 1},
    {"SPD", 2},
    {"MIX", 3},
    {"FB", 4},
    {"WID", 5},
    {"LP", 6},
    {"INP", 7},
    {"", 127},
    // 126 - mnm_fx_dynamix_model_names
    {"ATK", 0},
    {"REL", 1},
    {"THR", 2},
    {"MIX", 3},
    {"RAT", 4},
    {"GAI", 5},
    {"RMS", 6},
    {"INP", 7},
    {"", 127},
    // 135 - mnm_fx_ringmod_model_names
    {"WAV", 0},
    {"EXT", 1},
    {"MIX", 3},
    {"INP", 7},
    {"", 127},
    // 140 - mnm_generic_param_names
    {"ATK", 0x08},
    {"HLD", 0x09},
    {"DEC", 0x0a},
    {"REL", 0x0b},
    {"DST", 0x0c},
    {"VOL", 0x0d},
    {"PAN", 0x0e},
    {"PRT", 0x0f},

    {"BAS", 0x10},
    {"WDT", 0x11},
    {"HPQ", 0x12},
    {"LPQ", 0x13},
    {"ATK", 0x14},
    {"DEC", 0x15},
    {"BOF", 0x16},
    {"WOF", 0x17},

    {"EQF", 0x18},
    {"EQG", 0x19},
    {"SRR", 0x1a},
    {"DTI", 0x1b},
    {"DSN", 0x1c},
    {"DFB", 0x1d},
    {"DBS", 0x1e},
    {"DWI", 0x1f},

    {"PAG", 0x20},
    {"DST", 0x21},
    {"TRG", 0x22},
    {"WAV", 0x23},
    {"MLT", 0x24},
    {"SPD", 0x25},
    {"INT", 0x26},
    {"DPT", 0x27},

    {"PAG", 0x28},
    {"DST", 0x29},
    {"TRG", 0x2a},
    {"WAV", 0x2b},
    {"MLT", 0x2c},
    {"SPD", 0x2d},
    {"INT", 0x2e},
    {"DPT", 0x2f},

    {"PAG", 0x30},
    {"DST", 0x31},
    {"TRG", 0x32},
    {"WAV", 0x33},
    {"MLT", 0x34},
    {"SPD", 0x35},
    {"INT", 0x36},
    {"DPT", 0x37},

    {"MUT", 100},
    {"LEV", 101},

    {"", 127},
    // end
};