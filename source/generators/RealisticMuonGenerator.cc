// ----------------------------------------------------------------------------
// nexus | RealisticMuonGenerator.cc (patched)
//
// Generator for cosmic ray muons with realistic cos²θ distribution
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "RealisticMuonGenerator.h"
#include "DetectorConstruction.h"
#include "GeometryBase.h"
#include "FactoryBase.h"

#include <G4Event.hh>
#include <G4GenericMessenger.hh>
#include <G4ParticleDefinition.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4PrimaryVertex.hh>
#include <G4Event.hh>
#include <G4RandomDirection.hh>
#include <Randomize.hh>
#include <vector>
#include <fstream>

#include "CLHEP/Units/SystemOfUnits.h"

using namespace nexus;

REGISTER_CLASS(RealisticMuonGenerator, G4VPrimaryGenerator)

RealisticMuonGenerator::RealisticMuonGenerator():
  G4VPrimaryGenerator(), msg_(0), particle_definition_(0),
  use_lsc_dist_(false), axis_rotation_(0), rPhi_(NULL), user_dir_{},
  energy_min_(0.), energy_max_(0.), region_(""), ang_file_(""), dist_name_(""),
  bInitialize_(false), geom_(0), geom_solid_(0), fRandomGeneral_(0), gen_rad_(0.)
{
  msg_ = new G4GenericMessenger(this, "/Generator/RealisticMuonGenerator/",
                "Control commands of RealisticMuonGenerator.");

  G4GenericMessenger::Command& min_energy =
    msg_->DeclareProperty("min_energy", energy_min_, "Minimum kinetic energy of the particle.");
  min_energy.SetUnitCategory("Energy");
  min_energy.SetParameterName("min_energy", false);
  min_energy.SetRange("min_energy>0.");

  G4GenericMessenger::Command& max_energy =
    msg_->DeclareProperty("max_energy", energy_max_, "Maximum kinetic energy of the particle");
  max_energy.SetUnitCategory("Energy");
  max_energy.SetParameterName("max_energy", false);
  max_energy.SetRange("max_energy>0.");

  msg_->DeclareProperty("region", region_,
                        "Region of the geometry where the vertex will be generated.");

  msg_->DeclareProperty("user_dir",  user_dir_, "Set fixed muon direction.");

  msg_->DeclareProperty("use_lsc_dist", use_lsc_dist_, 
                        "Use muon distribution according to input file (false for cos²θ)");

  msg_->DeclareProperty("ang_file", ang_file_, "Name of file with distributions");

  msg_->DeclareProperty("dist_name", dist_name_, "Name of distribution in file");

  G4GenericMessenger::Command& gen_radius =
    msg_->DeclareProperty("generation_radius", gen_rad_, "Radius of disc for generation");
  gen_radius.SetUnitCategory("Length");
  gen_radius.SetParameterName("gen_radius", false);
  gen_radius.SetRange("gen_radius>=0.");

  G4GenericMessenger::Command& rotation =
    msg_->DeclareProperty("azimuth_rotation", axis_rotation_,
              "Angle between north and nexus z in anticlockwise");
  rotation.SetUnitCategory("Angle");
  rotation.SetParameterName("azimuth", false);
  rotation.SetRange("azimuth>=0.");

  DetectorConstruction* detconst =
    (DetectorConstruction*) G4RunManager::GetRunManager()->GetUserDetectorConstruction();
  geom_ = detconst->GetGeometry();
}

// Note: Additional constructor parameters removed to match header file


RealisticMuonGenerator::~RealisticMuonGenerator()
{
  delete msg_;
  delete fRandomGeneral_;
  if (rPhi_) delete rPhi_;
}

void RealisticMuonGenerator::SetupAngles()
{
  // Rotation from the axes used in file.
  // Rotates anticlockwise about Y.
  rPhi_ = new G4RotationMatrix();
  rPhi_->rotateY(-axis_rotation_);
}

