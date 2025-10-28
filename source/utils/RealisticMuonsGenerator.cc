// ----------------------------------------------------------------------------
// nexus | RealisticMuonsGenerator.cc
//
// A more realistic muon vertex and direction generator based on
// the empirical sea-level flux ~ cos^2(theta).
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "RealisticMuonsGenerator.h"
#include <Randomize.hh>
#include <G4ThreeVector.hh>
#include <CLHEP/Units/PhysicalConstants.h>
#include <cmath>
#include <iostream>

namespace nexus {

  using namespace CLHEP;

  RealisticMuonsGenerator::RealisticMuonsGenerator(G4double x, G4double yPoint, G4double z)
    : x_(x), yPoint_(yPoint), z_(z)
  {}

  G4ThreeVector RealisticMuonsGenerator::GenerateVertex()
  {
    // Generate random (x,z) uniformly over the surface
    G4double x = -x_ + G4UniformRand() * 2.0 * x_;
    G4double z = -z_ + G4UniformRand() * 2.0 * z_;

    return G4ThreeVector(x, yPoint_, z);
  }

  G4ThreeVector RealisticMuonsGenerator::GenerateDirection()
  {
    // Generate zenith angle according to I(θ) ∝ cos²(θ)
    // (muon intensity proportional to cos²θ)
    // For uniform sampling in cos²(θ): cos(θ) = √U
    
    G4double uniform = G4UniformRand();
    G4double cosTheta = std::sqrt(uniform);  // CORRECTED: cos(θ) = √U for uniform cos²(θ) 
    G4double zenith = std::acos(cosTheta);

    // Uniform azimuthal angle
    G4double azimuth = 2.0 * M_PI * G4UniformRand();

    // Direction components: muons coming from above (negative y)
    G4double dx = std::sin(zenith) * std::sin(azimuth);
    G4double dy = -std::cos(zenith);  // vertical axis = y
    G4double dz = -std::sin(zenith) * std::cos(azimuth);

    return G4ThreeVector(dx, dy, dz).unit();
  }


} // end namespace nexus
