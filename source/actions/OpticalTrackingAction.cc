// ----------------------------------------------------------------------------
// nexus | OpticalTrackingAction.cc
//
// This class saves the trajectories of optical photons, in addition to the
// particles saved by the default tracking action. Its purpose is to store
// optical photon information in the output file.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "OpticalTrackingAction.h"

#include "Trajectory.h"
#include "TrajectoryMap.h"
#include "FactoryBase.h"

#include <G4Track.hh>
#include <G4TrackingManager.hh>
#include <G4Trajectory.hh>
#include <G4OpticalPhoton.hh>
#include <G4Step.hh>
#include <G4StepPoint.hh>
#include <G4VProcess.hh>
#include <G4OpBoundaryProcess.hh>
#include <G4ProcessManager.hh>



using namespace nexus;

REGISTER_CLASS(OpticalTrackingAction, G4UserTrackingAction)

// Static member initialization
int OpticalTrackingAction::evt_photons_created_ = 0;
int OpticalTrackingAction::evt_teflon_hits_ = 0;
int OpticalTrackingAction::evt_source_hits_ = 0;

// Constructor: Initialize the photon counter
OpticalTrackingAction::OpticalTrackingAction(): G4UserTrackingAction(), total_photons_(0), sensor_photons_(0), teflon_photons_(0)
, aluminum_photons_(0), fiber_scint_photons_(0), fiber_wls_photons_(0), vacuum_chamber_photons_(0)
{
}

OpticalTrackingAction::~OpticalTrackingAction()
{
  // Print the total number of photons at the end of the simulation
  G4cout << "Total optical photons created: " << total_photons_ << G4endl;
  G4cout << "Total source photon hits: " << sensor_photons_ << G4endl;
  G4cout << "Total teflon photon hits: " << teflon_photons_ << G4endl;
  G4cout << "Total aluminiun photon hits: " << aluminum_photons_ << G4endl;
  G4cout << "Total fiber scintillation photon hits: " << fiber_scint_photons_ << G4endl;
  G4cout << "Total fiber WLS photon hits: " << fiber_wls_photons_ << G4endl;
  G4cout << "Total vacuum chamber photon hits: " << vacuum_chamber_photons_ << G4endl;
}

// Static methods for per-event counting
void OpticalTrackingAction::ResetEventCounters()
{
  evt_photons_created_ = 0;
  evt_teflon_hits_ = 0;
  evt_source_hits_ = 0;
}

void OpticalTrackingAction::PreUserTrackingAction(const G4Track* track)
{
  // Create a new trajectory associated to the track
  G4VTrajectory* trj = new Trajectory(track);

  // Set the trajectory in the tracking manager (guard pointer)
  if (fpTrackingManager) {
    fpTrackingManager->SetStoreTrajectory(true);
    fpTrackingManager->SetTrajectory(trj);
  }

  // Only inspect creator process if this is an optical photon
  if (track && track->GetDefinition() == G4OpticalPhoton::Definition()) {
    const G4VProcess* creator = track->GetCreatorProcess();
    if (creator) {
      // Safe to call GetProcessName() now
      if (creator->GetProcessName() == "Scintillation") {
        total_photons_++;
        evt_photons_created_++;  // Also increment per-event counter
      }
    }
  }
}


