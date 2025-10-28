// ----------------------------------------------------------------------------
// nexus | MuonsPointSampler.cc
//
// This class is a sampler of random uniform points in a rectangular surface,
// with fixed y coordinate.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "MuonsPointSampler.h"

#include <Randomize.hh>
#include <G4RotationMatrix.hh>

using namespace nexus;

MuonsPointSampler::MuonsPointSampler(G4double x, G4double yPoint, G4double z):
  x_(x), yPoint_(yPoint),z_(z)
{
}

G4ThreeVector MuonsPointSampler::GenerateVertex()
{

  G4ThreeVector vtx = GetXZPointInMuonsPlane();

  return vtx;
}

G4ThreeVector MuonsPointSampler::GetXZPointInMuonsPlane()
{

  G4double x = x_ * (G4UniformRand() - 0.5);
  G4double z = z_ * (G4UniformRand() - 0.5);
  G4ThreeVector vtx(x, yPoint_, z);

  return vtx;
}
