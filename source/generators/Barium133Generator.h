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

    std::vector<G4double> gamma_energies_;        // Gamma ray energies in keV
    std::vector<G4double> cumulative_intensities_; // Cumulative probabilities for energy selection
    
    G4String region_;
    G4ParticleDefinition* gamma_particle_;
  };

} // end namespace nexus

#endif
