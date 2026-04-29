#!/usr/bin/env python3
"""
Analysis script to compare Gaisser vs Gaisser/Tang energy distributions
Based on the Frontiers in Energy Research paper
"""

import numpy as np
import matplotlib.pyplot as plt
import h5py
from scipy.integrate import quad

def gaisser_original(E, E_min=1.0, E_max=50.0):
    """Original Gaisser model from our previous implementation"""
    epsilon_star = 115.0  # GeV
    gamma = 2.7
    
    # Normalize over the energy range
    def integrand(x):
        return x**(-gamma) * (1.0 + x/epsilon_star)**(-3.64)
    
    norm, _ = quad(integrand, E_min, E_max)
    
    return (E**(-gamma) * (1.0 + E/epsilon_star)**(-3.64)) / norm

def gaisser_tang(E, E_min=1.0, E_max=50.0):
    """Improved Gaisser/Tang model from the paper"""
    gamma = 2.7
    B_G = 0.054  # Kaon contribution factor
    
    # Pion and kaon terms (for vertical incidence, theta=0)
    pion_term = 1.0 / (1.0 + 1.1 * E / 115.0)
    kaon_term = B_G / (1.0 + 1.1 * E / 810.0)
    total_term = pion_term + kaon_term
    
    # Normalize over the energy range
    def integrand(x):
        p_term = 1.0 / (1.0 + 1.1 * x / 115.0)
        k_term = B_G / (1.0 + 1.1 * x / 810.0)
        return x**(-gamma) * (p_term + k_term)
    
    norm, _ = quad(integrand, E_min, E_max)
    
    return (E**(-gamma) * total_term) / norm

def analyze_simulation_data(filename):
    """Analyze energy distribution from HDF5 simulation file"""
    try:
        with h5py.File(filename, 'r') as f:
            print(f"📊 Analyzing file: {filename}")
            
            # Check available datasets
            print(f"Available groups: {list(f.keys())}")
            
            if 'particles' in f:
                particles = f['particles']
                print(f"Particle dataset shape: {particles.shape}")
                
                # Extract muon energies (assuming muons are particle_name == 13 or -13)
                particle_names = particles['particle_name']
                initial_energies = particles['initial_energy'] / 1000.0  # Convert MeV to GeV
                
                muon_mask = (np.abs(particle_names) == 13)  # Muons are ±13
                muon_energies = initial_energies[muon_mask]
                
                print(f"Found {len(muon_energies)} muons")
                return muon_energies
            else:
                print("❌ No 'particles' dataset found in file")
                return np.array([])
                
    except Exception as e:
        print(f"❌ Error reading file {filename}: {e}")
        return np.array([])

