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
#### Ubuntu
The code was tested to successfully compile on Ubuntu 22.04. Please use the following instructions to install the required dependences. 

```
sudo apt-get update
sudo apt-get install cmake make gcc g++ pkg-config libfftw3-dev libmbedtls-dev libsctp-dev libyaml-cpp-dev libgtest-dev libliquid-dev libconfig++-dev libzmq3-dev libspdlog-dev libfmt-dev
```

5GSniffer was tested with clang version 14 on Ubuntu:
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

#### Arch Linux
Alternatively, on Arch Linux (tested on version 2024-11) the required dependencies can be installed as follows:

```
pacman -S gcc git cmake clang make libuhd spdlog mbedtls boost lksctp-tools libconfig liquid-dsp cppzmq libvolk
```

### Hardware
Uses srsRAN basic SDR libraries, supports USRP B210, X310, bladeRF. For this release it is recommended to use a recorded file.

```
sudo apt-get install libuhd-dev uhd-host
```

### Building
#### Ubuntu
```
git clone --recurse-submodules https://github.com/spritelab/5GSniffer.git
cd 5GSniffer/5gsniffer
mkdir -p build
cd build
export CXX=/usr/bin/clang++-14
export CC=/usr/bin/clang-14
cmake -DCMAKE_C_COMPILER=/usr/bin/clang-14 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-14 ..
make -j 8
```

Add the `-DCMAKE_BUILD_TYPE=Debug` flag to build in developer / debug mode

You can also just:
```
cd 5gsniffer
./compile
```

#### Arch Linux
On Arch Linux, 5GSniffer was tested with `gcc` version 14.2.1 and `clang` version 18.1.8. The build process is similar to Ubuntu:

Using `gcc`:
```
git clone --recurse-submodules https://github.com/spritelab/5GSniffer.git
cd 5GSniffer/5gsniffer
mkdir -p build
cd build
cmake ..
make
```

When using `clang`, replace the CMake command with the following:
```
cmake -DCMAKE_C_COMPILER=/usr/bin/clang -DCMAKE_CXX_COMPILER=/usr/bin/clang++ ..
```

#### Docker
Docker can be used to build and run 5GSniffer on any distribution:

Building:
```
docker build -t 5gsniffer:latest .
```

Running: 
```
docker run --mount=type=bind,source=/path-to-downloaded-samples-folder/,target=/5gsniffer/5gsniffer/test/samples/ -it 5gsniffer:latest 
```

This will spawn an interactive shell inside the container, at which point you can continue as described below.

**Note for developers**: to make changes to the source code using the same build environment as the authors, you can clone the repository on your host machine and create a bind mount to the Docker container as such:

```
docker run --mount=type=bind,source=/path-to-your-projects-folder/5GSniffer,target=/5gsniffer --mount=type=bind,source=/path-to-downloaded-samples-folder/,target=/5gsniffer/5gsniffer/test/samples/ -it 5gsniffer:latest
```

Then, to build the project using the build environment inside the container:

```
# cd /5gsniffer/5gsniffer
# mkdir build_docker
# cd build_docker
# cmake ..
# make
```


## Usage instructions

The easiest way to try 5GSniffer is on a recorded file of I&Q samples. Download the following file to run the code:
https://drive.google.com/drive/folders/16YMVftlxgPgA8O3zwtno4VHCPzVWXFbX?usp=share_link
This recording was taken by connecting 2 smartphones to a srsRAN 5G gNB and generating traffic on both UEs.

To run, place the file under 5GSniffer/5gsniffer/test/samples and run the executable with the config file SpriteLab-Private5G.toml:

```
./src/5g_sniffer ../SpriteLab-Private5G.toml &> output.txt
```

### **Detailed instructions**

All configuration parameters should be specified in the config file used as input to the sniffer. The example config file “SpriteLab-Private5G.toml” included in the code can be used as a reference and template.

Currently, the sniffer supports the following configuration parameters.

#### **General sniffer config parameters:**

The sniffer can operate from recorded files offline, or using a SDR.

**file_path:** specifies the path to a spectrum recording, in case of reading from file.

**sample_rate:** specifies the sampling rate at which the file was recorded, or the sampling rate at which we want to operate the SDR.

**frequency:** specifies the center frequency used for the SDR operation.

**nid_1:** specifies the N_ID_1 parameter from cell ID. This would only look for this N_ID_1 value.

**ssb_numerology:** specifies the numerology used for the SSB block, i.e. numerology 0 for a subcarrier spacing of 15 kHz and 1 for 30 kHz.


