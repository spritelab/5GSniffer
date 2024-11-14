#ifndef SRSRAN_EXPORTS_H
#define SRSRAN_EXPORTS_H

#include <srsran/srsran.h>
#include <srsran/phy/phch/pbch_msg_nr.h>

extern "C" {

constexpr static int PBCH_NR_M = 432;
constexpr static int PBCH_NR_E = 864;
constexpr static int PBCH_NR_N = 1 << 9;
constexpr static int PBCH_NR_A = SRSRAN_PBCH_MSG_NR_SZ + 8;
constexpr static int PBCH_NR_CRC_LEN = 24;
constexpr static int PBCH_NR_K = PBCH_NR_A + PBCH_NR_CRC_LEN;

void pbch_nr_scramble(const srsran_pbch_nr_cfg_t* cfg, const uint8_t a[PBCH_NR_A], uint8_t a_prime[PBCH_NR_A]);
void pbch_nr_scramble_rx(const srsran_pbch_nr_cfg_t* cfg,
                         uint32_t                    ssb_idx,
                         const int8_t                b_hat[PBCH_NR_E],
                         int8_t                      b[PBCH_NR_E]);
void pbch_nr_scramble_tx(const srsran_pbch_nr_cfg_t* cfg,
                         uint32_t                    ssb_idx,
                         const uint8_t               b[PBCH_NR_E],
                         uint8_t                     b_hat[PBCH_NR_E]);

int pbch_nr_polar_rm_rx(srsran_pbch_nr_t* q, const int8_t llr[PBCH_NR_E], int8_t d[PBCH_NR_N]);
int pbch_nr_polar_decode(srsran_pbch_nr_t* q, const int8_t d[PBCH_NR_N], uint8_t c[PBCH_NR_K]);

void pbch_nr_pbch_msg_unpack(const srsran_pbch_nr_cfg_t* cfg, const uint8_t a[PBCH_NR_A], srsran_pbch_msg_nr_t* msg);

/* PDCCH */
int srsran_pdcch_nr_init_rx(srsran_pdcch_nr_t* q, const srsran_pdcch_nr_args_t* args);

int srsran_polar_code_get(srsran_polar_code_t* c, const uint16_t K, const uint16_t E, const uint8_t nMax);

uint32_t pdcch_nr_c_init(const srsran_pdcch_nr_t* q, const srsran_dci_msg_nr_t* dci_msg);

void srsran_polar_chanalloc_rx(const uint8_t*  output_decoder,
                               uint8_t*        message,
                               const uint16_t  K,
                               const uint8_t   nPC,
                               const uint16_t* K_set,
                               const uint16_t* PC_set);

void srsran_polar_interleaver_run(const void* in, void* out, uint32_t S, uint32_t K, bool dir);


}

#endif



