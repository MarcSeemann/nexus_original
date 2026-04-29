#!/usr/bin/env python3

import h5py
import numpy as np
import matplotlib.pyplot as plt

# Read the HDF5 file
filename = "Muon_Xe_gaisser_test.next.h5"

print(f"Debugging particle generation in {filename}")

with h5py.File(filename, 'r') as f:
    particles = f['MC/particles'][:]
    
    print(f"Total particles in file: {len(particles)}")
    print(f"Total events: {len(np.unique(particles['event_id']))}")
    
    # Check particle types
    particle_types = {}
    primary_count = 0
    secondary_count = 0
    
    for p in particles:
        particle_name = p['particle_name'].decode('utf-8')
        is_primary = p['primary'] == 1
        
        if particle_name not in particle_types:
            particle_types[particle_name] = {'total': 0, 'primary': 0, 'secondary': 0}
        
        particle_types[particle_name]['total'] += 1
        if is_primary:
            particle_types[particle_name]['primary'] += 1
            primary_count += 1
        else:
            particle_types[particle_name]['secondary'] += 1
            secondary_count += 1
    
    print(f"\nParticle type breakdown:")
    print(f"{'Particle':<15} {'Total':<8} {'Primary':<8} {'Secondary':<10}")
    print("-" * 45)
    for ptype, counts in sorted(particle_types.items()):
        print(f"{ptype:<15} {counts['total']:<8} {counts['primary']:<8} {counts['secondary']:<10}")
    
    print(f"\nSummary:")
    print(f"Total primary particles: {primary_count}")
    print(f"Total secondary particles: {secondary_count}")
    
    # Look specifically at muons (all types)
    all_muons = []
    primary_muons = []
    
    for p in particles:
        particle_name = p['particle_name'].decode('utf-8')
        if 'mu' in particle_name.lower():
            all_muons.append(p)
            if p['primary'] == 1:
                primary_muons.append(p)
    
    print(f"\nMuon analysis:")
    print(f"Total muons (all types): {len(all_muons)}")
    print(f"Primary muons: {len(primary_muons)}")
    
    if len(primary_muons) > 0:
        energies_gev = np.array([p['kin_energy']/1000.0 for p in primary_muons])
        print(f"Primary muon energies (GeV): {energies_gev}")
        print(f"Mean primary muon energy: {np.mean(energies_gev):.2f} GeV")
        
        # Check if this looks like Gaisser
        low_energy_fraction = np.sum(energies_gev < 5) / len(energies_gev)
        high_energy_fraction = np.sum(energies_gev > 30) / len(energies_gev)
        
        print(f"Fraction < 5 GeV: {low_energy_fraction:.2f}")
        print(f"Fraction > 30 GeV: {high_energy_fraction:.2f}")
        
        if low_energy_fraction > 0.7:  # Gaisser should have most events at low energy
            print("✅ Energy distribution looks Gaisser-like (most at low energy)")
        elif abs(np.mean(energies_gev) - 25.5) < 5:
            print("❌ Energy distribution looks uniform-like (mean ~25 GeV)")
        else:
            print("? Energy distribution is unclear with this sample size")
    
    # Check why we might be getting so few muons
    print(f"\nDebugging low muon count:")
    print(f"Events with particles: {len(np.unique(particles['event_id']))}")
    
    # Check event distribution
    events_with_muons = set()
    for p in primary_muons:
        events_with_muons.add(p['event_id'])
    
    print(f"Events that generated primary muons: {len(events_with_muons)} out of {len(np.unique(particles['event_id']))}")
    
    if len(events_with_muons) > 0:
        print(f"Event IDs with muons: {sorted(list(events_with_muons))}")
