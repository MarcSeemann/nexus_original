# IonGenerator Implementation for Ba-133

## Overview
Successfully switched from Barium133Generator to IonGenerator for realistic cascade decay simulation.

## Files Created

### 1. `macros/Ba133_IonGenerator.init.mac`
- Initializes the simulation framework
- Registers IonGenerator as the primary generator
- Loads all necessary physics lists
- Registers event actions for filtering
- Executes the configuration macro

### 2. `macros/Ba133_IonGenerator.config.mac`
- Sets Ba-133 parameters:
  - Atomic number: 56 (Barium)
  - Mass number: 133
  - Energy level: 0 MeV (ground state)
  - Decay at time zero: true (forces decay immediately)
  - Region: INSIDE_CIGAR (matches Cigar geometry source position)
- Configures Cigar detector:
  - Pressure: 5.5 bar
  - Gas: Xenon
  - Fiber type: Y11 with TPB coating
- Output file: Ba133_IonGenerator_Xe_5.5bar.next

## Changes to Cigar.cc
**No changes required** - The geometry already has:
- `inside_cigar_` sampler set up at the Barium source position
- CylinderPointSampler with radius 2.0 mm and height 1.0 mm
- Position on top of vacuum chamber at (0, 105 mm, 0)
- `INSIDE_CIGAR` region properly defined in `GenerateVertex()`

## How IonGenerator Works

The IonGenerator creates Ba-133 ions at the source position defined in Cigar geometry:
1. Generates Ba-133 ion at position from `inside_cigar_` sampler
2. Sets decay_at_time_zero=true to force immediate decay
3. Geant4's built-in radioactive decay simulation handles cascade decays
4. Produces cascade gammas with correct branching ratios
5. DefaultEventAction filters for primary gammas with gas ionization

## Key Advantages over Barium133Generator

1. **Realistic Physics**: Uses Geant4's built-in radioactive decay tables (ENSDF data)
2. **Cascade Decays**: Automatically produces multi-gamma cascades with correct probabilities
3. **Maintenance**: No need to update decay data manually
4. **Flexibility**: Can easily change energy level or initial state
5. **Uncertainty**: Includes proper branching ratio uncertainties

## Usage

Run the simulation with:
```bash
./nexus macros/Ba133_IonGenerator.init.mac
```

## Configuration Parameters

You can modify in `Ba133_IonGenerator.config.mac`:
- `/Generator/IonGenerator/atomic_number` - Atomic number
- `/Generator/IonGenerator/mass_number` - Mass number
- `/Generator/IonGenerator/energy_level` - Excited state (in MeV)
- `/Generator/IonGenerator/decay_at_time_zero` - Force immediate decay
- `/Generator/IonGenerator/region` - Vertex generation region
- `/Geometry/Cigar/pressure` - Chamber pressure
- `/Geometry/Cigar/gas` - Argon or Xenon
- Output filename and other physics/tracking settings

## Testing

With DefaultEventAction filtering enabled, you should see:
- Events with one or more primary gammas
- Cascade patterns matching Ba-133 decay structure
- Proper energy spectrum from cascade decays
- Reduced electron contamination (primary gammas only)