#### **PDCCH-specific config parameters:**

5G is flexible and highly configurable; it can operate over multiple Control Resource Sets (CORESET), and multiple PDCCH configurations. As such, it is more complex than LTE and requires additional prior information. The tool allows multiple [pdcch] configs in the configuration file. Each PDCCH can have multiple configurable parameters. The parameters are the following:

**coreset_id:** specifies the identifier of the CORESET.

**subcarrier_offset:** As the SSB and CORESET/PDCCH have different center frequencies, and it would depend on the configuration of the cell, we use this parameter to indicate how many subcarriers need to be shifted to center the given CORESET. More information and an example on this parameter can be found in the next section. 

**num_prbs:** indicates the number of Physical Resource Blocks (PRBs) used for PDCCH. Corresponds to “_frequencyDomainResources_” parameter in “_pdcch-Config_” in RRC.

**numerology:** indicates the numerology (0,1,2), i.e. subcarrier spacing and slot configuration of the cell.

**dci_sizes_list:** in 5G, the DCI sizes are variable. As such, this parameter indicates which DCI sizes we want to try to decode. From our experience, we did not see more than 4 DCI sizes in a cell. More information on DCI sizes can be found in the next section. 

**scrambling_id_start** and **scrambling_id_end:** these values specify the range of scrambling IDs, pdcch-ScramblingID, that we want to sniff over. We have found that this parameter is configured differently per operator/vendor, for instance, some operators might use a fixed value. Corresponds to “_pdcch-DMRS-ScramblingID_” parameter in “_pdcch-Config_” in RRC.

**rnti_start** and **rnti_end:** these values specify the range of RNTIs we want to sniff. From our network operation survey we found that operators only allocate RNTIs in specific subsets of RNTIs.

**coreset_interleaving_pattern:** indicates the interleaving pattern used. Corresponds to “_cce-REG-MappingType_” parameter in “_pdcch-Config_” in RRC.

**coreset_interleaver_size:** corresponds to “_interleaverSize_” parameter in “_pdcch-Config_” in RRC.

**coreset_reg_bundle_size:** corresponds to “_regBundleSize_” parameter in “_pdcch-Config_” in RRC.

**coreset_duration:** indicates how many OFDM symbols are used for CORESET. Corresponds to “_duration_” parameter in “_pdcch-Config_” in RRC.

**coreset_nshift:** indicates the cyclic shift used for the interleaving pattern. Corresponds to “_shiftIndex_” parameter in “_pdcch-Config_” in RRC.

**coreset_ofdm_symbol_start:** indicates the starting OFDM symbol in the slot that corresponds to the CORESET.

**AL_corr_thresholds:** indicates, for each aggregation level, what is the correlation threshold over which we will try to decode a DCI. Lowering this value increases the speed of the decoder, but also increases the probability of missing DCIs.

**num_candidates_per_AL:** indicates how many candidates should the sniffer look for per each aggregation level. Corresponds to “_nrofCandidates_” parameter in “_pdcch-Config_” in RRC.


#### **An example:**

To showcase how to configure the sniffer, we provide a real case scenario. We took a recording of a 5G operator in our area operating over a 10MHz bandwidth in band 71. In the image below, we can see the frequency grid, which we have labeled highlighting the **SSB**, and two CORESETs, **CORESET#0** and **CORESET#1**. We have also highlighted the PDSCH/PDCCH. CORESET#0 contains only SIB1, and CORESET#1 contains all other ue-specific scheduling:

