# 5GSniffer
5GSniffer is a free open-source 5G Physical Downlink Control Channel (PDCCH) blind decoder. The tool decodes the PDCCH of a specific 5G base station (gNB), and reveals the Radio Network Temporary Identifiers (RNTI) that are present in the cell. The code is written in C++ and uses optimization libraries. The project was build from scratch but uses certain srsRAN libraries as support.

5GSniffer enables analysis and XXX.
The capabilities of 5GSniffer have been illustrated in our research publication at 2023 IEEE Symposium on Security and Privacy (SP) [From 5G Sniffing to Harvesting Leakages of Privacy-Preserving Messengers](https://doi.ieeecomputersociety.org/10.1109/SP46215.2023.00110). Please cite as described in the [Acknowledgments](#acknowledgments) section.

This research project has been funded by XXX

## Features
- 5G PDCCH Decoder
- File or SDR (SDR requires additional config.)
- FDD only
- Configurable for speed/accuracy trade-off.


### License

## Installation
```
sudo apt-get update
```
### Pre-requisites

```
sudo apt-get update
```

### Hardware
Uses srsRAN basic SDR libraries, supports USRP B210, X310, bladeRF.

### Building
```
git clone --recurse-submodules git@github.com:spritelab/5GSniffer.git
cd 5GSniffer/5gsniffer
mkdir -p build
cd build
export CXX=/usr/bin/clang++
export CC=/usr/bin/clang
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
make -j 8
```

Add the -DCMAKE_BUILD_TYPE=Debug flag to build in developer / debug mode


## Usage instructions

Example file to run the code:
https://drive.google.com/drive/folders/16YMVftlxgPgA8O3zwtno4VHCPzVWXFbX?usp=share_link
This recording was taken by connecting 2 smartphones to a srsRAN 5G gNB and generating traffic on both UEs.

To run, place the file under /test/samples and run the executable with the config file srsRAN_n71.toml:

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
