// ----------------------------------------------------------------------------
// nexus | Barium133Generator.h
//
// This class is the primary generator for Barium-133 decay
// producing gamma rays at specific energies with realistic intensities.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef BARIUM133_GENERATOR_H
#define BARIUM133_GENERATOR_H

#include <vector>
#include <map>
#include <G4VPrimaryGenerator.hh>

class G4Event;
class G4ParticleDefinition;
class G4GenericMessenger;

namespace nexus {

  class GeometryBase;

  /// Generator for Barium-133 decay scheme producing gamma rays
  /// at specific energies: 53.16, 79.61, 80.98, 160.61, 223.24, 
  /// 276.40, 302.85, 356.01, and 383.85 keV
  /// with realistic branching ratios based on ENSDF data
  
  class Barium133Generator: public G4VPrimaryGenerator
  {
  public:
    // Constructor
    Barium133Generator();
    // Destructor
    ~Barium133Generator();

    void GeneratePrimaryVertex(G4Event* evt);

  private:

    G4GenericMessenger* msg_;
    const GeometryBase* geom_;

    // Cascade decay data structures for both decay chains
    struct Transition {
      size_t destination_level;    // Index of the destination energy level
      G4double transition_energy;  // Energy of the emitted gamma ray
      G4double branching_ratio;    // Probability of this transition
    };

    // Chain 1: Ground State (1/2+) - Primary decay branch
    std::vector<G4double> chain1_levels_;  // Energy levels in keV
    std::map<size_t, std::vector<Transition>> chain1_branches_;  // Transitions from each level

    // Chain 2: Isomeric State (11/2-) - Secondary decay branch
    std::vector<G4double> chain2_levels_;  // Energy levels in keV
    std::map<size_t, std::vector<Transition>> chain2_branches_;  // Transitions from each level

    // Relative weights for selecting between chains
    G4double chain1_weight_;  // Weight for ground state chain
    G4double chain2_weight_;  // Weight for isomeric state chain
    
    G4String region_;
    G4ParticleDefinition* gamma_particle_;
    
    // Directional control parameters
    G4bool use_hemisphere_emission_;              // Enable/disable hemisphere emission
    G4ThreeVector emission_direction_;            // Preferred emission direction (for hemisphere)
  };

} // end namespace nexus

#endif