![labeled_spectrum_drawIO_sniffer drawio](https://github.com/spritelab/5GSniffer/assets/45082699/d30b59db-181b-4d0b-923e-9d0042cfca31)

**Figure:** Labeled 5G spectrum depicting basic relevant channels and their locations.

CORESETs can have multiple configurations, e.g. number of frequency resources, OFDM symbol duration, interleaved vs non-interleaved, etc. This can be obtained through RRC Setup/RRC Reconfiguration messages, which can be obtained through tools such as QXDM, QCSuper, NSG...

The following screenshots from NSG contain the configuration parameters for the cell:

Common cell information, BWP sizes and subcarrier spacing:

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/7cc2627f-aa5b-4dca-95f9-483da22e8931" width="340">


**CORESET#0 Configuration:**

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/b65f8c5e-bfc5-4e8b-b849-cdaf74c7d47c" width="340">



**CORESET#1 Configuration:**

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/99de60c8-eea1-4077-a07a-1d3b399726e5" width="340">

Determining subcarrier_offset:

The position of the PDCCH region can be configured through multiple parameters, e.g. AbsoluteFrequencyPointA, LocationAndBandwidth, and can be relative to the SSB position, absoluteFrequencySSB, k_SSB. Please refer to online resources for more information, such as: https://www.sharetechnote.com/html/5G/5G_ResourceBlockIndexing.html

From the log, we can see that the absoluteFrequencySSB value is 125550, which translates to 627.750 MHz, and the absoluteFrequencyPointA is 124464, i.e. 622.320 MHz. 

The absoluteFrequencySSB is the frequency position of the subcarrier 0 of resource block 10 of the SS Block, i.e the center of the SSB as it occupies 20 RBs, 240 subcarriers.

Looking at frequencyDomainResources, we see that the bitmap has 8 bits set to 1, this means that our PDCCH occupies 6*8 = 48 RBs. 

The PDCCH BW is 48 RBs, and the subcarrier spacing is 15KHz. Thus the PDCCH bandwidth is 48 RBs *12 subcarrier/RB * 15KHz subcarrier spacing = 8.64 MHz.

We need to align the lowest subcarrier of the PDCCH BW to pointA, 622.320 MHz. To do so, as our recording is taken at center frequency 627.750 MHz, we need to put the center of the 8.64 MHz BW such that the lowest subcarrier is at BW 622.320 MHz.   622.320 MHz + (8.64/2) MHz = 626.64 MHz. Thus, we have to shift our center frequency by ((627.750 - 626.64) * 1000) KHz / 15 (subcarrier spacing) = 74 subcarriers.

#### **Additional example:**

We setup our private 5G testbed in our lab. The RRC configuration is the following:

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/260ffa72-d368-4272-9242-80ca2499f392" width="340">

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/a358d2d3-2711-46d3-ad31-2dbf2e9a8574" width="340">

In this example, the cell presents a different configuration, where pointA is now not specified in absolute values as before (AbsoluteFrequencyPointA), but as a relative position to the SSB (offsetToPointA). Furthermore, as pointA is relative to the SSB frequency location, we also have to account for k_SSB, ssb-subcarrierOffset, as seen in the Figure above. k_SSB is conveyed through the MIB.

The first step is to determine the frequency of pointA to align the lowest subcarrier of the PDCCH to this value. In this case, the SSB is centered in frequency 627.650 (SSB-ARFCN 125530), and the offsetToPointA is 16 PRBs, which corresponds to 16 * 12 * 0.015 MHz SCS = 2.88 MHz. In this case, our SSB center frequency is 627.65 MHz, and the obtained MIB is the following:

<img src="https://github.com/spritelab/5GSniffer/assets/45082699/7b491bc5-60d0-46e7-9b47-c28ebaef0e2d" width="340">

Where ssb-SubcarrierOffset is 10 subcarriers. Thus, the BWP lowest subcarrier, pointA, is:
pointAFreq = SSB_center_frequency - (SSB_BW/2) - k_SSB - offsetPointA = 
627.650 MHz - (240 SSB sc * 0.015 MHz/sc)/2 - (10 sc * 0.015 MHz/sc) - (16 PRB * 12 sc/PRB * 0.015 MHz/sc) = 632.480 MHz    

Then, to compute subcarrier_offset, as the PDCCH BW is 48 PRBs, we need to put the center of the 8.64 MHz BW such that the lowest subcarrier is at 622.82 MHz.   622.82 MHz + (8.64/2) MHz = 627.14 MHz. Thus, we have to shift our center frequency by ((627.650 - 627.14) * 1000) KHz / 15 (subcarrier spacing) = 34 subcarriers.



The rest of the parameters are as follows:

coreset_id = 1 <br>
#sc offset aligns the BWP to the lowest SC of the BWP <br>
subcarrier_offset = 34 <br>
num_prbs = 48 <br>
numerology = 0 <br>
dci_sizes_list = [39] <br>
scrambling_id_start = 1 <br>
scrambling_id_end = 1 <br>
interleaving_pattern = "non-interleaved" <br>
coreset_duration = 2 <br>
AL_corr_thresholds = [1, 1, 0.4, 1, 1] <br>
num_candidates_per_AL = [0, 0, 4, 0, 0] <br>

Video using the sniffer:

https://youtu.be/x9vkgp9ol44




### Logs

A sample output log of our tool is included, the logs include MIB decoding information and the found DCI bits.

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
