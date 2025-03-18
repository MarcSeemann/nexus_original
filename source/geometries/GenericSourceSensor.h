// -----------------------------------------------------------------------------
//  nexus | GenericSquarePhotosensor.h
//
//  Geometry of a configurable box-shaped photosensor.
//
//  The NEXT Collaboration
// -----------------------------------------------------------------------------

#ifndef GENERIC_SOURCESENSOR_H
#define GENERIC_SOURCESENSOR_H

#include "GeometryBase.h"
#include <G4MaterialPropertyVector.hh>

class G4Material;
class G4GenericMessenger;
class G4MaterialPropertiesTable;

namespace nexus {

  class GenericSourceSensor: public GeometryBase
  {
  public:
    // Constructor for a circular sensor providing
    // The default thickness corresponds to a typical value for
    // a silicon photomultiplier.
    GenericSourceSensor(G4String name,   G4double radius, G4double thickness = 2.0 * mm);

    // Constructor for a circular sensor
    GenericSourceSensor(G4String name, G4double size);

    // Destructor
    ~GenericSourceSensor();

    //
    void Construct();

    //
    G4double GetRadius()       const;
    G4double GetThickness()   const;
    const G4String& GetName() const;

    void SetVisibility           (G4bool visibility);
    void SetOpticalProperties    (G4MaterialPropertiesTable* mpt);
    void SetTimeBinning          (G4double time_binning);
    void SetSensorDepth          (G4int sensor_depth);
    void SetMotherDepth          (G4int mother_depth);
    void SetNamingOrder          (G4int naming_order);

  private:

    void ComputeDimensions();
    void DefineMaterials();

    G4String name_;

    G4double radius_, thickness_;
    G4double sensarea_thickness_;
    G4double reduced_radius_;

    G4Material* case_mat_;
    G4Material* window_mat_;
    G4Material* sensitive_mat_;

    G4MaterialPropertiesTable* sensitive_mpt_;

    G4int    sensor_depth_;
    G4int    mother_depth_;
    G4int    naming_order_;
    G4double time_binning_;

    G4bool visibility_;
  };


  inline G4double GenericSourceSensor::GetRadius()       const { return radius_; }
  inline G4double GenericSourceSensor::GetThickness()   const { return thickness_; }
  inline const G4String& GenericSourceSensor::GetName() const { return name_; }

  inline void GenericSourceSensor::SetVisibility(G4bool visibility)
  { visibility_ = visibility; }

  inline void GenericSourceSensor::SetOpticalProperties(G4MaterialPropertiesTable* mpt)
  { sensitive_mpt_ = mpt; }

  inline void GenericSourceSensor::SetTimeBinning(G4double time_binning)
  { time_binning_ = time_binning; }

  inline void GenericSourceSensor::SetSensorDepth(G4int sensor_depth)
  { sensor_depth_ = sensor_depth; }

  inline void GenericSourceSensor::SetMotherDepth(G4int mother_depth)
  { mother_depth_ = mother_depth; }

  inline void GenericSourceSensor::SetNamingOrder(G4int naming_order)
  { naming_order_ = naming_order; }


} // namespace nexus

#endif
