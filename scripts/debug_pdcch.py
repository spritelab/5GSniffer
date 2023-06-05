#!/usr/bin/env python3
import os
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.patches import Rectangle
import argparse

class DCI:
  def __init__(self, sample_index, scrambling_id, aggregation_level, candidate_index, correlation):
    self.sample_index = int(sample_index)
    self.scrambling_id = int(scrambling_id)
    self.aggregation_level = int(aggregation_level)
    self.candidate_index = int(candidate_index)
    self.correlation = float(correlation)

arg_parser = argparse.ArgumentParser(description="")
arg_parser.add_argument("flow_index", type=int, default=0, help="Flow index to show.")

args = arg_parser.parse_args()

sample_rate = 23040000
if args.flow_index == 0:
  subcarriers_in_coreset = 52*12
  subcarrier_offset = 48
else:
  subcarriers_in_coreset = 48*12
  subcarrier_offset = 0

#subcarriers_in_coreset = 48*12
subcarrier_spacing = 15000
fft_size = sample_rate // subcarrier_spacing
symbol_duration = 0.001/14.0
#subcarrier_offset = 73
#args = arg_parser.parse_args()

pdcch_data = []
if os.path.exists("/tmp/pdcch_" + str(args.flow_index) + ".csv"):
  with open("/tmp/pdcch_" + str(args.flow_index) + ".csv", "r") as f:
    for line in f:
      pdcch_data.append(DCI(*line.strip().split(",")))
else:
  print("Meta file doesn't exist")
  exit()

print("Got {} potential DCIs".format(len(pdcch_data)))
data = np.fromfile("/tmp/flow_" + str(args.flow_index) + ".cfile", dtype=np.complex64)

# Truncate according to multiple of num_subcarriers
fig, ax = plt.subplots()
Pxx, freqs, bins, im = ax.specgram(data, NFFT=fft_size, Fs=sample_rate, noverlap=0)

for dci in pdcch_data:
  symbol_bandwidth = subcarriers_in_coreset*subcarrier_spacing
  symbol_offset = subcarrier_offset*subcarrier_spacing
  indicator = Rectangle((dci.sample_index / sample_rate, -((symbol_bandwidth // 2) + symbol_offset)), symbol_duration, symbol_bandwidth, alpha=0.20, color="red")
  ax.add_artist(indicator)
plt.show()
