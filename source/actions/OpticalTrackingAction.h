#ifndef OPTICAL_TRACKING_ACTION_H
#define OPTICAL_TRACKING_ACTION_H

#include <G4UserTrackingAction.hh>

class G4Track;

namespace nexus {

  // Optical-checking user tracking action

  class OpticalTrackingAction: public G4UserTrackingAction
  {
  public:
    /// Constructor
    OpticalTrackingAction();
    /// Destructor
    virtual ~OpticalTrackingAction();

    virtual void PreUserTrackingAction(const G4Track*);
    virtual void PostUserTrackingAction(const G4Track*);

  private:
    int total_photons_;  // Variable to store the total number of optical photons
  };

}

#endif
