# 5GSniffer
5GSniffer is a free open-source 5G Physical Downlink Control Channel (PDCCH) blind decoder. The tool decodes the PDCCH of a specific 5G base station (gNB), which contains the Downlink Control Information (DCI). This reveals the Radio Network Temporary Identifiers (RNTI) that are present in the cell, as well as other information enabling traffic analysis. 5GSniffer is optimized to overcome several of the challenges introduced by the 5G design. For instance, decoding requires descrambling with information (RNTI and pdcch-DMRS-ScramblingID) provided to the UE in encrypted message. The code is written in C++ and uses optimization libraries. The sniffer was developed from scratch but uses srsRAN libraries as support (e.g., polar decoding).

The capabilities of 5GSniffer have been illustrated in our research publication at 2023 IEEE Symposium on Security and Privacy (SP) [From 5G Sniffing to Harvesting Leakages of Privacy-Preserving Messengers](https://doi.ieeecomputersociety.org/10.1109/SP46215.2023.00110). Please cite as described in the [Acknowledgments](#acknowledgments) section.

This research was conducted as part of a research project, 5G ROSETA, funded by the Office of Naval Research (ONR).

## Features
- 5G PDCCH Decoder
- File or SDR (SDR requires additional config.)
- FDD only
- Configurable for speed/accuracy trade-off.


### License

## Installation

### Pre-requisites
The code was tested to successfully compile on Ubuntu 22.04. Please use the following instructions to install the required dependences. 

```
sudo apt-get update
sudo apt-get install cmake make gcc g++ pkg-config libfftw3-dev libmbedtls-dev libsctp-dev libyaml-cpp-dev libgtest-dev libliquid-dev libconfig++-dev libzmq3-dev libspdlog-dev libfmt-dev
```

5GSniffer was tested with clang version 14:
```
sudo apt install clang
```

<!--
SPDLOG?
```
sudo apt-get install libspdlog-dev
```

LIBFMT?
```
sudo add-apt-repository universe
sudo apt update
sudo apt install libfmt-dev
``` -->

### Hardware
Uses srsRAN basic SDR libraries, supports USRP B210, X310, bladeRF. For this release it is recommended to use a recorded file.

```
sudo apt-get install libuhd-dev uhd-host
```

### Building
```
git clone --recurse-submodules https://github.com/spritelab/5GSniffer.git
cd 5GSniffer/5gsniffer
mkdir -p build
cd build
export CXX=/usr/bin/clang++14
export CC=/usr/bin/clang-14
cmake -DCMAKE_C_COMPILER=/usr/bin/clang-14 -DCMAKE_CXX_COMPILER=/usr/bin/clang++14 ..
make -j 8
```

Add the -DCMAKE_BUILD_TYPE=Debug flag to build in developer / debug mode

You can also just:
```
cd 5gsniffer
./compile
```

## Usage instructions

The easiest way to try 5GSniffer is on a recorded file of I&Q samples. Download the following file to run the code:
https://drive.google.com/drive/folders/16YMVftlxgPgA8O3zwtno4VHCPzVWXFbX?usp=share_link
This recording was taken by connecting 2 smartphones to a srsRAN 5G gNB and generating traffic on both UEs.

To run, place the file under 5GSniffer/5gsniffer/test/samples and run the executable with the config file srsRAN_n71.toml:

```
./src/5g_sniffer ../srsRAN_n71.toml &> output.txt
```

### Executables

### Config files

### Logs

## Acknowledgments
If you enjoyed the tool, please acknowledge us in your publications by citing:
```
@INPROCEEDINGS {5gsniffing2023,
author = {N. Ludant and P. Robyns and G. Noubir},
booktitle = {2023 IEEE Symposium on Security and Privacy (SP)},
title = {{From 5G Sniffing to Harvesting Leakages of Privacy-Preserving Messengers}},
year = {2023},
month = {May}
}
```
