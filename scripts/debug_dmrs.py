#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt

data = np.fromfile("/tmp/load_matlab_slot1_symbol1", dtype=np.complex64)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()

data = np.fromfile("/tmp/correlation_pdcch_dmrs_AL8", dtype=np.float32)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()

data = np.fromfile("/tmp/correlation_pdcch_dmrs_AL4", dtype=np.float32)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()
