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



using namespace nexus;

REGISTER_CLASS(OpticalTrackingAction, G4UserTrackingAction)

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

void OpticalTrackingAction::PreUserTrackingAction(const G4Track* track)
{
  // Create a new trajectory associated to the track
  G4VTrajectory* trj = new Trajectory(track);

  // Set the trajectory in the tracking manager
  fpTrackingManager->SetStoreTrajectory(true);
  fpTrackingManager->SetTrajectory(trj);

  // Check if the track is an optical photon and increment the counter if so
  if ((track->GetDefinition() == G4OpticalPhoton::Definition()) && (track->GetCreatorProcess()->GetProcessName() == "Scintillation")) {
    total_photons_++;
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

  // In case of optical photons
  if (track->GetDefinition() == G4OpticalPhoton::Definition()) {
    // If optical-photon has no NextVolume (escaping from the world)
    // Assign current volume as the decay one
    if (track->GetNextVolume()) trj->SetFinalVolume(track->GetNextVolume()->GetName());
    else                        trj->SetFinalVolume(track->GetVolume()->GetName());
  }
  // Final Volume of non optical photons
  else trj->SetFinalVolume(track->GetVolume()->GetName());

  // Record last process of the track
  G4String final_process = track->GetStep()->GetPostStepPoint()
                                ->GetProcessDefinedStep()->GetProcessName();

  trj->SetFinalProcess(final_process);

  // ['TEFLON4' 'TEFLON3' 'TEFLON2' 'TEFLON1' 'TEFLON_FRONT' 'TEFLON_BACK']
  if ((trj->GetFinalVolume() == "TEFLON1") || (trj->GetFinalVolume() == "TEFLON2") || (trj->GetFinalVolume() == "TEFLON3")
  || (trj->GetFinalVolume() == "TEFLON4") || (trj->GetFinalVolume() == "TEFLON_FRONT") || (trj->GetFinalVolume() == "TEFLON_BACK")) {
    teflon_photons_++;
  }
  if ((trj->GetFinalVolume() == "SOURCEPLATE1") || (trj->GetFinalVolume() == "SOURCE_PLATE_SENSAREA")) {
    sensor_photons_++;
  }
  // Thirty aluminum plates per panel and four panels
  for (int panel = 1; panel <= 4; ++panel) {
    for (int i = 0; i < 30; ++i) {
      std::string volumeName = "ALUMINUM" + std::to_string(panel) + "-" + std::to_string(i);
      if (trj->GetFinalVolume() == volumeName) {
        aluminum_photons_++;
        break; // Exit the loop once a match is found
      }
    }
  }

  // Thirty fibers per panel and four panels
  for (int panel = 1; panel <= 4; ++panel) {
    for (int i = 0; i < 30; ++i) {
      std::string volumeName = "FIBER" + std::to_string(panel) + "-" + std::to_string(i);
      if ((trj->GetFinalVolume() == volumeName) && (trj->GetCreatorProcess() == "Scintillation") && ((trj->GetFinalProcess() == "OpWLS") || (trj->GetFinalProcess() == "OpAbsorption"))) {
        fiber_scint_photons_++;
        break; // Exit the loop once a match is found
      }
    }
  }

  // Thirty fibers per panel and four panels
  for (int panel = 1; panel <= 4; ++panel) {
    for (int i = 0; i < 30; ++i) {
      std::string volumeName = "FIBER" + std::to_string(panel) + "-" + std::to_string(i);
      if ((trj->GetFinalVolume() == volumeName) && (trj->GetCreatorProcess() == "OpWLS")) {
        fiber_wls_photons_++;
        break; // Exit the loop once a match is found
      }
    }
  }

  if ((trj->GetFinalVolume() == "VAC_CHAMBER_END_FRONT") || (trj->GetFinalVolume() == "VAC_CHAMBER_END_BACK") || (trj->GetFinalVolume() == "VAC_CHAMBER")) {
    vacuum_chamber_photons_++;
  }



}
