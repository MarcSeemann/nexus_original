// -----------------------------------------------------------------------------
//  nexus | GenericSourceSensor.cc
//
//  Geometry of a configurable box-shaped photosensor.
//
//  The NEXT Collaboration
// -----------------------------------------------------------------------------

#include "GenericSourceSensor.h"

#include "MaterialsList.h"
#include "SensorSD.h"
#include "OpticalMaterialProperties.h"
#include "Visibilities.h"

#include <G4Box.hh>
#include <G4Tubs.hh>

#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4Material.hh>
#include <G4NistManager.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4SDManager.hh>
#include <G4GenericMessenger.hh>

using namespace nexus;


GenericSourceSensor::GenericSourceSensor(G4String name,
                                       G4double radius,
                                       G4double thickness):
  GeometryBase        (),
  name_               (name),          // Width of the Sensitive Area
  radius_             (radius),          // Height of the Sensitive Area
  thickness_          (thickness),       // Thickness of the whole sensor
  sensarea_thickness_ (0.1 * mm),        // Sensitive thickness (similar to Sensl SiPMs)
  sensitive_mpt_      (nullptr),
  sensor_depth_       (-1),
  mother_depth_       (0),
  naming_order_       (0),
  time_binning_       (1.0 * us),
  visibility_         (false)
{
}


GenericSourceSensor::GenericSourceSensor(G4String name, G4double size):
  GenericSourceSensor(name, size, size)
{
}


GenericSourceSensor::~GenericSourceSensor()
{
}


void GenericSourceSensor::ComputeDimensions()
{
  // Reduced size for components inside the case except the WLS
  reduced_radius_  = radius_  - 1. * micrometer;

  // Check that components (window + sensitive + wls) fits into the case
  if ((sensarea_thickness_) > thickness_) {
    G4Exception("[GenericSourceSensor]", "ComputeDimensions()", FatalException,
                ("Sensor size too small. Required thickness >= " +
                 std::to_string(thickness_) +
                 " mm").data());
  }
}


void GenericSourceSensor::DefineMaterials()
{

  // Sensitive /////

  sensitive_mat_ = materials::Steel();
  sensitive_mat_->SetMaterialPropertiesTable(opticalprops::Steel());

}


void GenericSourceSensor::Construct()
{
  ComputeDimensions();

  DefineMaterials();

  // PHOTOSENSITIVE AREA /////////////////////////////////////////////
  G4String name = name_ + "_SENSAREA";

  G4Tubs* sensarea_solid_vol = new G4Tubs(name, 0, reduced_radius_ / 2., thickness_ / 2., 0, twopi);

  G4LogicalVolume* sensarea_logic_vol =
    new G4LogicalVolume(sensarea_solid_vol, sensitive_mat_, name);

  G4double sensarea_zpos = thickness_/2.;

  // Assuming you want to place the sensitive area directly in the world volume or another appropriate volume
  new G4PVPlacement(nullptr, G4ThreeVector(0., 0., sensarea_zpos), sensarea_logic_vol,
                    name, nullptr, false, 0, false);

  GeometryBase::SetLogicalVolume(sensarea_logic_vol);



  // VISIBILITIES /////////////////////////////////////////////
  if (visibility_) {
    sensarea_logic_vol->SetVisAttributes(nexus::DarkGrey());
  }
  else {
    sensarea_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());
  }


  // SENSOR OPTICAL PROPERTIES ////////////////////////////////////////
  if (!sensitive_mpt_)
    G4Exception("[GenericSourceSensor]", "Construct()", FatalException,
                "Sensor Optical Properties must be set before constructing");
  G4OpticalSurface* sensitive_opsurf =
    new G4OpticalSurface(name + "_optSurf", unified, polished, dielectric_metal);
  sensitive_opsurf->SetMaterialPropertiesTable(sensitive_mpt_);
  new G4LogicalSkinSurface(name + "_optSurf", sensarea_logic_vol, sensitive_opsurf);


  // SENSITIVE DETECTOR //////////////////////////////////////////////
  G4String sdname = "/GENERIC_PHOTOSENSOR/" + name_;
  G4SDManager* sdmgr = G4SDManager::GetSDMpointer();

  if (!sdmgr->FindSensitiveDetector(sdname, false)) {
    SensorSD* sensdet = new SensorSD(sdname);
    if (sensor_depth_ == -1)
      G4Exception("[GenericSourceSensor]", "Construct()", FatalException,
                  "Sensor Depth must be set before constructing");

    if ((naming_order_ > 0) & (mother_depth_ == 0))
      G4Exception("[GenericSourceSensor]", "Construct()", FatalException,
                  "naming_order set without setting mother_depth");

    sensdet->SetDetectorVolumeDepth(sensor_depth_);
    sensdet->SetMotherVolumeDepth  (mother_depth_);
    sensdet->SetDetectorNamingOrder(naming_order_);
    sensdet->SetTimeBinning        (time_binning_);

    G4SDManager::GetSDMpointer()->AddNewDetector(sensdet);
    sensarea_logic_vol->SetSensitiveDetector(sensdet);
  }
}