void RealisticMuonGenerator::GeneratePrimaryVertex(G4Event* event)
{
  // Initialize on the first event
  if (!bInitialize_){
    std::cout << "[RealisticMuonGenerator]: Generating muons with uniform azimuth "
                 "and cos^2 distribution for zenith " << std::endl;
    InitMuonZenithDist();  // Initialize the cos²θ distribution
    bInitialize_ = true;
  }

  // Init the rotation matrix
  if (rPhi_ == NULL)
    SetupAngles();

  particle_definition_ =
    G4ParticleTable::GetParticleTable()->FindParticle(MuonCharge());

  if (!particle_definition_)
    G4Exception("[RealisticMuonGenerator]", "SetParticleDefinition()",
                FatalException, " can not create a muon ");

  // Particle properties
  G4double mass = particle_definition_->GetPDGMass();
  
  // Set default momentum and angular variables
  G4ThreeVector p_dir;
  G4double zenith;
  G4double azimuth;
  G4double energy;
  G4double kinetic_energy;

  // User specified muon direction in some fixed direction
  if ( user_dir_ != G4ThreeVector{}) {
    p_dir   = user_dir_.unit();
    zenith  = p_dir.getTheta();
    azimuth = p_dir.getPhi() + pi; // change azimuth interval to be between 0, twopi
    
    // Generate uniform random energy in [E_min, E_max]
    kinetic_energy = energy_min_ + (energy_max_ - energy_min_) * G4UniformRand();
    energy = kinetic_energy + mass;
  }
  // Use GetDirection method for sampling (cos²θ distribution)
  else {
    // Sample kinetic energy uniformly (as before)
    kinetic_energy = energy_min_ + (energy_max_ - energy_min_) * G4UniformRand();
    energy = kinetic_energy + mass;

    GetDirection(p_dir, zenith, azimuth, energy, kinetic_energy, mass);
  }

  // Direction stored temporarily for this event

  // Generate vertex position using ProjectToVertex method
  G4ThreeVector position = ProjectToVertex(p_dir);

  // Check overlap (always returns true for RealisticMuonGenerator)
  if (!CheckOverlap(position, p_dir)) {
    // This shouldn't happen for RealisticMuonGenerator, but included for interface compatibility
    position = geom_->GenerateVertex(region_);
  }
  
  // DEBUG: Print vertex and direction info for first few muons
  static int vertex_count = 0;
  if (vertex_count < 10) {
    G4cout << "[DEBUG VERTEX " << vertex_count << "] pos=(" 
           << position.x()/cm << "," << position.y()/cm << "," 
           << position.z()/cm << ") cm, dir=(" << p_dir.x() << "," 
           << p_dir.y() << "," << p_dir.z() << ")" << G4endl;
    vertex_count++;
  }

  G4double pmod   = std::sqrt(energy*energy - mass*mass);
  G4double px = pmod * p_dir.x();
  G4double py = pmod * p_dir.y();
  G4double pz = pmod * p_dir.z();

  // Particle generated at start-of-event
  G4double time = 0.;
  
  // Create a new vertex
  G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

  // Create the new primary particle and set it some properties
  G4PrimaryParticle* particle =
    new G4PrimaryParticle(particle_definition_, px, py, pz);

  // Add particle to the vertex and this to the event
  vertex->SetPrimary(particle);
  event->AddPrimaryVertex(vertex);
}

G4String RealisticMuonGenerator::MuonCharge() const
{
  // Ratio of Mu+/Mu- is energy dependent, ranges from 1.3 to 1.5:
  // https://arxiv.org/pdf/1111.6675.pdf
  // Assume (approx) flat ratio up to energy range of interest ~6 TeV 
  // mu+/mu- ~1.3

  // Sample random number to give 1.3 to 1 ratio of Mu+/Mu-
  G4double rndCh = 2.3 * G4UniformRand(); 
  if (rndCh < 1.3)
    return "mu+";
  else
    return "mu-";
}

