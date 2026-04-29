#!/usr/bin/env python3

import h5py
import numpy as np
import matplotlib.pyplot as plt

# Read the HDF5 file
filename = "Muon_Xe_gaisser_test.next.h5"
print(f"Analyzing energy distribution in {filename}")

try:
    with h5py.File(filename, 'r') as f:
        print("File structure:")
        def print_structure(name, obj):
            print(name)
        f.visititems(print_structure)
        
        print("\nLooking for energy data...")
        
        # Common locations for particle data in NEXT files
        if 'MC' in f:
            mc_group = f['MC']
            print(f"MC group keys: {list(mc_group.keys())}")
            
            if 'particles' in mc_group:
                particles = mc_group['particles']
                print(f"Particles dataset shape: {particles.shape}")
                print(f"Particles dtype: {particles.dtype}")
                
                # Read particle data
                particle_data = particles[:]
                print(f"Number of particles: {len(particle_data)}")
                
                # Look for energy field
                if 'kin_energy' in particle_data.dtype.names:
                    energies = particle_data['kin_energy']
                    print(f"Energies (MeV): {energies}")
                    print(f"Energy range: {np.min(energies):.2f} - {np.max(energies):.2f} MeV")
                    print(f"Energy range (GeV): {np.min(energies)/1000:.2f} - {np.max(energies)/1000:.2f} GeV")
                    
                    # Convert to GeV
                    energies_gev = energies / 1000.0
                    
                    # Check if distribution looks uniform vs Gaisser
                    print(f"\nEnergy distribution analysis:")
                    print(f"Mean energy: {np.mean(energies_gev):.2f} GeV")
                    print(f"Std energy: {np.std(energies_gev):.2f} GeV")
                    
                    # For uniform distribution between 1-50 GeV: mean=25.5, std=14.14
                    # For Gaisser distribution: mean should be much lower (weighted towards low energy)
                    uniform_mean = (1 + 50) / 2  # 25.5 GeV
                    uniform_std = (50 - 1) / np.sqrt(12)  # ~14.14 GeV
                    
                    print(f"Expected uniform: mean={uniform_mean:.2f} GeV, std={uniform_std:.2f} GeV")
                    
                    if abs(np.mean(energies_gev) - uniform_mean) < 2.0 and abs(np.std(energies_gev) - uniform_std) < 2.0:
                        print("⚠️  DISTRIBUTION APPEARS UNIFORM!")
                    else:
                        print("✅ Distribution appears non-uniform (potentially Gaisser)")
                        
                    # Plot histogram
                    plt.figure(figsize=(10, 6))
                    plt.hist(energies_gev, bins=20, alpha=0.7, edgecolor='black')
                    plt.xlabel('Energy (GeV)')
                    plt.ylabel('Count')
                    plt.title('Muon Energy Distribution')
                    plt.grid(True, alpha=0.3)
                    plt.savefig('energy_distribution.png', dpi=150, bbox_inches='tight')
                    print("Saved histogram as energy_distribution.png")
                    
                else:
                    print("Field names in particle data:")
                    for name in particle_data.dtype.names:
                        print(f"  {name}")
                        
        else:
            print("No MC group found")
            print(f"Root level keys: {list(f.keys())}")
            
except FileNotFoundError:
    print(f"File {filename} not found")
except Exception as e:
    print(f"Error reading file: {e}")