def plot_energy_comparisons():
    """Plot comparison between different Gaisser models"""
    
    # Energy range for plotting
    E_range = np.logspace(0, 1.7, 100)  # 1 to 50 GeV
    
    # Calculate theoretical distributions
    gaisser_orig_dist = [gaisser_original(E) for E in E_range]
    gaisser_tang_dist = [gaisser_tang(E) for E in E_range]
    
    # Create comparison plot
    plt.figure(figsize=(12, 8))
    
    # Plot theoretical curves
    plt.subplot(2, 2, 1)
    plt.loglog(E_range, gaisser_orig_dist, 'r-', label='Original Gaisser', linewidth=2)
    plt.loglog(E_range, gaisser_tang_dist, 'b-', label='Gaisser/Tang (Improved)', linewidth=2)
    plt.xlabel('Energy (GeV)')
    plt.ylabel('Normalized Differential Flux')
    plt.title('Theoretical Energy Spectra Comparison')
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    # Plot linear scale comparison
    plt.subplot(2, 2, 2)
    plt.plot(E_range, gaisser_orig_dist, 'r-', label='Original Gaisser', linewidth=2)
    plt.plot(E_range, gaisser_tang_dist, 'b-', label='Gaisser/Tang (Improved)', linewidth=2)
    plt.xlabel('Energy (GeV)')
    plt.ylabel('Normalized Differential Flux')
    plt.title('Linear Scale Comparison')
    plt.legend()
    plt.grid(True, alpha=0.3)
    plt.xlim(1, 20)
    
    # Show difference between models
    plt.subplot(2, 2, 3)
    ratio = np.array(gaisser_tang_dist) / np.array(gaisser_orig_dist)
    plt.semilogx(E_range, ratio, 'g-', linewidth=2)
    plt.xlabel('Energy (GeV)')
    plt.ylabel('Gaisser/Tang / Original Gaisser')
    plt.title('Ratio of Improved vs Original Model')
    plt.grid(True, alpha=0.3)
    plt.axhline(y=1, color='k', linestyle='--', alpha=0.5)
    
    # Calculate statistics
    plt.subplot(2, 2, 4)
    
    # Theoretical statistics for both models
    E_test = np.random.choice(E_range, size=10000, p=gaisser_orig_dist/np.sum(gaisser_orig_dist))
    orig_mean = np.mean(E_test)
    orig_median = np.median(E_test)
    orig_below_5 = np.sum(E_test < 5) / len(E_test) * 100
    
    E_test_tang = np.random.choice(E_range, size=10000, p=gaisser_tang_dist/np.sum(gaisser_tang_dist))
    tang_mean = np.mean(E_test_tang)
    tang_median = np.median(E_test_tang)
    tang_below_5 = np.sum(E_test_tang < 5) / len(E_test_tang) * 100
    
    stats_text = f"""
    Model Comparison:
    
    Original Gaisser:
    • Mean: {orig_mean:.2f} GeV
    • Median: {orig_median:.2f} GeV
    • <5 GeV: {orig_below_5:.1f}%
    
    Gaisser/Tang (Improved):
    • Mean: {tang_mean:.2f} GeV
    • Median: {tang_median:.2f} GeV
    • <5 GeV: {tang_below_5:.1f}%
    
    Key Improvements:
    • Better low-energy accuracy
    • Includes kaon contributions
    • More realistic cosmic ray spectrum
    """
    
    plt.text(0.05, 0.95, stats_text, transform=plt.gca().transAxes, 
             fontsize=10, verticalalignment='top', fontfamily='monospace',
             bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.8))
    plt.axis('off')
    
    plt.tight_layout()
    plt.savefig('gaisser_tang_comparison.png', dpi=300, bbox_inches='tight')
    print("📊 Comparison plot saved as 'gaisser_tang_comparison.png'")
    plt.show()

def main():
    """Main analysis function"""
    print("🚀 Gaisser vs Gaisser/Tang Model Analysis")
    print("=" * 50)
    
    # Plot theoretical comparisons
    plot_energy_comparisons()
    
    # Try to analyze existing simulation files
    potential_files = [
        'Muon_Xe_gaisser_500k_5.5bar.next.h5',
        'output.next.h5',
        'Cigar_Xe_1.5bar_alpha_tracking.next.h5'
    ]
    
    for filename in potential_files:
        try:
            energies = analyze_simulation_data(filename)
            if len(energies) > 0:
                print(f"\n📈 Analysis for {filename}:")
                print(f"   Mean energy: {np.mean(energies):.2f} GeV")
                print(f"   Median energy: {np.median(energies):.2f} GeV")
                print(f"   Energy range: {np.min(energies):.2f} - {np.max(energies):.2f} GeV")
                print(f"   Muons < 5 GeV: {np.sum(energies < 5) / len(energies) * 100:.1f}%")
                break
        except:
            continue
    
    print("\n✅ Analysis complete!")
    print("\n🔬 Scientific Summary:")
    print("The Gaisser/Tang model provides significant improvements over the basic Gaisser model:")
    print("1. More accurate in the low-energy region (1-10 GeV)")
    print("2. Includes contributions from both pion and kaon decay")
    print("3. Better matches experimental data according to the Frontiers paper")
    print("4. Reduces overestimation of low-energy muon flux")

if __name__ == "__main__":
    main()
