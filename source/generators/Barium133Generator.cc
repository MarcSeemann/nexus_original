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
    G4VPrimaryGenerator(), geom_(0), msg_(0), gamma_particle_(0),
    use_hemisphere_emission_(false), emission_direction_(0., 0., 1.)
  {
    // Barium-133 has two decay chains. We implement cascade decays for realistic physics.
    // 
    // CHAIN 1: Ground State (1/2+) - 100% decay branch
    // Energy levels: 0 → 80.998 → 160.612 → 383.849 → 437.011 keV
    // This is the primary decay chain used in most applications.
    //
    // CHAIN 2: Isomeric State (11/2-) - Minor decay branch  
    // Energy levels: 0 → 12.327 → 288.252 keV
    // This is a secondary decay mode with lower intensity.

    // Define energy levels for Chain 1 (Ground state)
    chain1_levels_.push_back(0.0*keV);           // Ground state
    chain1_levels_.push_back(80.9979*keV);       // Level 1
    chain1_levels_.push_back(160.6121*keV);      // Level 2
    chain1_levels_.push_back(383.8491*keV);      // Level 3
    chain1_levels_.push_back(437.0113*keV);      // Level 4 (top)

    // Define branching from each level in Chain 1
    // Structure: {destination_level_index, transition_energy, branching_ratio}
    
    // From level 4 (437.011 keV): can go to 3, 2, or 1
    chain1_branches_[4].push_back({3, 53.1622*keV, 2.140725});    // to 383.849 keV
    chain1_branches_[4].push_back({2, 276.3989*keV, 7.16057});    // to 160.612 keV
    chain1_branches_[4].push_back({1, 356.0129*keV, 62.05});      // to 80.998 keV (dominant)
    
    // From level 3 (383.849 keV): can go to 2, 1, or 0
    chain1_branches_[3].push_back({2, 223.2368*keV, 0.452965});   // to 160.612 keV
    chain1_branches_[3].push_back({1, 302.8508*keV, 18.335775});  // to 80.998 keV
    chain1_branches_[3].push_back({0, 383.8485*keV, 8.941405});   // to ground
    
    // From level 2 (160.612 keV): can go to 1 or 0
    chain1_branches_[2].push_back({1, 79.6142*keV, 2.649535});    // to 80.998 keV
    chain1_branches_[2].push_back({0, 160.612*keV, 0.637874});    // to ground
    
    // From level 1 (80.998 keV): can go to 0
    chain1_branches_[1].push_back({0, 80.9979*keV, 32.94855});    // to ground

    // Define energy levels for Chain 2 (Isomeric state)
    chain2_levels_.push_back(0.0*keV);           // Ground state
    chain2_levels_.push_back(12.327*keV);        // Level 1
    chain2_levels_.push_back(288.252*keV);       // Level 2 (top)

    // Define branching from each level in Chain 2
    // From level 2 (288.252 keV): can go to 1 or 0
    chain2_branches_[2].push_back({1, 275.925*keV, 17.7});        // to 12.327 keV
    chain2_branches_[2].push_back({0, 288.0*keV, 0.000318});      // to ground (very rare)
    
    // From level 1 (12.327 keV): can go to 0
    chain2_branches_[1].push_back({0, 12.327*keV, 1.42});         // to ground

    // Branch probability to each chain (relative intensities)
    // Chain 1 is the dominant decay, Chain 2 is rare
    chain1_weight_ = 99.0;  // 99% of decays go through ground state chain
    chain2_weight_ = 1.0;   // 1% go through isomeric state chain

    G4cout << "[Barium133Generator] Initialized with cascade decay support" << G4endl;
    G4cout << "[Barium133Generator] Chain 1 (Ground State): " << chain1_levels_.size() << " levels" << G4endl;
    G4cout << "[Barium133Generator] Chain 2 (Isomeric State): " << chain2_levels_.size() << " levels" << G4endl;
    G4cout << "[Barium133Generator] Default: Isotropic emission (use /Generator/Barium133Generator/use_hemisphere_emission true to enable directional)" << G4endl;

    // Set up messenger for region configuration
    msg_ = new G4GenericMessenger(this, "/Generator/Barium133Generator/",
                                  "Control commands of Barium133 generator.");

    msg_->DeclareProperty("region", region_,
                          "Set the region of the geometry where the vertex will be generated.");
    
    msg_->DeclareProperty("use_hemisphere_emission", use_hemisphere_emission_,
                          "Enable hemisphere emission (true) or isotropic emission (false).");
    
    msg_->DeclarePropertyWithUnit("emission_direction", "mm", emission_direction_,
                                  "Set emission direction for hemisphere emission (unit vector).");

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

    // Generate position from geometry (single vertex for cascade)
    G4ThreeVector position = geom_->GenerateVertex(region_);
    G4double time = 0.0;
    
    G4PrimaryVertex* vertex = new G4PrimaryVertex(position, time);

    // Select which decay chain to use
    G4double total_weight = chain1_weight_ + chain2_weight_;
    G4double chain_random = G4UniformRand() * total_weight;
    
    bool use_chain1 = (chain_random < chain1_weight_);
    
    // Get reference to the appropriate chain
    std::vector<G4double>& levels = use_chain1 ? chain1_levels_ : chain2_levels_;
    std::map<size_t, std::vector<Transition>>& branches = use_chain1 ? chain1_branches_ : chain2_branches_;
    
    // Start from the highest energy level (top of the chain)
    size_t current_level = levels.size() - 1;  // Start at top level
    
    // Cascade down to ground state, emitting gammas
    while (current_level > 0) {
      // Get possible transitions from current level
      if (branches.find(current_level) == branches.end()) {
        G4Exception("[Barium133Generator]", "GeneratePrimaryVertex", FatalException, 
                    "No transitions defined for current level!");
      }
      
      std::vector<Transition>& possible_transitions = branches[current_level];
      
      // Calculate total branching ratio from this level
      G4double total_branching = 0.0;
      for (const auto& trans : possible_transitions) {
        total_branching += trans.branching_ratio;
      }
      
      // Select transition based on branching ratios
      G4double trans_random = G4UniformRand() * total_branching;
      G4double cumulative = 0.0;
      
      Transition selected_transition = possible_transitions[0];  // Default to first
      for (const auto& trans : possible_transitions) {
        cumulative += trans.branching_ratio;
        if (trans_random <= cumulative) {
          selected_transition = trans;
          break;
        }
      }
      
      // Generate direction for this gamma ray
      G4ThreeVector momentum_direction;
      
      if (use_hemisphere_emission_) {
        // Generate random direction in hemisphere towards emission_direction
        G4ThreeVector preferred_dir = emission_direction_.unit();
        G4double costheta = G4UniformRand();  // Random between 0 and 1 (hemisphere)
        G4double phi = G4UniformRand() * 2.0 * M_PI;
        
        G4double sintheta = std::sqrt(1.0 - costheta*costheta);
        G4ThreeVector local_dir(sintheta*std::cos(phi), sintheta*std::sin(phi), costheta);
        
        // Create orthogonal coordinate system
        G4ThreeVector u, v;
        if (std::abs(preferred_dir.z()) < 0.9) {
          u = preferred_dir.cross(G4ThreeVector(0, 0, 1)).unit();
        } else {
          u = preferred_dir.cross(G4ThreeVector(1, 0, 0)).unit();
        }
        v = preferred_dir.cross(u).unit();
        
        momentum_direction = local_dir.x()*u + local_dir.y()*v + local_dir.z()*preferred_dir;
      } else {
        // Full isotropic emission
        momentum_direction = G4RandomDirection();
      }

      // Calculate momentum components for the gamma ray
      G4double mass = gamma_particle_->GetPDGMass();
      G4double total_energy = selected_transition.transition_energy + mass;
      G4double momentum_magnitude = std::sqrt(total_energy*total_energy - mass*mass);
      
      G4double px = momentum_magnitude * momentum_direction.x();
      G4double py = momentum_magnitude * momentum_direction.y();
      G4double pz = momentum_magnitude * momentum_direction.z();

      // Create primary gamma particle for this transition
      G4PrimaryParticle* gamma = new G4PrimaryParticle(gamma_particle_, px, py, pz);
      gamma->SetMass(mass);
      gamma->SetCharge(gamma_particle_->GetPDGCharge());
      gamma->SetPolarization(0.0, 0.0, 0.0);

      vertex->SetPrimary(gamma);
      
      // Move to the destination level
      current_level = selected_transition.destination_level;
    }

    evt->AddPrimaryVertex(vertex);

    // Debug output (can be removed in production)
    static G4int event_count = 0;
    if (event_count < 10) {
      G4cout << "[Barium133Generator] Event " << event_count 
             << ": Generated cascade from " << (use_chain1 ? "Chain 1 (Ground State)" : "Chain 2 (Isomeric State)");
      if (use_hemisphere_emission_) {
        G4cout << " (hemisphere toward " << emission_direction_ << ")";
      } else {
        G4cout << " (isotropic)";
      }
      G4cout << G4endl;
    }
    event_count++;
  }

} // end namespace nexus
