// ----------------------------------------------------------------------------
// nexus | SiPM66.h
//
// Geometry of a Hamamatsu 6x6 mm2 SiPM.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef SILICON_PM_66_H
#define SILICON_PM_66_H

#include "GeometryBase.h"
#include <G4ThreeVector.hh>

class G4GenericMessenger;

namespace nexus {


  /// Geometry of the Hamamatsu surface-mounted 6x6 mm2 MPPC (SiPM)

  class SiPM66: public GeometryBase
  {
  public:
    /// Constructor
    SiPM66();
    /// Destructor
    ~SiPM66();

    /// Return dimensions of the SiPM
    G4ThreeVector GetDimensions() const;

    /// Invoke this method to build the volumes of the geometry
    void Construct();

  private:
    G4ThreeVector dimensions_; ///< external dimensions of the SiPM66

    // Visibility of the tracking plane
    G4bool visibility_;

     // Messenger for the definition of control commands
    G4GenericMessenger* msg_;

  };


} // end namespace nexus

#endif
