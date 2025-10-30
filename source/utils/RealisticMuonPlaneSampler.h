// ----------------------------------------------------------------------------
// nexus | RealisticMuonPlaneSampler.h
//
// This class is a sampler of random uniform points in a rectangular plane
// that can be oriented in 3D space. Used for realistic muon generation
// where muons are created on a plane surface.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef REALISTIC_MUON_PLANE_SAMPLER_H
#define REALISTIC_MUON_PLANE_SAMPLER_H

#include <G4ThreeVector.hh>
#include <G4RotationMatrix.hh>

namespace nexus {

  class RealisticMuonPlaneSampler
  {
  public:
    /// Constructor
    /// @param width: Width of the plane (X dimension)
    /// @param height: Height of the plane (Z dimension) 
    /// @param center: Center position of the plane
    /// @param rotation: Optional rotation matrix for plane orientation
    RealisticMuonPlaneSampler(G4double width, G4double height, 
                             const G4ThreeVector& center, 
                             G4RotationMatrix* rotation = nullptr);

    /// Destructor
    ~RealisticMuonPlaneSampler();

    /// Returns a random vertex on the plane surface
    G4ThreeVector GenerateVertex(const G4String& region);
    
    /// Get the plane normal vector (for direction calculations)
    G4ThreeVector GetPlaneNormal() const;

  private:
    /// Default constructor is hidden
    RealisticMuonPlaneSampler();

  private:
    G4double width_;          ///< Plane width (X dimension)
    G4double height_;         ///< Plane height (Z dimension)
    G4ThreeVector center_;    ///< Center position of the plane
    G4RotationMatrix* rotation_; ///< Plane orientation (optional)
    G4ThreeVector normal_;    ///< Plane normal vector
  };

  // inline methods ..................................................
  inline RealisticMuonPlaneSampler::~RealisticMuonPlaneSampler() { }

} // namespace nexus

#endif
