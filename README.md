# QKD-SystemC-Simulation

## Overview
  This project implements a simulation of the BB84 Quantum Key Distribution (QKD) protocol using SystemC.
  It models the complete pipeline from raw key generation to final secure key extraction.
  This project was developed as part of a group assignment.

## Features 
  - Random key and basis generation (Alice & Bob)
  - Sifting based on basis matching
  - QBER (Quantum Bit Error Rate) estimation
  - Hamming (15,11) error correction
  - Privacy amplification using random binary matrix

## Architecture
  - Sifting → Filters valid bits using basis comparison
  - Error Estimation → Computes QBER
  - Hamming Correction → Corrects bit errors
  - Privacy Amplification → Compresses key securely
    
## How to Run
  1. Open EDA Playground
  2. Select C++ / SystemC
  3. Upload design.cpp and testbench.cpp
  4. Compile and run

## Sample Output
  - See docs/sample_output.txt

## My Contribution
  - Designed and implemented the complete SystemC-based QKD pipeline
  - Implemented Hamming error correction module
  - Implemented privacy amplification using random matrix
