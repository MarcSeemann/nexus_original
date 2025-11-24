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

      // ========================================================================
      // OLD CODE - Filter based on total energy deposit across all volumes
      // ========================================================================
      // // Get the trajectories stored for this event and loop through them
      // // to calculate the total energy deposit and check for primary muons
      //
      // G4double edep = 0.;
      // G4bool hasPrimaryMuon = false;
      //
      // G4TrajectoryContainer* tc = event->GetTrajectoryContainer();
      // if (tc) {
      //   // in interactive mode, a G4TrajectoryContainer would exist
      //   // but the trajectories will not cast to Trajectory
      //   Trajectory* trj = dynamic_cast<Trajectory*>((*tc)[0]);
      //   if (trj == nullptr){
      //     G4Exception("[DefaultEventAction]", "EndOfEventAction()", FatalException,
      //                 "DefaultTrackingAction is required when using DefaultEventAction");
      //   }
      //   for (unsigned int i=0; i<tc->size(); ++i) {
      //     Trajectory* tr = dynamic_cast<Trajectory*>((*tc)[i]);
      //     edep += tr->GetEnergyDeposit();
      //     
      //     // Check if this is a primary muon (parent ID == 0 and is muon)
      //     if (tr->GetParentID() == 0) {
      //       G4String particleName = tr->GetParticleName();
      //       if (particleName == "mu+" || particleName == "mu-") {
      //         hasPrimaryMuon = true;
      //       }
      //     }
      //   }
      // }
      // else {
      //   G4Exception("[DefaultEventAction]", "EndOfEventAction()", FatalException,
      //               "DefaultTrackingAction is required when using DefaultEventAction");
      // }
      //
      // PersistencyManager* pm = dynamic_cast<PersistencyManager*>
      //   (G4VPersistencyManager::GetPersistencyManager());
      //
      // if (!event->IsAborted() && edep>0) {
      // 	pm->InteractingEvent(true);
      // } else {
      // 	pm->InteractingEvent(false);
      // }
      // 
      // // CRITICAL: Always store events with primary muons, even if they deposit no energy!
      // G4bool shouldStore = false;
      // if (hasPrimaryMuon) {
      //   shouldStore = true;
      // } else if (!event->IsAborted() && edep > energy_min_ && edep < energy_max_) {
      //   shouldStore = true;
      // }
      // 
      // if (shouldStore) {
      // 	pm->StoreCurrentEvent(true);
      // } else {
      // 	pm->StoreCurrentEvent(false);
      // }
      // ========================================================================


      // ========================================================================
      // NEW CODE - Filter based on hits in /Cigar/GasIonInside ONLY
      // ========================================================================
      // Check if there are hits specifically in /Cigar/GasIonInside
      G4HCofThisEvent* hce = event->GetHCofThisEvent();
      G4SDManager* sdmgr = G4SDManager::GetSDMpointer();
      
      G4bool hasGasHits = false;
      G4double gasEdep = 0.;

      // Cache the collection ID to avoid repeated lookups and warnings
      static G4int cached_hcid = -1;
      
      if (hce) {
        // Only call GetCollectionID once (on first event)
        if (cached_hcid == -1) {
          cached_hcid = sdmgr->GetCollectionID("/Cigar/GasIonInside/IonizationHitsCollection");
        }
        
        if (cached_hcid >= 0) {
          G4VHitsCollection* hc = hce->GetHC(cached_hcid);
          IonizationHitsCollection* hits = dynamic_cast<IonizationHitsCollection*>(hc);
          
          if (hits && hits->entries() > 0) {
            hasGasHits = true;
            // Sum energy deposited in the gas volume
            for (size_t i=0; i<hits->entries(); i++) {
              IonizationHit* hit = dynamic_cast<IonizationHit*>(hits->GetHit(i));
              if (hit) {
                gasEdep += hit->GetEnergyDeposit();
              }
            }
          }
        }
      }

      PersistencyManager* pm = dynamic_cast<PersistencyManager*>
        (G4VPersistencyManager::GetPersistencyManager());

      // Mark if event interacted in the gas volume
      if (!event->IsAborted() && hasGasHits && gasEdep > 0) {
        pm->InteractingEvent(true);
      } else {
        pm->InteractingEvent(false);
      }
      
      // Only store events that have hits in the gas volume with energy above threshold
      G4bool shouldStore = false;
      if (!event->IsAborted() && hasGasHits && gasEdep > energy_min_ && gasEdep < energy_max_) {
        shouldStore = true;
      }
      
      if (shouldStore) {
        pm->StoreCurrentEvent(true);
        // G4cout << "[DefaultEventAction] Storing event " << nevt_-1 
        //        << " with " << gasEdep/keV << " keV deposited in gas volume" << G4endl;
      } else {
        pm->StoreCurrentEvent(false);
      }
      // ========================================================================

    }
  }


} // end namespace nexus
