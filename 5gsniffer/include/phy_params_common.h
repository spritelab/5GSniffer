#ifndef PHY_PARAMS_COMMON_H
#define PHY_PARAMS_COMMON_H

#include <cstdint>

/* PHY related params */
static constexpr uint8_t max_numerology = 5;
static constexpr uint16_t subcarrier_spacing_15khz = 15'000;

/* SSB Related params */
static constexpr uint8_t ssb_rb = 20;
static constexpr uint16_t ssb_sc = ssb_rb * 12;
static constexpr uint16_t ssb_nfft = 256; // minimum FFT size to cover SS burst
static constexpr uint8_t max_i_ssb = 7;
static constexpr uint8_t max_n_hf = 1;
static constexpr uint8_t PRB_RE = 12;
static constexpr float pss_sss_times_avg_threshold = 1.0; ///< Number of times the PSS or SSS correlation must be higher than the average before it is considered as valid.
static constexpr float pss_average_corr_threshold = 0; // Additional check for PSS validity to reduce false positives
static constexpr uint8_t sss_length = 127;
/*Cell ID and synch signals params*/
static constexpr uint16_t nid_1_max = 335; 
static constexpr uint8_t nid_2_max = 2; 
static constexpr uint16_t nid_max = nid_1_max*3 + nid_2_max; // Max Cell ID (3*nid_1_max + nid_2_max)

/*PN-Sequence/DMRS related constant parameters*/
static constexpr uint8_t gold_sequence_length = 31;
static constexpr uint16_t Nc = 1600;
static constexpr uint16_t pbch_dmrs_length = 144*2;

/*CORESET related parameters*/
static constexpr uint8_t REG_RE = PRB_RE;
static constexpr uint8_t CCE_REG = 6;
static constexpr uint8_t DMRS_RE_PRB = 3;
static constexpr uint8_t DMRS_SC_CCE = 18;

static constexpr uint8_t NUM_ALs = 5;

static constexpr uint8_t AL_1 = 1;
static constexpr uint8_t AL_2 = 2;
static constexpr uint8_t AL_4 = 4;
static constexpr uint8_t AL_8 = 8;
static constexpr uint8_t AL_16 = 16;

/*RNTIs*/
static constexpr uint16_t SI_RNTI = 65535;

/* Channel estimation related parameters */
static constexpr int eq_order = 9;

#endif // PHY_PARAMS_COMMON_H

