// ----------------------------------------------------------------------------
// nexus | DefaultEventAction.cc
//
// This is the default event action of the NEXT simulations. Only events with
// deposited energy larger than 0 are saved in the nexus output file.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "DefaultEventAction.h"
#include "Trajectory.h"
#include "PersistencyManager.h"
#include "IonizationHit.h"
#include "FactoryBase.h"

#include <G4Event.hh>
#include <G4VVisManager.hh>
#include <G4Trajectory.hh>
#include <G4GenericMessenger.hh>
#include <G4HCofThisEvent.hh>
#include <G4SDManager.hh>
#include <G4HCtable.hh>
#include <G4MuonPlus.hh>
#include <G4MuonMinus.hh>
#include <globals.hh>
#include <G4SystemOfUnits.hh>
#include <set>


namespace nexus {

REGISTER_CLASS(DefaultEventAction, G4UserEventAction)

  DefaultEventAction::DefaultEventAction():
    G4UserEventAction(), nevt_(0), nupdate_(10), energy_min_(0.), energy_max_(DBL_MAX)
  {
    msg_ = new G4GenericMessenger(this, "/Actions/DefaultEventAction/");

    G4GenericMessenger::Command& thresh_cmd =
       msg_->DeclareProperty("min_energy", energy_min_,
                             "Minimum deposited energy to save the event to file.");
    thresh_cmd.SetParameterName("min_energy", true);
    thresh_cmd.SetUnitCategory("Energy");
    thresh_cmd.SetRange("min_energy>0.");

    G4GenericMessenger::Command& max_energy_cmd =
      msg_->DeclareProperty("max_energy", energy_max_,
                            "Maximum deposited energy to save the event to file.");
    max_energy_cmd.SetParameterName("max_energy", true);
    max_energy_cmd.SetUnitCategory("Energy");
    max_energy_cmd.SetRange("max_energy>0.");

    PersistencyManager* pm = dynamic_cast<PersistencyManager*>
      (G4VPersistencyManager::GetPersistencyManager());

    pm->SaveNumbOfInteractingEvents(true);
    pm->OnlyStorePrimaryGammas(true);  // Enable filtering to save only primary gammas
  }



  DefaultEventAction::~DefaultEventAction()
  {
  }



  void DefaultEventAction::BeginOfEventAction(const G4Event* /*event*/)
  {
    // Print out event number info
    if ((nevt_ % nupdate_) == 0) {
      G4cout << " >> Event no. " << nevt_  << G4endl;
      if (nevt_  == (10 * nupdate_)) nupdate_ *= 10;
    }
  }



  void DefaultEventAction::EndOfEventAction(const G4Event* event)
  {
    nevt_++;

    // Determine whether total energy deposit in ionization sensitive
    // detectors is above threshold
    if (energy_min_ >= 0.) {

      PersistencyManager* pm = dynamic_cast<PersistencyManager*>
        (G4VPersistencyManager::GetPersistencyManager());

      // Enable trajectory filtering to only save primary gammas
      pm->OnlyStorePrimaryGammas(true);

      G4bool shouldStore = false;

      // Check if event has primary gamma with interaction in gas volume
      if (!event->IsAborted()) {
        // First, find primary gammas
        G4bool hasPrimaryGamma = false;
        std::set<G4int> primary_gamma_ids;
        
        G4TrajectoryContainer* tc = event->GetTrajectoryContainer();
        if (tc) {
          for (unsigned int i=0; i<tc->size(); ++i) {
            Trajectory* trj = dynamic_cast<Trajectory*>((*tc)[i]);
            if (trj && trj->GetParentID() == 0 && trj->GetParticleName() == "gamma") {
              hasPrimaryGamma = true;
              primary_gamma_ids.insert(trj->GetTrackID());
            }
          }
        }

        // If we have primary gammas, check if they created hits in gas volume
        if (hasPrimaryGamma) {
          G4HCofThisEvent* hce = event->GetHCofThisEvent();
          G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
          
          if (hce) {
            // Get the gas ionization hits collection
            static G4int cached_hcid = -1;
            if (cached_hcid == -1) {
              cached_hcid = sdmgr->GetCollectionID("GasIonInside/IonizationHitsCollection");
            }
            
            if (cached_hcid >= 0) {
              G4VHitsCollection* hc = hce->GetHC(cached_hcid);
              IonizationHitsCollection* hits = dynamic_cast<IonizationHitsCollection*>(hc);
              
              if (hits && hits->entries() > 0) {
                // Check if any hits came from a primary gamma
                for (size_t i=0; i<hits->entries(); i++) {
                  IonizationHit* hit = dynamic_cast<IonizationHit*>(hits->GetHit(i));
                  if (hit && primary_gamma_ids.find(hit->GetTrackID()) != primary_gamma_ids.end()) {
                    shouldStore = true;
                    break;
                  }
                }
              }
            }
          }
        }
      }

      // Store or reject the event
      if (shouldStore) {
        pm->StoreCurrentEvent(true);
        pm->InteractingEvent(true);
      } else {
        pm->StoreCurrentEvent(false);
        pm->InteractingEvent(false);
      }
    }
  }


} // end namespace nexus
