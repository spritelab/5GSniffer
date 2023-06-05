#!/usr/bin/env python3
import os
import numpy as np
import matplotlib.pyplot as plt

dmrs_ref = np.fromfile("/tmp/dmrs_ref", dtype=np.complex64)
plt.plot(np.real(dmrs_ref))
plt.plot(np.imag(dmrs_ref))
plt.show()

plot_abs = False
fig, (ax1, ax2) = plt.subplots(2)

channel = np.fromfile("/tmp/channel", dtype=np.complex64)
if plot_abs:
  ax1.plot(np.abs(channel))
else:
  ax1.plot(np.real(channel))
  ax1.plot(np.imag(channel))

k = 4
offset = 2
channel_ierp = np.fromfile("/tmp/channel_ierp", dtype=np.complex64)
noninterp_indexes = [i for i in range(len(channel_ierp)) if i % k == 0]
noninterp_indexes = [i + offset for i in noninterp_indexes]
print(noninterp_indexes)

if plot_abs:
  ax2.plot(noninterp_indexes, [np.abs(channel_ierp)[x] for x in noninterp_indexes], marker="x", color="blue", linestyle="None")
  ax2.plot(np.abs(channel_ierp))
else:
  ax2.plot(noninterp_indexes, [np.real(channel_ierp)[x] for x in noninterp_indexes], marker="x", color="blue", linestyle="None")
  ax2.plot(noninterp_indexes, [np.imag(channel_ierp)[x] for x in noninterp_indexes], marker="x", color="orange", linestyle="None")
  ax2.plot(np.real(channel_ierp))
  ax2.plot(np.imag(channel_ierp))

plt.show()

# ----------------------------

symbol_chest_before = np.fromfile("/tmp/symbol_chest_before", dtype=np.complex64)
symbol_chest_after = np.fromfile("/tmp/symbol_chest_after", dtype=np.complex64)
symbol_chest_before = symbol_chest_before / np.max(np.abs(symbol_chest_before))
symbol_chest_after = symbol_chest_after / np.max(np.abs(symbol_chest_after))
symbol_chest_before = np.flip(symbol_chest_before)
symbol_chest_after = np.flip(symbol_chest_after)
data = np.stack((symbol_chest_before, symbol_chest_after), axis=1)
print(data.shape)

num_subcarriers = data.shape[0]
num_symbols = data.shape[1]

fig, ax = plt.subplots()
ax.imshow(np.abs(data), extent=[0, num_symbols, 0, num_subcarriers], aspect=num_symbols / num_subcarriers, interpolation="nearest", cmap="plasma") 
plt.show()

plt.plot(np.real(symbol_chest_before), np.imag(symbol_chest_before), marker="o", linestyle="none", fillstyle="none")
plt.show()

plt.plot(np.real(symbol_chest_after), np.imag(symbol_chest_after), marker="o", linestyle="none", fillstyle="none")
plt.show()