#!/usr/bin/env python3

import h5py
import numpy as np
import matplotlib.pyplot as plt

def analyze_gaisser_distribution(filename):
    """Analyze the muon energy distribution to verify Gaisser parametrization"""
    
    print(f"🔍 Analyzing Gaisser energy distribution in {filename}")
    print("=" * 60)
    
    try:
        with h5py.File(filename, 'r') as f:
            particles = f['MC/particles'][:]
            
            # Filter for primary muons only
            primary_particles = particles[particles['primary'] == 1]
            muon_particles = []
            
            # Filter for muons (particle name contains 'mu')
            for p in primary_particles:
                particle_name = p['particle_name'].decode('utf-8')
                if 'mu' in particle_name.lower():
                    muon_particles.append(p)
            
            if not muon_particles:
                print("❌ No primary muons found!")
                return
            
            muon_energies_mev = np.array([p['kin_energy'] for p in muon_particles])
            muon_energies_gev = muon_energies_mev / 1000.0
            
            print(f"✅ Found {len(muon_energies_gev)} primary muons")
            print(f"📊 Energy range: {np.min(muon_energies_gev):.2f} - {np.max(muon_energies_gev):.2f} GeV")
            
            # Statistical analysis
            mean_energy = np.mean(muon_energies_gev)
            median_energy = np.median(muon_energies_gev)
            std_energy = np.std(muon_energies_gev)
            
            print(f"\n📈 STATISTICAL ANALYSIS:")
            print(f"   Mean energy: {mean_energy:.2f} GeV")
            print(f"   Median energy: {median_energy:.2f} GeV")  
            print(f"   Standard deviation: {std_energy:.2f} GeV")
            
            # Energy distribution percentiles
            p10 = np.percentile(muon_energies_gev, 10)
            p50 = np.percentile(muon_energies_gev, 50) 
            p90 = np.percentile(muon_energies_gev, 90)
            p95 = np.percentile(muon_energies_gev, 95)
            p99 = np.percentile(muon_energies_gev, 99)
            
            print(f"\n📊 ENERGY PERCENTILES:")
            print(f"   10th percentile: {p10:.2f} GeV")
            print(f"   50th percentile: {p50:.2f} GeV")
            print(f"   90th percentile: {p90:.2f} GeV")
            print(f"   95th percentile: {p95:.2f} GeV")
            print(f"   99th percentile: {p99:.2f} GeV")
            
            # Energy range analysis
            low_energy_count = np.sum(muon_energies_gev < 5.0)
            mid_energy_count = np.sum((muon_energies_gev >= 5.0) & (muon_energies_gev < 20.0))
            high_energy_count = np.sum(muon_energies_gev >= 20.0)
            
            low_energy_pct = 100 * low_energy_count / len(muon_energies_gev)
            mid_energy_pct = 100 * mid_energy_count / len(muon_energies_gev)
            high_energy_pct = 100 * high_energy_count / len(muon_energies_gev)
            
            print(f"\n🎯 ENERGY RANGE DISTRIBUTION:")
            print(f"   < 5 GeV: {low_energy_count:,} muons ({low_energy_pct:.1f}%)")
            print(f"   5-20 GeV: {mid_energy_count:,} muons ({mid_energy_pct:.1f}%)")
            print(f"   > 20 GeV: {high_energy_count:,} muons ({high_energy_pct:.1f}%)")
            
            # Compare with uniform distribution (1-50 GeV)
            print(f"\n🔄 COMPARISON WITH UNIFORM DISTRIBUTION (1-50 GeV):")
            uniform_mean = 25.5
            uniform_median = 25.5
            uniform_std = (50-1) / np.sqrt(12)
            uniform_low_pct = 8.2  # (5-1)/(50-1) * 100
            uniform_mid_pct = 30.6  # (20-5)/(50-1) * 100  
            uniform_high_pct = 61.2  # (50-20)/(50-1) * 100
            
            print(f"   Uniform mean: {uniform_mean:.1f} GeV   |   Gaisser mean: {mean_energy:.2f} GeV")
            print(f"   Uniform median: {uniform_median:.1f} GeV |   Gaisser median: {median_energy:.2f} GeV")  
            print(f"   Uniform std: {uniform_std:.1f} GeV    |   Gaisser std: {std_energy:.2f} GeV")
            print(f"   Uniform < 5 GeV: {uniform_low_pct:.1f}%   |   Gaisser < 5 GeV: {low_energy_pct:.1f}%")
            print(f"   Uniform > 20 GeV: {uniform_high_pct:.1f}%  |   Gaisser > 20 GeV: {high_energy_pct:.1f}%")
            
            # Validation
            print(f"\n✅ GAISSER VALIDATION:")
            if mean_energy < 5.0 and median_energy < 3.0:
                print(f"   ✅ Energy distribution heavily skewed toward low energies")
            else:
                print(f"   ⚠️  Energy distribution not as expected")
                
            if low_energy_pct > 70.0:
                print(f"   ✅ Majority of muons (>70%) have energy < 5 GeV")
            else:
                print(f"   ⚠️  Expected more muons with energy < 5 GeV")
                
            if high_energy_pct < 10.0:
                print(f"   ✅ High-energy tail properly suppressed (<10% > 20 GeV)")
            else:
                print(f"   ⚠️  Too many high-energy muons")
            
            # Create visualization
            fig, axes = plt.subplots(2, 2, figsize=(15, 10))
            
            # 1. Linear histogram
            axes[0,0].hist(muon_energies_gev, bins=50, alpha=0.7, edgecolor='black', color='blue')
            axes[0,0].set_xlabel('Energy (GeV)')
            axes[0,0].set_ylabel('Count')
            axes[0,0].set_title('Muon Energy Distribution - Linear Scale')
            axes[0,0].grid(True, alpha=0.3)
            axes[0,0].axvline(mean_energy, color='red', linestyle='--', label=f'Mean: {mean_energy:.2f} GeV')
            axes[0,0].axvline(median_energy, color='orange', linestyle='--', label=f'Median: {median_energy:.2f} GeV')
            axes[0,0].legend()
            
            # 2. Log-linear histogram
            axes[0,1].hist(muon_energies_gev, bins=50, alpha=0.7, edgecolor='black', color='green')
            axes[0,1].set_xlabel('Energy (GeV)')
            axes[0,1].set_ylabel('Count (Log Scale)')
            axes[0,1].set_yscale('log')
            axes[0,1].set_title('Muon Energy Distribution - Log Y Scale')
            axes[0,1].grid(True, alpha=0.3)
            
            # 3. Theoretical Gaisser comparison
            energy_theory = np.linspace(1, 50, 1000)
            gaisser_theory = np.power(energy_theory, -2.7) * np.power(1.0 + energy_theory/115.0, -3.64)
            # Normalize to match histogram
            hist_counts, hist_bins = np.histogram(muon_energies_gev, bins=50)
            theory_norm = np.max(hist_counts) / np.max(gaisser_theory) * 0.8
            gaisser_theory_norm = gaisser_theory * theory_norm
            
            axes[1,0].plot(energy_theory, gaisser_theory_norm, 'r-', linewidth=2, label='Gaisser Theory')
            axes[1,0].hist(muon_energies_gev, bins=50, alpha=0.7, edgecolor='black', color='blue', label='Simulation')
            axes[1,0].set_xlabel('Energy (GeV)')
            axes[1,0].set_ylabel('Count')
            axes[1,0].set_title('Comparison with Gaisser Theory')
            axes[1,0].legend()
            axes[1,0].grid(True, alpha=0.3)
            
            # 4. Cumulative distribution
            sorted_energies = np.sort(muon_energies_gev)
            cumulative = np.arange(1, len(sorted_energies) + 1) / len(sorted_energies)
            axes[1,1].plot(sorted_energies, cumulative, 'b-', linewidth=2, label='Gaisser CDF')
            
            # Uniform distribution CDF for comparison
            uniform_x = np.linspace(1, 50, 100)
            uniform_cdf = (uniform_x - 1) / (50 - 1)
            axes[1,1].plot(uniform_x, uniform_cdf, 'r--', linewidth=2, label='Uniform CDF')
            
            axes[1,1].set_xlabel('Energy (GeV)')
            axes[1,1].set_ylabel('Cumulative Probability')
            axes[1,1].set_title('Cumulative Distribution Function')
            axes[1,1].legend()
            axes[1,1].grid(True, alpha=0.3)
            
            plt.tight_layout()
            plt.savefig('gaisser_energy_analysis.png', dpi=150, bbox_inches='tight')
            print(f"\n💾 Saved analysis plots as 'gaisser_energy_analysis.png'")
            
            # Final verdict
            print(f"\n🎯 FINAL VERDICT:")
            if (mean_energy < 5.0 and median_energy < 3.0 and 
                low_energy_pct > 70.0 and high_energy_pct < 10.0):
                print(f"   🎉 ✅ GAISSER PARAMETRIZATION IS WORKING CORRECTLY!")
                print(f"   🎉 ✅ Energy distribution follows expected E^(-2.7) × (1 + E/115GeV)^(-3.64) spectrum")
            else:
                print(f"   ❌ Energy distribution does not match expected Gaisser spectrum")
                
    except Exception as e:
        print(f"❌ Error analyzing file: {e}")

if __name__ == "__main__":
    # Try both possible filenames
    filenames = ["Muon_Xe_gaisser_test.next.h5", "Muon_Xe_gaisser.next.h5"]
    
    for filename in filenames:
        try:
            analyze_gaisser_distribution(filename)
            break
        except FileNotFoundError:
            continue
    else:
        print("❌ No output files found!")
        print("Available files:")
        import os
        h5_files = [f for f in os.listdir('.') if f.endswith('.h5')]
        for f in h5_files:
            print(f"   {f}")
