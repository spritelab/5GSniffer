#!/usr/bin/env python3
import copy
import os
import random
import pickle
import numpy as np
import matplotlib.pyplot as plt

gold_sequence_length = 31
Nc = 1600

def to_str(prs):
  return ''.join(["%s" % x for x in prs])

def gen_cinit_pdcch(scrambling_id,symbols_per_slot=14, slot_number=4, symbol_number=2):
  return (2**17 * (symbols_per_slot * slot_number + symbol_number + 1) * (2*scrambling_id + 1) + 2*scrambling_id) % 2**31

def pseudo_random_sequence(seq_length, c_init):
  c_init = c_init & 0xffffffff
  size_x = seq_length + gold_sequence_length + Nc

  x1 = [0] * size_x
  x2 = [0] * size_x
  c = [0] * seq_length
  
  x1[0] = 1

  for n in range(0, gold_sequence_length):
    x2[n] = (c_init >> n) & 0x1

  for n in range(0, Nc + seq_length):
    x1[n+31] = (x1[n+3] + x1[n]) % 2
    x2[n+31] = (x2[n+3] + x2[n+2] + x2[n+1] + x2[n]) % 2

  for n in range(0, seq_length):
    c[n] = (x1[n+Nc] + x2[n+Nc]) % 2
  
  return c

class X1:
  def __init__(self, keep=256):
    self.state = [0] * 31
    self.state[0] = 1
    self.index = 0
    self.keep = keep
    self.memory = copy.deepcopy(self.state)

  def advance(self):
    self.state[(31+self.index) % 31] = (self.state[(3+self.index) % 31] + self.state[(0+self.index) % 31]) % 2
    self.index = (self.index + 1) % 31
    new_val = self.state[(31+self.index) % 31]

    # Keep in memory last x bits
    self.memory.append(new_val)
    if len(self.memory) > self.keep:
      self.memory = self.memory[1:]

    return new_val

class X2:
  def __init__(self, c_init, keep=256):
    self.state = [0] * gold_sequence_length
    for n in range(0, gold_sequence_length):
      self.state[n] = (c_init >> n) & 0x1
    self.index = 0
    self.keep = keep
    self.memory = copy.deepcopy(self.state)

  def advance(self):
    self.state[(31+self.index) % 31] = (self.state[(3+self.index) % 31] + self.state[(2+self.index) % 31] + self.state[(1+self.index) % 31] + self.state[(0+self.index) % 31]) % 2
    self.index = (self.index + 1) % 31
    new_val = self.state[(31+self.index) % 31]

    # Keep in memory last x bits
    self.memory.append(new_val)
    if len(self.memory) > self.keep:
      self.memory = self.memory[1:]

    return new_val

class PRNG:
  def __init__(self, keep=256):
    self.memory = None
    self.keep = keep

  def generate_memoryless(self, seq_length, c_init, memory_search=None):
    # Clear memory since we are starting anew
    self.memory = []
    print("Doing %d iterations" % seq_length)

    c_init = c_init & 0xffffffff
    size_x = seq_length + gold_sequence_length + Nc

    x1 = X1(keep=256)
    x2 = X2(c_init, keep=256)

    for n in range(0, Nc-1):
      x1.advance()
      x2.advance()

    for n in range(0, seq_length):
      x1_val = x1.advance()
      x2_val = x2.advance()
      c = (x1_val + x2_val) % 2
      self.memory.append(c)
      if memory_search is not None:
        if self.memory[-len(memory_search):] == memory_search:
          print("Found repetition of length %d at index n=%d" % (len(memory_search), n+1-len(memory_search)))
      if len(self.memory) > self.keep:
        self.memory = self.memory[1:]

    return self.memory

# Reversing PRNG
# ------------------------------------------------------------------------------
# Auto generate predicates?
# [0, 0, 0, 0, 0, 0, 1, 0]
# c[7] = 0
# (x1[1607] + x2[1607]) % 2 = 0
# => (x1[1607] = 1 and x2[1607] = 1) or (x1[1607] = 0 and x2[1607] = 0)
# Assume 1
## => 1 = (x1[1579] + x1[1576]) % 2
## => (x1[1579] = 0 and x1[1576] = 1) or (x1[1579] = 1 and x1[1576] = 0)
## => 1 = (x2[1579] + x2[1578] + x2[1577] + x2[1576]) % 2
## => ...
# Assume 0
## => 0 = (x1[1579] + x1[1576]) % 2
## => (x1[1579] = 0 and x1[1576] = 0) or (x1[1579] = 1 and x1[1576] = 1)
## => 0 = (x2[1579] + x2[1578] + x2[1577] + x2[1576]) % 2
## => ...

# Finding repetitions naive way
# ------------------------------------------------------------------------------
#part = pseudo_random_sequence(256, 0)
#full = pseudo_random_sequence(500000, 0)
#result = np.correlate(full, part, "valid")
#plt.plot(result)
#plt.show()

# Finding repetitions
# ------------------------------------------------------------------------------
#print(pseudo_random_sequence(256, 0))
#p = PRNG()
#p.generate_memoryless(2**32, 0, memory_search=[0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 0])
# Result of finding repetitions experiment: PRNG repeats every 2147483647 (2^31 - 1) iterations. 

# Finding seed for first 1024*8 bits
# ------------------------------------------------------------------------------

seed_map = dict()
if not os.path.exists("seed_cache.p"):
  bits_to_generate = 1024*8
  for i in range(0, 2**16):
    print(i)
    c_init_pdcch = gen_cinit_pdcch(i)
    seed_map[i] = pseudo_random_sequence(bits_to_generate, c_init_pdcch)

  with open("seed_cache.p", "wb") as f:
    pickle.dump(seed_map, f)
else:
  print("LOADING SEEDS FROM CACHE!")
  with open("seed_cache.p", "rb") as f:
    seed_map = pickle.load(f)
  print(seed_map[0])

  with open("experiment_results.txt", "w+") as f:
    for num_bits in range(1, 65):
      random_seed_truth = random.randint(0, 2**16-1)
      random_sequence_truth = seed_map[random_seed_truth][-num_bits:]
      num_matches = 0

      for seed_guess in range(0, 2**16):
        sequence = seed_map[seed_guess]
        if to_str(random_sequence_truth) in to_str(sequence):
          #print("Seed could be %d (%s in %s)" % (seed_guess, to_str(random_sequence_truth), to_str(sequence)))
          print("Seed could be %d" % seed_guess)
          num_matches+=1

      print("Found %d matches in total for %d bits" % (num_matches, num_bits))
      experiment_result = "%d %d\n" % (num_bits, num_matches)
      f.write(experiment_result)
      f.flush()
