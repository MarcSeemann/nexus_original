#!/usr/bin/env python3

import h5py
import numpy as np
import matplotlib.pyplot as plt

def analyze_muon_energy_distribution(filename):
    """
    Comprehensive analysis of muon energy distribution from nexus HDF5 output.
    Tests if the Gaisser parametrization E^(-2.7) * (1 + E/115GeV)^(-3.64) is working.
    """
    
    print(f"=== ANALYZING MUON ENERGY DISTRIBUTION ===")
    print(f"File: {filename}")
    
    try:
        with h5py.File(filename, 'r') as f:
            particles = f['MC/particles'][:]
            
            # Filter for primary muons (mu+ and mu-)
            primary_particles = particles[particles['primary'] == 1]
            muon_mask = np.array([b'mu' in p['particle_name'] for p in primary_particles])
            muons = primary_particles[muon_mask]
            
            if len(muons) == 0:
                print("❌ NO PRIMARY MUONS FOUND!")
                return
                
            # Extract energies in GeV
            energies_mev = muons['kin_energy']
            energies_gev = energies_mev / 1000.0
            
            print(f"✅ Found {len(muons)} primary muons")
            print(f"Energy range: {np.min(energies_gev):.3f} - {np.max(energies_gev):.3f} GeV")
            
            # Statistical analysis
            mean_energy = np.mean(energies_gev)
            median_energy = np.median(energies_gev)
            std_energy = np.std(energies_gev)
            
            print(f"\n=== STATISTICAL ANALYSIS ===")
            print(f"Mean energy: {mean_energy:.3f} GeV")
            print(f"Median energy: {median_energy:.3f} GeV")
            print(f"Standard deviation: {std_energy:.3f} GeV")
            
            # Expected values for uniform distribution (1-50 GeV)
            uniform_mean = 25.5
            uniform_median = 25.5
            uniform_std = (50-1) / np.sqrt(12)  # ≈ 14.14
            
            print(f"\nUniform distribution (1-50 GeV) would have:")
            print(f"  Mean: {uniform_mean:.3f} GeV")
            print(f"  Median: {uniform_median:.3f} GeV")
            print(f"  Std: {uniform_std:.3f} GeV")
            
            # Expected values for Gaisser distribution
            # Gaisser spectrum is heavily weighted toward low energies
            print(f"\nGaisser distribution (1-50 GeV) should have:")
            print(f"  Mean: ~2-5 GeV (much lower than uniform)")
            print(f"  Median: ~1-3 GeV (much lower than uniform)")
            print(f"  Most muons < 10 GeV")
            
            # Distribution characteristics
            low_energy_fraction = np.sum(energies_gev < 5.0) / len(energies_gev) * 100
            high_energy_fraction = np.sum(energies_gev > 20.0) / len(energies_gev) * 100
            very_high_energy_fraction = np.sum(energies_gev > 40.0) / len(energies_gev) * 100
            
            print(f"\n=== DISTRIBUTION CHARACTERISTICS ===")
            print(f"% of muons < 5 GeV: {low_energy_fraction:.1f}%")
            print(f"% of muons > 20 GeV: {high_energy_fraction:.1f}%")
            print(f"% of muons > 40 GeV: {very_high_energy_fraction:.1f}%")
            
            print(f"\nFor comparison:")
            print(f"Uniform: 8.2% < 5 GeV, 61.2% > 20 GeV, 20.4% > 40 GeV")
            print(f"Gaisser: ~80-90% < 5 GeV, <10% > 20 GeV, <2% > 40 GeV")
            
            # Determine distribution type
            print(f"\n=== DISTRIBUTION ASSESSMENT ===")
            if mean_energy > 20 and abs(mean_energy - uniform_mean) < 5:
                print("🔴 APPEARS UNIFORM - Gaisser implementation not working!")
            elif mean_energy < 10 and low_energy_fraction > 60:
                print("🟢 APPEARS GAISSER - Implementation working correctly!")
            else:
                print("🟡 MIXED/UNCLEAR - Needs further investigation")
            
            # Create visualization
            fig, axes = plt.subplots(2, 2, figsize=(15, 12))
            
            # 1. Linear histogram
            axes[0,0].hist(energies_gev, bins=50, alpha=0.7, edgecolor='black', color='blue')
            axes[0,0].axvline(mean_energy, color='red', linestyle='--', linewidth=2, label=f'Mean: {mean_energy:.2f} GeV')
            axes[0,0].axvline(uniform_mean, color='orange', linestyle='--', linewidth=2, label=f'Uniform mean: {uniform_mean:.1f} GeV')
            axes[0,0].set_xlabel('Energy (GeV)')
            axes[0,0].set_ylabel('Count')
            axes[0,0].set_title('Muon Energy Distribution - Linear Scale')
            axes[0,0].legend()
            axes[0,0].grid(True, alpha=0.3)
            
            # 2. Log-scale histogram
            axes[0,1].hist(energies_gev, bins=50, alpha=0.7, edgecolor='black', color='red')
            axes[0,1].set_xlabel('Energy (GeV)')
            axes[0,1].set_ylabel('Count')
            axes[0,1].set_yscale('log')
            axes[0,1].set_title('Muon Energy Distribution - Log Y Scale')
            axes[0,1].grid(True, alpha=0.3)
            
            # 3. Theoretical Gaisser vs data
            energy_theory = np.linspace(1, 50, 1000)
            epsilon_star = 115.0  # GeV
            gaisser_theory = np.power(energy_theory, -2.7) * np.power(1.0 + energy_theory/epsilon_star, -3.64)
            # Normalize to approximate the same scale
            gaisser_normalized = gaisser_theory / np.max(gaisser_theory) * len(muons) * 0.8
            
            axes[1,0].plot(energy_theory, gaisser_normalized, 'r-', linewidth=3, label='Gaisser Theory', alpha=0.8)
            axes[1,0].hist(energies_gev, bins=50, alpha=0.6, edgecolor='black', color='blue', label='Simulation Data')
            axes[1,0].set_xlabel('Energy (GeV)')
            axes[1,0].set_ylabel('Count')
            axes[1,0].set_title('Comparison with Gaisser Theory')
            axes[1,0].legend()
            axes[1,0].grid(True, alpha=0.3)
            axes[1,0].set_xlim(0, 30)  # Focus on main region
            
            # 4. Cumulative distribution
            sorted_energies = np.sort(energies_gev)
            cumulative = np.arange(1, len(sorted_energies) + 1) / len(sorted_energies)
            axes[1,1].plot(sorted_energies, cumulative, 'b-', linewidth=3, label='Simulation CDF')
            
            # Theoretical uniform CDF
            uniform_x = np.linspace(1, 50, 100)
            uniform_cdf = (uniform_x - 1) / (50 - 1)
            axes[1,1].plot(uniform_x, uniform_cdf, 'orange', linestyle='--', linewidth=2, label='Uniform CDF')
            
            # Approximate Gaisser CDF (analytical approximation)
            gaisser_cdf = 1 - np.power(uniform_x, -1.7) * np.power(1 + uniform_x/115, -2.64)
            gaisser_cdf = (gaisser_cdf - np.min(gaisser_cdf)) / (np.max(gaisser_cdf) - np.min(gaisser_cdf))
            axes[1,1].plot(uniform_x, gaisser_cdf, 'r--', linewidth=2, label='Approx Gaisser CDF')
            
            axes[1,1].set_xlabel('Energy (GeV)')
            axes[1,1].set_ylabel('Cumulative Probability')
            axes[1,1].set_title('Cumulative Distribution Function')
            axes[1,1].legend()
            axes[1,1].grid(True, alpha=0.3)
            axes[1,1].set_xlim(0, 30)
            
            plt.tight_layout()
            plt.savefig('muon_energy_analysis_comprehensive.png', dpi=150, bbox_inches='tight')
            print(f"\n📊 Saved comprehensive analysis plot as 'muon_energy_analysis_comprehensive.png'")
            
            # Final verdict
            print(f"\n=== FINAL VERDICT ===")
            if mean_energy < 8 and low_energy_fraction > 70:
                print("✅ SUCCESS: Gaisser energy spectrum is working correctly!")
                print("   The distribution shows the expected E^(-2.7) behavior with most muons at low energy.")
            elif mean_energy > 15:
                print("❌ FAILURE: Still using uniform energy distribution!")
                print("   The Gaisser implementation is not being used correctly.")
            else:
                print("⚠️  PARTIAL: Mixed behavior - may need further investigation.")
            
    except FileNotFoundError:
        print(f"❌ File {filename} not found!")
    except Exception as e:
        print(f"❌ Error analyzing file: {e}")

if __name__ == "__main__":
    analyze_muon_energy_distribution("Muon_Xe_gaisser_test.next.h5")
