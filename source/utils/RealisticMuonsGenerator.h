#ifndef REALISTIC_MUONS_GENERATOR_H
#define REALISTIC_MUONS_GENERATOR_H

#include <G4ThreeVector.hh>
#include <globals.hh>

namespace nexus {

  class RealisticMuonsGenerator {
  public:
    RealisticMuonsGenerator(G4double x, G4double yPoint, G4double z);

    G4ThreeVector GenerateVertex();    // position on surface
    G4ThreeVector GenerateDirection(); // direction with cos^2(theta) dependence

  private:
    G4double x_;
    G4double yPoint_;
    G4double z_;
  };

} // namespace nexus

#endif
