#!/usr/bin/env python3
import numpy as np
from joblib import Parallel, delayed
import matplotlib.pyplot as plt
from scipy import signal

sample_rate = 23040000

def rotate(input, hz):
    x = np.linspace(0, len(input)/sample_rate, num=len(input))
    return np.multiply(input, np.exp(1j*2.0*np.pi*hz*x))

def norm_corr(rotated_data, refgrid, offset_start, offset_end):
    section = rotated_data[offset_start:offset_end]
    num_corrs = len(section) - len(refgrid) + 1
    corr = []
    for i in range(num_corrs):
        corr.append(np.real(np.corrcoef(section[i:i+len(refgrid)], refgrid)[0,1]))
    return corr

def get_correlation_for_rotation(data, rotation, refgrid, offset_start, offset_end):
    # Rotate the signal
    rotated_data = rotate(data, rotation)
    #corr = np.abs(np.correlate(rotated_data[offset_start:offset_end], refgrid))  # Cross-correlation
    corr = norm_corr(rotated_data, refgrid, offset_start, offset_end)  # Pearson (normalized) correlation

    # Debug: plot
    #plt.plot(corr)
    #plt.show()

    return corr

refgrid = np.fromfile("/tmp/corr_ref_grid", dtype=np.complex64)
#plt.specgram(refgrid)
#plt.show()

data = np.fromfile("/tmp/corr_samples", dtype=np.complex64)
#plt.specgram(data)
#plt.show()

all_corrs = []
rot_range = 2000
num_steps = 100
offset_start = 40000  # Search space begin
offset_end = 75000  # Search space end
best_corr = 0
best_rotation = 0
best_timing = 0
current_step = 0

rotations = np.linspace(-rot_range,rot_range,num=num_steps)
all_corrs = Parallel(n_jobs=8)(delayed(get_correlation_for_rotation)(data, rotation, refgrid, offset_start, offset_end) for rotation in rotations)

for i, corr in enumerate(all_corrs):
    # Store the correlation if it is larger than previous max
    max_corr = np.max(corr)
    if max_corr > best_corr:
        best_corr = max_corr
        best_rotation = rotations[i]
        best_timing = np.argmax(corr) + offset_start

print(f"Best timing is {best_timing} with a correlation of {best_corr} and rotation of {best_rotation}")
print(f"Best timing should be equal to 'Fine timing offset based on full-rate SSS'")
sss_offset = 1536+120 + 3*(1536+108) + 108 # Symbol 0, 1, 2, 3 and CP of SSS
print(f"SSS is at {sss_offset+best_timing} (assuming 1536 FFT size). This value should be equal to 'Full-rate SSS is actually at'")
sync_data = data[best_timing:best_timing+len(refgrid)]

print("")
print("Values in C++ impl: 52733 and 59429")
# Best result I found: "Best timing is 52733 with a correlation of 3110.3357541661817 and rotation of 989.7474368592152"


plt.imshow(np.stack(all_corrs), aspect=len(corr) / num_steps, interpolation="nearest")
plt.show()

plt.specgram(sync_data)
plt.show()
