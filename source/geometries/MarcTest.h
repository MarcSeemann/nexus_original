// ----------------------------------------------------------------------------
// nexus | MarcTest.h
//

#ifndef MARCTEST_H
#define MARCTEST_H

#include "GeometryBase.h"
#include "CylinderPointSampler2020.h"
#include "BoxPointSampler.h"

class G4GenericMessenger;

namespace nexus {

  class MarcTest: public GeometryBase {
  public:
    /// Constructor
    MarcTest();

    /// Destructor
    ~MarcTest();

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
