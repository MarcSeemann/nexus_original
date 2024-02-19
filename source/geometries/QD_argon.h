// ----------------------------------------------------------------------------
// nexus | QDARGON.h
//

#ifndef QDARGON_H
#define QDARGON_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"
#include "BoxPointSampler.h"

class G4GenericMessenger;

namespace nexus {

  class QD_argon: public GeometryBase {
  public:
    /// Constructor
    QD_argon();

    /// Destructor
    ~QD_argon();

    void Construct();
    G4ThreeVector GenerateVertex(const G4String& region) const;


  private:

    // Messenger for the definition of control commands
    G4GenericMessenger* msg_;
    CylinderPointSampler2020* inside_source_;

    // Dimension of the crystals
    G4double epoxy_layers;

  };
}
#endif
