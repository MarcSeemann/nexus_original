// ----------------------------------------------------------------------------
// nexus | RealisticMuonGenerator.cc
//
// This class generates realistic muons with cos²θ angular distribution
// typical of atmospheric muons at sea level. Muons are generated on a 
// user-defined plane with energies between specified limits.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "RealisticMuonGenerator.h"

#include "DetectorConstruction.h"
#include "GeometryBase.h"
#include "RandomUtils.h"
#include "FactoryBase.h"

#include <G4GenericMessenger.hh>
#include <G4ParticleDefinition.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4PrimaryVertex.hh>
#include <G4Event.hh>
#include <G4MuonMinus.hh>
#include <G4MuonPlus.hh>
#include <Randomize.hh>

#include "CLHEP/Units/SystemOfUnits.h"

using namespace nexus;
using namespace CLHEP;

REGISTER_CLASS(RealisticMuonGenerator, G4VPrimaryGenerator)

RealisticMuonGenerator::RealisticMuonGenerator():
  G4VPrimaryGenerator(), msg_(0), muon_definition_(0),
  energy_min_(1.*GeV), energy_max_(10.*GeV), geom_(0)
{
  msg_ = new G4GenericMessenger(this, "/Generator/RealisticMuon/",
    "Control commands of realistic muon generator.");

  G4GenericMessenger::Command& min_energy =
    msg_->DeclareProperty("min_energy", energy_min_, 
                         "Minimum kinetic energy of the muon.");
  min_energy.SetUnitCategory("Energy");
  min_energy.SetParameterName("min_energy", false);
  min_energy.SetRange("min_energy>0.");

  G4GenericMessenger::Command& max_energy =
    msg_->DeclareProperty("max_energy", energy_max_, 
                         "Maximum kinetic energy of the muon");
  max_energy.SetUnitCategory("Energy");
  max_energy.SetParameterName("max_energy", false);
  max_energy.SetRange("max_energy>0.");

  msg_->DeclareProperty("region", region_,
                        "Region of the geometry where the vertex will be generated.");


  // Set muon as default particle (we'll randomly choose mu+ or mu-)
  muon_definition_ = G4MuonMinus::MuonMinus();

  DetectorConstruction* detconst = (DetectorConstruction*) 
    G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  geom_ = detconst->GetGeometry();
}

RealisticMuonGenerator::~RealisticMuonGenerator()
{
  delete msg_;
}

void RealisticMuonGenerator::GeneratePrimaryVertex(G4Event* event)
{
  // Randomly choose between mu+ and mu- (approximately equal rates at sea level)
  if (G4UniformRand() < 0.5) {
    muon_definition_ = G4MuonMinus::MuonMinus();
  } else {
    muon_definition_ = G4MuonPlus::MuonPlus();
  }

  // Generate uniform random energy in [E_min, E_max]
  G4double kinetic_energy = nexus::UniformRandomInRange(energy_max_, energy_min_);

  // Calculate momentum magnitude
  G4double mass = muon_definition_->GetPDGMass();
  G4double energy = kinetic_energy + mass;
  G4double pmod = std::sqrt(energy*energy - mass*mass);

  // Generate direction following cos²θ distribution
  G4ThreeVector p_dir = GenerateCosThetaSquaredDirection();
  G4ThreeVector p = pmod * p_dir;

  // Create the new primary particle
  auto particle = new G4PrimaryParticle(muon_definition_, p.x(), p.y(), p.z());

  // Generate an initial position for the particle using the geometry
  G4ThreeVector position = geom_->GenerateVertex(region_);

  // Particle generated at start-of-event
  G4double time = 0.;

  // Create a new vertex
  G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

  // Add particle to the vertex and this to the event
  vertex->SetPrimary(particle);
  event->AddPrimaryVertex(vertex);
}

G4ThreeVector RealisticMuonGenerator::GenerateCosThetaSquaredDirection() const
{
  // Generate zenith angle following cos²θ distribution
  G4double zenith_angle = GenerateZenithAngle();
  
  // Generate uniform azimuthal angle
  G4double azimuth_angle = 2 * pi * G4UniformRand();
  
  // Convert spherical coordinates to Cartesian
  // Note: Geant4 coordinate system has Y pointing up
  G4double sin_zenith = sin(zenith_angle);
  G4double cos_zenith = cos(zenith_angle);
  
  G4double px = sin_zenith * cos(azimuth_angle);
  G4double py = -cos_zenith;  // Negative for downward direction
  G4double pz = sin_zenith * sin(azimuth_angle);
  
  return G4ThreeVector(px, py, pz);
}

G4double RealisticMuonGenerator::GenerateZenithAngle() const
{
  // Generate zenith angle following cos²θ distribution using inverse transform method
  // PDF: f(θ) = (3) * cos²θ * sin θ for θ ∈ [0, π/2]
  // CDF: F(θ) = 1 - cos³θ
  // Inverse: θ = arccos((1-u)^(1/3)) where u ~ Uniform(0,1)
  
  G4double u = G4UniformRand();
  G4double zenith_angle = acos(std::cbrt(1.0 - u));
  
  return zenith_angle;
}
