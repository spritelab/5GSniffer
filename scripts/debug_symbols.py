#!/usr/bin/env python3
import os
import numpy as np
import matplotlib.pyplot as plt
import argparse

arg_parser = argparse.ArgumentParser(description="")
arg_parser.add_argument("num_subcarriers", type=int, default=240, help="Number of subcarriers per symbol.")
args = arg_parser.parse_args()

if os.path.exists("/tmp/symbol_t"):
  data = np.fromfile("/tmp/symbol_t", dtype=np.complex64)
  amplitude = np.abs(data)
  x1 = (18, 18)
  y1 = (min(amplitude), max(amplitude))
  x2 = (256, 256)
  y2 = (min(amplitude), max(amplitude))
  plt.plot(x1, y1, x2, y2)
  plt.plot(amplitude)
  plt.show()

  matlab = np.angle(np.fft.fftshift(np.fft.fft(data[18:18+256])))
  ours = np.angle(np.fft.fftshift(np.fft.fft(data[9:9+256])))
  plt.plot(matlab, label="matlab")
  plt.plot(ours, label="ours")
  plt.legend()
  plt.show()

# Mimic Matlab's imagesc
data = np.fromfile("/tmp/symbols_" + str(args.num_subcarriers), dtype=np.complex64)
data = np.flip(np.reshape(data, (-1, args.num_subcarriers)).T, axis=0)
print(data.shape)
data = np.abs(data)
num_subcarriers = data.shape[0]
num_symbols = data.shape[1]
fig, ax = plt.subplots()
ax.imshow(data, extent=[0, num_symbols, 0, num_subcarriers], aspect=num_symbols / num_subcarriers, interpolation="nearest", cmap="plasma") 
plt.show()


data = np.fromfile("/tmp/symbols_eq", dtype=np.complex64)
data = np.flip(np.reshape(data, (-1, args.num_subcarriers)).T, axis=0)
print(data.shape)
data = np.abs(data)
num_subcarriers = data.shape[0]
num_symbols = data.shape[1]
fig, ax = plt.subplots()
ax.imshow(data, extent=[0, num_symbols, 0, num_subcarriers], aspect=num_symbols / num_subcarriers, interpolation="nearest", cmap="plasma") 
plt.show()