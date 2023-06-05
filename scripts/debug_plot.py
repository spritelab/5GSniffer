#!/usr/bin/env python3
import numpy as np
import matplotlib.pyplot as plt

for x in range(3):
    data = np.fromfile("/tmp/pss_corr%d" % x, dtype=np.float32)
    plt.plot(data)
plt.show()

data = np.fromfile("/tmp/delayed", dtype=np.complex64)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()

data = np.fromfile("/tmp/downsampled", dtype=np.complex64)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()

data = np.fromfile("/tmp/pss_fine", dtype=np.complex64)
magnitude = np.abs(data)
plt.plot(magnitude)
plt.show()

data = np.fromfile("/tmp/sss_corr_fine", dtype=np.float32)
plt.plot(data)
plt.show()