void OpticalTrackingAction::PostUserTrackingAction(const G4Track* track)
{
  Trajectory* trj = (Trajectory*) TrajectoryMap::Get(track->GetTrackID());

  // Do nothing if the track has no associated trajectory in the map
  if (!trj) return;

  // Record final time and position of the track
  trj->SetFinalPosition(track->GetPosition());
  trj->SetFinalTime(track->GetGlobalTime());
  trj->SetTrackLength(track->GetTrackLength());
  trj->SetFinalMomentum(track->GetMomentum());

  // Get final volume name - need special handling for optical photons at boundaries
  G4String final_vol_name = "";
  
  // In case of optical photons
  if (track->GetDefinition() == G4OpticalPhoton::Definition()) {
    // For optical photons, we need to carefully determine where they ended up
    // especially for boundary absorptions
    
    const G4Step* step = track->GetStep();
    if (step) {
      G4StepPoint* postPoint = step->GetPostStepPoint();
      
      // Get the boundary process to check absorption status
      static G4OpBoundaryProcess* boundary = nullptr;
      if (!boundary) {
        G4ProcessVector* pv = track->GetDefinition()->GetProcessManager()->GetProcessList();
        for (size_t i = 0; i < pv->size(); i++) {
          if ((*pv)[i]->GetProcessName() == "OpBoundary") {
            boundary = (G4OpBoundaryProcess*)(*pv)[i];
            break;
          }
        }
      }
      
      // Check if at geometry boundary
      if (postPoint->GetStepStatus() == fGeomBoundary && boundary) {
        G4OpBoundaryProcessStatus status = boundary->GetStatus();
        
        if (status == Absorption) {
          // Photon was absorbed at boundary
          // The absorbing volume is the one the photon was trying to enter
          if (track->GetNextVolume()) {
            final_vol_name = track->GetNextVolume()->GetName();
          } else {
            // NextVolume is NULL - this shouldn't happen for Absorption
            // Fall back to checking the touchable
            G4TouchableHandle touch = postPoint->GetTouchableHandle();
            if (touch && touch->GetVolume()) {
              final_vol_name = touch->GetVolume()->GetName();
            } else {
              final_vol_name = track->GetVolume()->GetName();
            }
          }
        } else {
          // Not absorbed at boundary - use standard logic
          if (track->GetNextVolume()) {
            final_vol_name = track->GetNextVolume()->GetName();
          } else {
            final_vol_name = track->GetVolume()->GetName();
          }
        }
      } else {
        // Not at geometry boundary - use standard logic
        if (track->GetNextVolume()) {
          final_vol_name = track->GetNextVolume()->GetName();
        } else {
          final_vol_name = track->GetVolume()->GetName();
        }
      }
    } else {
      // No step available - fallback
      if (track->GetNextVolume()) {
        final_vol_name = track->GetNextVolume()->GetName();
      } else {
        final_vol_name = track->GetVolume()->GetName();
      }
    }
    
    trj->SetFinalVolume(final_vol_name);
  }
  // Final Volume of non optical photons
  else {
    final_vol_name = track->GetVolume()->GetName();
    trj->SetFinalVolume(final_vol_name);
  }

  // Record last process of the track
  G4String final_process = track->GetStep()->GetPostStepPoint()
                                ->GetProcessDefinedStep()->GetProcessName();

  trj->SetFinalProcess(final_process);

  // Count teflon hits - check all teflon volume names
  if ((final_vol_name == "TEFLON1") || (final_vol_name == "TEFLON2") || 
      (final_vol_name == "TEFLON3") || (final_vol_name == "TEFLON4") || 
      (final_vol_name == "TEFLON_FRONT") || (final_vol_name == "TEFLON_BACK") ||
      (final_vol_name.find("TEFLON") != std::string::npos)) {
    // Only count if this photon was created by Scintillation process
    if (trj->GetCreatorProcess() == "Scintillation") {
      teflon_photons_++;
      evt_teflon_hits_++;
    }
  }
  
  // Count source hits - only scintillation photons
  if ((final_vol_name == "SOURCEPLATE1") || (final_vol_name == "SOURCE_PLATE_SENSAREA") ||
      (final_vol_name.find("SOURCE") != std::string::npos)) {
    // Only count if this photon was created by Scintillation process
    if (trj->GetCreatorProcess() == "Scintillation") {
      sensor_photons_++;
      evt_source_hits_++;
    }
  }
  
  // Count aluminum hits
  if (final_vol_name.find("ALUMINUM") != std::string::npos) {
    aluminum_photons_++;
  }

  // Count fiber hits (scintillation photons absorbed in fibers)
  if (final_vol_name.find("FIBER") != std::string::npos && 
      final_vol_name.find("FIBER_SENSOR") == std::string::npos) {
    if (trj->GetCreatorProcess() == "Scintillation" && 
        (final_process == "OpWLS" || final_process == "OpAbsorption")) {
      fiber_scint_photons_++;
    }
    if (trj->GetCreatorProcess() == "OpWLS") {
      fiber_wls_photons_++;
    }
  }

  // Count vacuum chamber hits
  if ((final_vol_name == "VAC_CHAMBER_END_FRONT") || 
      (final_vol_name == "VAC_CHAMBER_END_BACK") || 
      (final_vol_name == "VAC_CHAMBER") ||
      (final_vol_name.find("CHAMBER") != std::string::npos)) {
    vacuum_chamber_photons_++;
  }
}
