// ----------------------------------------------------------------------------
// nexus | RealisticMuonPlaneSampler.cc
//
// This class is a sampler of random uniform points in a rectangular plane
// that can be oriented in 3D space. Used for realistic muon generation
// where muons are created on a plane surface.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "RealisticMuonPlaneSampler.h"

#include <Randomize.hh>

using namespace nexus;

RealisticMuonPlaneSampler::RealisticMuonPlaneSampler(G4double width, G4double height,
                                                   const G4ThreeVector& center,
                                                   G4RotationMatrix* rotation):
  width_(width), height_(height), center_(center), rotation_(rotation)
{
  // Calculate the plane normal vector
  if (rotation_) {
    // If rotated, the normal is the rotated Y-axis
    normal_ = G4ThreeVector(0., 1., 0.);
    normal_ = (*rotation_) * normal_;
  } else {
    // Default normal is positive Y-axis (upward)
    normal_ = G4ThreeVector(0., 1., 0.);
  }
}

G4ThreeVector RealisticMuonPlaneSampler::GenerateVertex(const G4String& region)
{
  // For now, ignore the region parameter and just generate on the plane
  // In the future, could support different regions like "INSIDE", "SURFACE", etc.
  
  // Generate random point in local plane coordinates
  G4double local_x = width_ * (G4UniformRand() - 0.5);
  G4double local_z = height_ * (G4UniformRand() - 0.5);
  G4ThreeVector local_point(local_x, 0., local_z);
  
  // Apply rotation if specified
  if (rotation_) {
    local_point = (*rotation_) * local_point;
  }
  
  // Translate to world coordinates
  G4ThreeVector world_point = center_ + local_point;
  
  return world_point;
}

G4ThreeVector RealisticMuonPlaneSampler::GetPlaneNormal() const
{
  return normal_;
}
