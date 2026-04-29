#!/usr/bin/env python3

import h5py
import numpy as np
import matplotlib.pyplot as plt

# Read the HDF5 file
filename = "Muon_Xe_gaisser_test.next.h5"

print(f"Analyzing energy distribution in {filename}")

with h5py.File(filename, 'r') as f:
    particles = f['MC/particles'][:]
    
    # Filter for primary muons only
    primary_particles = particles[particles['primary'] == 1]
    
    muon_particles = []
    for p in primary_particles:
        particle_name = p['particle_name'].decode('utf-8')
        if 'mu' in particle_name.lower():
            muon_particles.append(p)
    
    if muon_particles:
        muon_energies_mev = np.array([p['kin_energy'] for p in muon_particles])
        muon_energies_gev = muon_energies_mev / 1000.0
        
        print(f"Found {len(muon_energies_gev)} primary muons")
        print(f"Energy range: {np.min(muon_energies_gev):.2f} - {np.max(muon_energies_gev):.2f} GeV")
        print(f"Mean energy: {np.mean(muon_energies_gev):.2f} GeV")
        print(f"Median energy: {np.median(muon_energies_gev):.2f} GeV")
        
        # Statistical analysis
        print(f"\n=== STATISTICAL ANALYSIS ===")
        print(f"Mean: {np.mean(muon_energies_gev):.2f} GeV")
        print(f"Median: {np.median(muon_energies_gev):.2f} GeV")
        print(f"90th percentile: {np.percentile(muon_energies_gev, 90):.2f} GeV")
        print(f"% of muons < 5 GeV: {100 * np.sum(muon_energies_gev < 5) / len(muon_energies_gev):.1f}%")
        print(f"% of muons > 20 GeV: {100 * np.sum(muon_energies_gev > 20) / len(muon_energies_gev):.1f}%")
        print(f"% of muons > 30 GeV: {100 * np.sum(muon_energies_gev > 30) / len(muon_energies_gev):.1f}%")
        
        print(f"\nFor comparison - uniform distribution (1-50 GeV) would have:")
        print(f"Mean: 25.5 GeV")
        print(f"Median: 25.5 GeV") 
        print(f"90th percentile: 45.6 GeV")
        print(f"% of muons < 5 GeV: 8.2%")
        print(f"% of muons > 20 GeV: 61.2%")
        print(f"% of muons > 30 GeV: 40.8%")
        
        # Create the histogram that matches your code
        plt.figure(figsize=(12, 8))
        
        # Subplot 1: Your histogram (energy in MeV, linear scale)
        plt.subplot(2, 2, 1)
        plt.hist(muon_energies_mev, bins=100, color='blue', alpha=0.7, histtype='stepfilled')
        plt.title('Muon Energy Distribution (Your Code)')
        plt.xlabel('Energy (MeV)')
        plt.ylabel('Number of Muons')
        plt.grid(True)
        plt.ylim(0, max(6000, len(muon_energies_gev)//20))
        
        # Subplot 2: Energy in GeV, linear scale
        plt.subplot(2, 2, 2)
        plt.hist(muon_energies_gev, bins=50, color='red', alpha=0.7, histtype='stepfilled')
        plt.title('Muon Energy Distribution (GeV, Linear)')
        plt.xlabel('Energy (GeV)')
        plt.ylabel('Number of Muons')
        plt.grid(True)
        
        # Subplot 3: Log scale (reveals Gaisser better)
        plt.subplot(2, 2, 3)
        plt.hist(muon_energies_gev, bins=50, color='green', alpha=0.7, histtype='stepfilled')
        plt.title('Muon Energy Distribution (GeV, Log Y)')
        plt.xlabel('Energy (GeV)')
        plt.ylabel('Number of Muons')
        plt.yscale('log')
        plt.grid(True)
        
        # Subplot 4: Theoretical comparison
        plt.subplot(2, 2, 4)
        # Plot histogram
        counts, bins, _ = plt.hist(muon_energies_gev, bins=50, alpha=0.7, color='blue', label='Simulation')
        
        # Overlay theoretical Gaisser curve
        energy_theory = np.linspace(1, 50, 1000)
        gaisser_theory = np.power(energy_theory, -2.7) * np.power(1.0 + energy_theory/115.0, -3.64)
        # Normalize to match histogram
        bin_width = (bins[1] - bins[0])
        normalization = len(muon_energies_gev) * bin_width / np.trapz(gaisser_theory, energy_theory)
        gaisser_theory_norm = gaisser_theory * normalization
        
        plt.plot(energy_theory, gaisser_theory_norm, 'r-', linewidth=2, label='Gaisser Theory')
        plt.xlabel('Energy (GeV)')
        plt.ylabel('Number of Muons')
        plt.title('Comparison with Gaisser Theory')
        plt.legend()
        plt.grid(True)
        
        plt.tight_layout()
        plt.savefig('gaisser_analysis_10k.png', dpi=150, bbox_inches='tight')
        print(f"\n✅ Saved analysis plot as 'gaisser_analysis_10k.png'")
        
        # Determine if distribution is uniform or Gaisser-like
        uniform_mean = 25.5
        if np.mean(muon_energies_gev) < 10.0:
            print(f"\n✅ CONCLUSION: Distribution is clearly Gaisser-like!")
            print(f"   - Mean energy ({np.mean(muon_energies_gev):.2f} GeV) is much lower than uniform (25.5 GeV)")
            print(f"   - Most muons are at low energies as expected from E^(-2.7) spectrum")
        else:
            print(f"\n❌ CONCLUSION: Distribution appears more uniform than expected")
            print(f"   - Mean energy ({np.mean(muon_energies_gev):.2f} GeV) is closer to uniform (25.5 GeV)")
            
    else:
        print("No muons found in primary particles!")
        print("Available particle types:")
        for p in primary_particles[:10]:
            print(f"  {p['particle_name']}")
