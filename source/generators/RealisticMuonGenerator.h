// ----------------------------------------------------------------------------
// nexus | RealisticMuonGenerator.h
//
// This class generates realistic muons with cos²θ angular distribution
// typical of atmospheric muons at sea level. Muons are generated on a 
// user-defined plane with energies between specified limits.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef REALISTIC_MUON_GENERATOR_H
#define REALISTIC_MUON_GENERATOR_H

#include <G4VPrimaryGenerator.hh>
#include <G4ThreeVector.hh>

class G4GenericMessenger;
class G4Event;
class G4ParticleDefinition;

namespace nexus {

  class GeometryBase;

  class RealisticMuonGenerator: public G4VPrimaryGenerator
  {
  public:
    /// Constructor
    RealisticMuonGenerator();
    /// Destructor
    ~RealisticMuonGenerator();

    /// Generate primary vertex for the event
    void GeneratePrimaryVertex(G4Event*);

  private:
    /// Generate muon direction following cos²θ distribution
    G4ThreeVector GenerateCosThetaSquaredDirection() const;
    
    /// Generate zenith angle following cos²θ distribution  
    G4double GenerateZenithAngle() const;

  private:
    G4GenericMessenger* msg_;

    G4ParticleDefinition* muon_definition_;  ///< Muon particle definition

    G4double energy_min_; ///< Minimum kinetic energy
    G4double energy_max_; ///< Maximum kinetic energy

    const GeometryBase* geom_; ///< Pointer to the detector geometry

    G4String region_;     ///< Region for vertex generation

  };

} // end namespace nexus

#endif
