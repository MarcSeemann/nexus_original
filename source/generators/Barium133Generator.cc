// ----------------------------------------------------------------------------
// nexus | Barium133Generator.cc
//
// This class is the primary generator for Barium-133 decay
// producing gamma rays at specific energies with realistic intensities.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "Barium133Generator.h"

#include "DetectorConstruction.h"
#include "GeometryBase.h"
#include "FactoryBase.h"

#include <G4Event.hh>
#include <G4GenericMessenger.hh>
#include <G4RunManager.hh>
#include <G4ParticleTable.hh>
#include <G4RandomDirection.hh>
#include <Randomize.hh>

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include <cmath>

using namespace nexus;

REGISTER_CLASS(Barium133Generator, G4VPrimaryGenerator)

namespace nexus {

  using namespace CLHEP;

  Barium133Generator::Barium133Generator() : 
    G4VPrimaryGenerator(), geom_(0), msg_(0), gamma_particle_(0)
  {
    // Barium-133 gamma ray energies (in keV) and intensities (%) from ENSDF data
    // Energy [keV]    Intensity [%]
    gamma_energies_.push_back(53.1622*keV);   // 2.140725%
    gamma_energies_.push_back(79.6142*keV);   // 2.649535% 
    gamma_energies_.push_back(80.9979*keV);   // 32.94855%
    gamma_energies_.push_back(160.612*keV);   // 0.637874%
    gamma_energies_.push_back(223.2368*keV);  // 0.452965%
    gamma_energies_.push_back(276.3989*keV);  // 7.16057%
    gamma_energies_.push_back(302.8508*keV);  // 18.335775%
    gamma_energies_.push_back(356.0129*keV);  // 62.05%
    gamma_energies_.push_back(383.8485*keV);  // 8.941405%

    // Create cumulative probability distribution for energy selection
    std::vector<G4double> intensities = {
      2.140725,   // 53.16 keV
      2.649535,   // 79.61 keV  
      32.94855,   // 80.98 keV (strongest in this range)
      0.637874,   // 160.61 keV
      0.452965,   // 223.24 keV
      7.16057,    // 276.40 keV
      18.335775,  // 302.85 keV
      62.05,      // 356.01 keV (strongest overall)
      8.941405    // 383.85 keV
    };

    // Normalize intensities and create cumulative distribution
    G4double total_intensity = 0.0;
    for (size_t i = 0; i < intensities.size(); ++i) {
      total_intensity += intensities[i];
    }

    G4double cumulative = 0.0;
    for (size_t i = 0; i < intensities.size(); ++i) {
      cumulative += intensities[i] / total_intensity;
      cumulative_intensities_.push_back(cumulative);
    }

    G4cout << "[Barium133Generator] Initialized with " << gamma_energies_.size() 
           << " gamma ray energies" << G4endl;
    G4cout << "[Barium133Generator] Total normalized intensity: " << total_intensity << "%" << G4endl;

    // Set up messenger for region configuration
    msg_ = new G4GenericMessenger(this, "/Generator/Barium133Generator/",
                                  "Control commands of Barium133 generator.");

    msg_->DeclareProperty("region", region_,
                          "Set the region of the geometry where the vertex will be generated.");

    // Get gamma particle definition
    gamma_particle_ = G4ParticleTable::GetParticleTable()->FindParticle("gamma");
    if (!gamma_particle_) {
      G4Exception("[Barium133Generator]", "Constructor", FatalException, 
                  "Could not find gamma particle definition!");
    }

    // Get geometry from detector construction
    DetectorConstruction* detconst = (DetectorConstruction*)
      G4RunManager::GetRunManager()->GetUserDetectorConstruction();
    if (detconst) {
      geom_ = detconst->GetGeometry();
    } else {
      G4Exception("[Barium133Generator]", "Constructor", FatalException, 
                  "Could not get detector construction!");
    }
  }

  Barium133Generator::~Barium133Generator()
  {
    delete msg_;
  }

  void Barium133Generator::GeneratePrimaryVertex(G4Event* evt)
  {
    // Check if geometry is properly initialized
    if (!geom_) {
      G4Exception("[Barium133Generator]", "GeneratePrimaryVertex", FatalException, 
                  "Geometry not initialized!");
    }

    // Generate position from geometry
    G4ThreeVector position = geom_->GenerateVertex(region_);
    G4double time = 0.0;
    
    G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

    // Select gamma ray energy based on branching ratios
    G4double random_value = G4UniformRand();
    size_t selected_index = 0;
    
    for (size_t i = 0; i < cumulative_intensities_.size(); ++i) {
      if (random_value <= cumulative_intensities_[i]) {
        selected_index = i;
        break;
      }
    }

    G4double selected_energy = gamma_energies_[selected_index];
    
    // Generate random direction in downward hemisphere (negative Y direction)
    // For a hemisphere pointing in -Y direction:
    // - Y component should be negative (toward detector)
    // - X and Z components can be anything within the hemisphere
    
    G4double costheta = -G4UniformRand(); // Random between -1 and 0 (negative Y hemisphere)
    G4double phi = G4UniformRand() * 2.0 * M_PI; // Random azimuthal angle
    
    G4double sinTheta = std::sqrt(1.0 - costheta*costheta);
    
    // Create direction vector with Y as the polar axis (pointing down)
    G4ThreeVector momentum_direction(sinTheta*std::cos(phi),  // X component (lateral)
                                     costheta,                 // Y component (always negative = downward)
                                     sinTheta*std::sin(phi)); // Z component (lateral)

    // Calculate momentum components
    G4double mass = gamma_particle_->GetPDGMass();
    G4double total_energy = selected_energy + mass;
    G4double momentum_magnitude = std::sqrt(total_energy*total_energy - mass*mass);
    
    G4double px = momentum_magnitude * momentum_direction.x();
    G4double py = momentum_magnitude * momentum_direction.y();
    G4double pz = momentum_magnitude * momentum_direction.z();

    // Create primary gamma ray particle
    G4PrimaryParticle* gamma = new G4PrimaryParticle(gamma_particle_, px, py, pz);
    gamma->SetMass(mass);
    gamma->SetCharge(gamma_particle_->GetPDGCharge());
    gamma->SetPolarization(0.0, 0.0, 0.0);

    vertex->SetPrimary(gamma);
    evt->AddPrimaryVertex(vertex);

    // Debug output (can be removed in production)
    static G4int event_count = 0;
    if (event_count < 10) {
      G4cout << "[Barium133Generator] Event " << event_count 
             << ": Generated " << selected_energy/keV << " keV gamma ray" << G4endl;
    }
    event_count++;
  }

} // end namespace nexus