G4double RealisticMuonGenerator::GetZenith() const
{
  // Use direct sampling from cos²θ distribution
  // For cosmic ray flux I(θ) ∝ cos²θ, we want to sample θ such that
  // the probability density includes the solid angle factor sin(θ)
  // This gives us: P(θ) ∝ cos²(θ) × sin(θ)
  
  // We can use rejection sampling or inverse transform
  // For simplicity, use the analytical result: cos(θ) = U^(1/3)
  // where U is uniform random in [0,1]
  
  // Remove artificial constraints - let natural limits apply
  // G4UniformRand() already returns [0,1), and pow/acos handle edge cases naturally
  G4double uniform = G4UniformRand();
  if (uniform <= 0.0) uniform = 1e-15;  // Only protect against exactly zero
  
  // Sample cos(θ) and convert to θ
  G4double cos_zenith = std::pow(uniform, 1.0/3.0);
  G4double zenith = std::acos(cos_zenith);
  
  // DEBUG: Print first few zenith angles for diagnostics
  static int zenith_count = 0;
  if (zenith_count < 10) {
    G4cout << "[DEBUG ZENITH " << zenith_count << "] U=" << uniform 
           << ", cos(θ)=" << cos_zenith << ", θ=" << zenith/deg << "°" << G4endl;
    zenith_count++;
  }
  
  return zenith;
}

G4double RealisticMuonGenerator::GetAzimuth() const
{
  // Generate uniform azimuth angle in [0, 2π]
  G4double azimuth = twopi * G4UniformRand();
  return azimuth;
}

void RealisticMuonGenerator::GetDirection(G4ThreeVector& dir, G4double& zenith, G4double& azimuth,
                      G4double& energy, G4double& kinetic_energy, G4double mass)
{
  // For RealisticMuonGenerator, we use cos²θ distribution instead of file-based distribution
  zenith = GetZenith();
  azimuth = GetAzimuth();
  
  // Calculate the vector components of the muon
  // Zenith angle is measured from vertical (Y-axis), azimuth is uniform in [0, 2π]
  // For downward muons: Y component is negative, X and Z spread according to zenith angle
  dir.setX(sin(zenith) * cos(azimuth));  // X component (horizontal)
  dir.setY(-cos(zenith));               // Y component (downward, negative)
  dir.setZ(sin(zenith) * sin(azimuth));  // Z component (horizontal)

  // DEBUG: Print first few muons for diagnostic
  static int debug_count = 0;
  if (debug_count < 10) {
    G4cout << "[DEBUG " << debug_count << "] BEFORE rotation: zenith=" << zenith/deg 
           << "°, azimuth=" << azimuth/deg << "°, dir=(" 
           << dir.x() << "," << dir.y() << "," << dir.z() << ")" 
           << ", energy=" << kinetic_energy/GeV << " GeV" << G4endl;
  }

  // Apply any coordinate system rotation
  dir *= *rPhi_;
  
  // DEBUG: Print after rotation
  if (debug_count < 10) {
    G4cout << "[DEBUG " << debug_count << "] AFTER rotation: dir=(" 
           << dir.x() << "," << dir.y() << "," << dir.z() << ")" 
           << ", axis_rotation=" << axis_rotation_/deg << "°" << G4endl;
    debug_count++;
  }
}

G4ThreeVector RealisticMuonGenerator::ProjectToVertex(const G4ThreeVector& dir)
{
  // Use geometry-based vertex generator for the specified region
  if (geom_)
    return geom_->GenerateVertex(region_);

  // Fallback: origin
  return G4ThreeVector(0., 0., 0.);
}

G4bool RealisticMuonGenerator::CheckOverlap(const G4ThreeVector& vtx, const G4ThreeVector& dir)
{
  // For RealisticMuonGenerator, we don't use overlap checking
  // Always return true to accept all generated vertices
  return true;
}

void RealisticMuonGenerator::LoadMuonDistribution()
{
  // RealisticMuonGenerator doesn't load from file - it uses analytical cos²θ distribution
  // This method is included for interface compatibility but does nothing
}

void RealisticMuonGenerator::InitMuonZenithDist()
{
  // Using analytical sampling for cos²θ distribution via inverse transform
  // P(cos θ) ∝ cos²θ gives cos(θ) = U^(1/3) where U ~ Uniform[0,1]
  // This automatically accounts for the solid angle factor sin(θ)
  
  G4cout << "[RealisticMuonGenerator] Using analytical cos²θ sampling: cos(θ) = U^(1/3)" << G4endl;
}

// Note: GetLastDirection() method removed to match header file

// Note: Standalone test code removed - use external validation instead
