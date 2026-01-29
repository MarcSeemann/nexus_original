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

    // Static methods for per-event counting
    static void ResetEventCounters();
    
    // Getters for current event counts
    static int GetEventPhotonsCreated() { return evt_photons_created_; }
    static int GetEventTeflonHits() { return evt_teflon_hits_; }
    static int GetEventSourceHits() { return evt_source_hits_; }

  private:
    // Run totals (printed at end of simulation)
    int total_photons_;
    int sensor_photons_;
    int teflon_photons_;
    int aluminum_photons_;
    int fiber_scint_photons_;
    int fiber_wls_photons_;
    int vacuum_chamber_photons_;
    
    // Static per-event counters
    static int evt_photons_created_;
    static int evt_teflon_hits_;
    static int evt_source_hits_;
  };

}

#endif
