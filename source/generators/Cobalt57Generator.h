// ----------------------------------------------------------------------------
// nexus | Cobalt57Generator.h
//
// This class is the primary generator for Cobalt-57 decay
// producing gamma rays at specific energies with realistic intensities.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef COBALT57_GENERATOR_H
#define COBALT57_GENERATOR_H

#include <vector>
#include <G4VPrimaryGenerator.hh>

class G4Event;
class G4ParticleDefinition;
class G4GenericMessenger;

namespace nexus {

  class GeometryBase;

  /// Generator for Cobalt-57 decay scheme producing gamma rays
  /// at specific energies: 14.4129, 122.06065, and 136.47356 keV, 230.4 keV, 339.69 keV
  /// , 352.33 keV, 366.8 keV, 570.09 keV, 692.41 keV, 706.54 keV
  /// with realistic branching ratios based on ENSDF data

  class Cobalt57Generator: public G4VPrimaryGenerator
  {
  public:
    // Constructor
    Cobalt57Generator();
    // Destructor
    ~Cobalt57Generator();

    void GeneratePrimaryVertex(G4Event* evt);

  private:

    G4GenericMessenger* msg_;
    const GeometryBase* geom_;

    std::vector<G4double> gamma_energies_;        // Gamma ray energies in keV
    std::vector<G4double> cumulative_intensities_; // Cumulative probabilities for energy selection
    
    G4String region_;
    G4ParticleDefinition* gamma_particle_;
    
    // Directional control parameters
    G4bool use_hemisphere_emission_;              // Enable/disable hemisphere emission
    G4ThreeVector emission_direction_;            // Preferred emission direction (for hemisphere)
  };

} // end namespace nexus

#endif
