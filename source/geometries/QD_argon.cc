// ----------------------------------------------------------------------------
// nexus | QD_argon.cc
//
// This class describes a pair of QD_argon crystals with a radioactive source in the middle
//
// ----------------------------------------------------------------------------

#include "QD_argon.h"
#include "MaterialsList.h"
#include "Visibilities.h"
#include "OpticalMaterialProperties.h"
#include "SiPM33.h"
#include "SiPM66.h"
#include "GenericWLSFiber.h"

#include <G4Tubs.hh>
#include <G4SubtractionSolid.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalBorderSurface.hh>

#include <G4Box.hh>
#include <G4NistManager.hh>
#include <G4Material.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4VisAttributes.hh>
#include <G4GenericMessenger.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>

#include "FactoryBase.h"


#include <CLHEP/Units/SystemOfUnits.h>

using namespace nexus;

REGISTER_CLASS(QD_argon, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  QD_argon::QD_argon():
    GeometryBase(),
    epoxy_layers (1)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/QD_argon/",
                                  "Control commands of geometry QD_argon.");
    msg_->DeclareProperty("epoxy_layers", epoxy_layers, "Crystal width");
  }

  QD_argon::~QD_argon()
  {
    delete msg_;
  }


  void QD_argon::Construct()
  {
    G4RotationMatrix *rot_z = new G4RotationMatrix();
      rot_z->rotateX(90 * deg);
    G4RotationMatrix *temp_rot = new G4RotationMatrix();
      temp_rot->rotateY(90 * deg);
    G4double LED_cyl_y = 6 *cm;
    // Create source
    inside_source_ = new CylinderPointSampler2020(0, 0.5*mm / 2,  2.5 * mm / 2,  0, twopi, temp_rot, G4ThreeVector(0, LED_cyl_y, 0));


    G4double size = 0.5 * m;
    // Create lab environment
    G4Box* lab_solid =
      new G4Box("LAB", size/2., size/2., size/2.);
    G4Material* Ar = materials::GAr();
    Ar->SetMaterialPropertiesTable(opticalprops::GAr(15000*0.72, 1.0));
    G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid, Ar, "LAB");
    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    // Define all objects inside the lab
    // Source
    // G4Tubs* source = new G4Tubs("SOURCE", 0, 1*cm, 2 * cm, 0, 2*pi );
    // G4LogicalVolume* source_logic =
    //   new G4LogicalVolume(source,
    //                       Ar,
    //                       "SOURCE");
    // source_logic->SetVisAttributes(nexus::Lilla());

    // new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, LED_cyl_y, 0)),
    //               source_logic, "SOURCE", lab_logic,
    //               true, 1, true);
    // Teflon panel
    // G4Tubs* absorber = new G4Tubs("ABSORBER", 0, 1*cm, 0.5 * mm /2, 0, 2*pi );
    // G4Material* lead = G4NistManager::Instance()->FindOrBuildMaterial("G4_Pb");
    // G4LogicalVolume* absorber_logic = 
    //   new G4LogicalVolume(absorber,lead,"ABSORBER");
    // absorber_logic->SetVisAttributes(nexus::White());
    // new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(5*cm, 0, 5*cm)),
    //           absorber_logic, "ABSORBER1", lab_logic,
    //           true, 1, true);
    // new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(-5*cm, 0, -5*cm)),
    //       absorber_logic, "ABSORBER2", lab_logic,
    //       true, 1, true);
    
    SiPM66 *sipm_geom = new SiPM66();
    sipm_geom->Construct();
    G4LogicalVolume* sipm_logic = sipm_geom->GetLogicalVolume();
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 0, 0)), sipm_logic,
                      "SIPM66", lab_logic, true, 0);


  }

  G4ThreeVector QD_argon::GenerateVertex(const G4String& region) const
  {
    return inside_source_->GenerateVertex("VOLUME");
    // return box_source_->GenerateVertex("INSIDE");
  }

}
