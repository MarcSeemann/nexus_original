# Realistic Muon Generator Documentation

## Overview
The `RealisticMuonGenerator` is a new primary particle generator for nexus that generates atmospheric muons with realistic angular and energy distributions. Unlike the `SingleParticleGenerator` which uses uniform angular distributions, this generator implements the cos²θ distribution typical of sea-level cosmic ray muons.

## Features

### Angular Distribution
- **Zenith angle**: Follows cos²θ distribution where θ is the angle from vertical
- **Azimuthal angle**: Uniform distribution (0 to 2π)
- **Configurable maximum zenith angle**: Default 85° (prevents nearly horizontal muons)

### Energy Distribution  
- **Uniform distribution** between user-specified min/max energies
- **Typical range**: 1-10 GeV (representative of atmospheric muon spectrum)
- **Particle types**: Randomly chooses between μ⁺ and μ⁻ (approximately equal rates)

### Position Generation
- Uses the geometry's region system (same as SingleParticleGenerator)
- New `MUON_PLANE` region in Cigar geometry
- Plane positioned above detector for realistic muon entry

## Usage

### Basic Configuration
```bash
# Set the generator
/nexus/primaryGeneration RealisticMuonGenerator

# Energy range (in GeV)
/Generator/RealisticMuon/min_energy 1 GeV
/Generator/RealisticMuon/max_energy 10 GeV

# Region for muon generation
/Generator/RealisticMuon/region MUON_PLANE

# Optional: limit zenith angle (default: 85 degrees)
/Generator/RealisticMuon/max_zenith_angle 80 deg
```

### Available Commands
- `/Generator/RealisticMuon/min_energy`: Minimum kinetic energy
- `/Generator/RealisticMuon/max_energy`: Maximum kinetic energy  
- `/Generator/RealisticMuon/region`: Geometry region for vertex generation
- `/Generator/RealisticMuon/max_zenith_angle`: Maximum zenith angle from vertical

## Geometry Integration

### New Components
1. **RealisticMuonPlaneSampler**: Generates uniform points on a rectangular plane
2. **MUON_PLANE region**: Added to Cigar geometry for muon generation

### Plane Configuration (in Cigar geometry)
```cpp
// Muon plane positioned above chamber
G4double muon_plane_width = 2 * chamber_diameter;
G4double muon_plane_height = 2 * chamber_diameter;  
G4double muon_plane_y = chamber_diameter + 50*cm;
```

## Physics Background

### Cos²θ Distribution
Atmospheric muons at sea level follow a cos²θ angular distribution due to:
- **Atmospheric absorption**: More material to traverse at larger zenith angles
- **Geometric effects**: Solid angle considerations
- **Production mechanisms**: Pion/kaon decay in atmosphere

### Implementation
Uses rejection sampling:
1. Generate random zenith angle θ ∈ [0, max_zenith_angle]
2. Calculate weight = cos²θ × sin θ  
3. Accept/reject based on weight compared to maximum possible weight

## Comparison with SingleParticleGenerator

| Feature | SingleParticleGenerator | RealisticMuonGenerator |
|---------|------------------------|------------------------|
| Angular Distribution | Uniform or user-defined | Automatic cos²θ |
| Particle Type | User-specified | Automatic μ±⁺/μ⁻ |
| Energy | Uniform in range | Uniform in range |
| Physics Realism | Generic | Atmospheric muons |
| Configuration | More parameters | Simpler setup |

## Example Macros

### Basic Realistic Muons
```bash
/nexus/primaryGeneration RealisticMuonGenerator
/Generator/RealisticMuon/min_energy 1 GeV
/Generator/RealisticMuon/max_energy 10 GeV
/Generator/RealisticMuon/region MUON_PLANE
/run/beamOn 1000
```

### Comparison Study
See `macros/generators/MuonComparison.mac` for side-by-side comparison.

## Files Created
- `source/generators/RealisticMuonGenerator.h/.cc`
- `source/utils/RealisticMuonPlaneSampler.h/.cc` 
- `macros/generators/RealisticMuon.mac`
- `macros/generators/MuonComparison.mac`

## Notes
- The generator automatically handles μ⁺/μ⁻ selection
- Direction vectors point downward (negative Y in Geant4 coordinates)
- Plane sampler can be extended for other geometries beyond Cigar
- Maximum zenith angle prevents unrealistic near-horizontal muons
