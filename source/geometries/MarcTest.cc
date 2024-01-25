// ----------------------------------------------------------------------------
// nexus | MarcTest.cc
//
// This class describes a pair of MarcTest crystals with a radioactive source in the middle
//
// ----------------------------------------------------------------------------

#include "MarcTest.h"
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

REGISTER_CLASS(MarcTest, GeometryBase)

namespace nexus {

  using namespace CLHEP;

  MarcTest::MarcTest():
    GeometryBase(),
    epoxy_layers (1)
  {
    /// Messenger
    msg_ = new G4GenericMessenger(this, "/Geometry/MarcTest/",
                                  "Control commands of geometry MarcTest.");
    msg_->DeclareProperty("epoxy_layers", epoxy_layers, "Crystal width");
  }

  MarcTest::~MarcTest()
  {
    delete msg_;
  }


  void MarcTest::Construct()
  {

    // Create source
    inside_source_ = new CylinderPointSampler2020(0, 0.5*mm / 2,  2.5 * mm / 2,  0, twopi, nullptr, G4ThreeVector(0, 0, 0));

    // Define constants
    G4int n_fibers = 2;
    G4double fiber_rad = 2. *mm;
    G4double fiber_length = 20. *cm;

    G4double fiber_z_pos = 5. *cm;
    G4double fiber_x_pos = -fiber_rad*n_fibers/2;
    G4double fiber_y_pos = 10. *mm;

    G4double size = 0.5 * m;
    // Create lab environment
    G4Box* lab_solid =
      new G4Box("LAB", size/2., size/2., size/2.);
    G4Material* air = G4NistManager::Instance()->FindOrBuildMaterial("G4_Air");
    G4LogicalVolume* lab_logic = new G4LogicalVolume(lab_solid, air, "LAB");
    lab_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    // Set this volume as the wrapper for the whole geometry
    // (i.e., this is the volume that will be placed in the world)
    this->SetLogicalVolume(lab_logic);

    // Define all objects inside the lab
    // Source
    G4Tubs* source = new G4Tubs("SOURCE", 0, 5.*mm /2, 0.5 * mm /2, 0, 2*pi );
    G4LogicalVolume* source_logic =
      new G4LogicalVolume(source,
                          G4NistManager::Instance()->FindOrBuildMaterial("G4_POLYCARBONATE"),
                          "SOURCE");
    source_logic->SetVisAttributes(nexus::Lilla());
    G4RotationMatrix *rot_z = new G4RotationMatrix();
      rot_z->rotateX(90 * deg);
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 10*cm, 0)),
                  source_logic, "SOURCE", lab_logic,
                  true, 1, true);
    // Teflon panel
    G4Box* teflon_panel = new G4Box("TEFLONPANEL", 5. * cm, 5. *cm, 0.5 * cm);
    G4LogicalVolume* teflon_panel_logic = 
      new G4LogicalVolume(teflon_panel,G4NistManager::Instance()->FindOrBuildMaterial("G4_PTFE"),"TEFLONPANEL");
    teflon_panel_logic->SetVisAttributes(nexus::White());
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, 0, 0)),
              teflon_panel_logic, "TEFLONPANEL", lab_logic,
              true, 1, true);
    // Fibers
    G4Material *fiber_material = nullptr;
    G4MaterialPropertiesTable *fiber_material_optics = nullptr;
    G4Material *coating_material = nullptr;
    G4MaterialPropertiesTable *coating_material_optics = nullptr;

    fiber_material = materials::PS();
    fiber_material_optics = opticalprops::Y11();
    coating_material = materials::TPB();
    coating_material_optics = opticalprops::TPB();

    GenericWLSFiber* fiber = new GenericWLSFiber("FIBER", true, true, fiber_rad, fiber_length, true, true, fiber_material, coating_material, true);
    fiber->SetCoreOpticalProperties(fiber_material_optics);
    fiber->SetCoatingOpticalProperties(coating_material_optics);
    fiber->Construct();
    G4LogicalVolume* fiber_logic = fiber->GetLogicalVolume();
    fiber_logic->SetVisAttributes(nexus::DarkGreen);

    // Epoxy layers
    G4double epoxy_z = 5. *mm;
    G4VSolid* epoxy_outside = new G4Box("EPOXYOUT", 25. *mm, 5. *mm, epoxy_z);
    G4VSolid* epoxy_inside = new G4Tubs("EPOXYIN", 0, fiber_rad/2, epoxy_z, 0, 2*pi);



    std::vector<G4ThreeVector> fib_epox_positions;
    for (G4int i=0; i < n_fibers; i++) {

        G4double x_f = fiber_x_pos + fiber_rad * i*1.5;

        std::string label = std::to_string(i);

        G4ThreeVector position(x_f, 0.0, 0.0);
        fib_epox_positions.push_back(position);

        new G4PVPlacement(0, G4ThreeVector(x_f, fiber_y_pos, fiber_z_pos),fiber_logic, fiber_logic->GetName() + "_" + label, lab_logic,true, 0, true);


       }
    
    G4VSolid* epoxy_union = new G4SubtractionSolid("EPOXY", epoxy_outside, epoxy_inside, 0, fib_epox_positions[0]);
    // Perform additional subtractions using a loop for fiber holes
    for (const auto& position : fib_epox_positions) {
        epoxy_union = new G4SubtractionSolid("EPOXYLAYER", epoxy_union, epoxy_inside, 0, position);
    }

    G4LogicalVolume* epoxy_logic = 
      new G4LogicalVolume(epoxy_union,G4NistManager::Instance()->FindOrBuildMaterial("G4_EPOXY"),"EPOXYLAYER");
    epoxy_logic->SetVisAttributes(nexus::YellowAlpha);
    
    new G4PVPlacement(0, G4ThreeVector(0, fiber_y_pos, 40*mm),epoxy_logic, epoxy_logic->GetName() + "_" + "0", lab_logic,true, 0, true);
  
    // Create optical coupler

    G4Box *optical_coupler = new G4Box("OPT_COUPLER", 3.*mm , 3.*mm , 0.1 / 2 * mm);
    G4Material * opt_coupler_material = materials::OpticalSilicone();
    opt_coupler_material->SetMaterialPropertiesTable(opticalprops::OptCoupler());

    G4LogicalVolume* optical_coupler_logic =
    new G4LogicalVolume(optical_coupler,
                        opt_coupler_material,
                        "OPT_COUPLER");




    SiPM66 *sipm_geom = new SiPM66();
    sipm_geom->Construct();
    G4LogicalVolume* sipm_logic = sipm_geom->GetLogicalVolume();
    new G4PVPlacement(0, G4ThreeVector(0,fiber_y_pos-1.5*mm,fiber_length-fiber_z_pos+2*mm), sipm_logic,
                      "SIPM66", lab_logic, true, 0);
  
    new G4PVPlacement(0, G4ThreeVector(0,fiber_y_pos-1.5*mm,fiber_length-fiber_z_pos+2*mm), optical_coupler_logic,
                  "OPT_COUPLER1", lab_logic, false, 0);
    

  }

  G4ThreeVector MarcTest::GenerateVertex(const G4String& region) const
  {
    return inside_source_->GenerateVertex("VOLUME");
    // return box_source_->GenerateVertex("INSIDE");
  }

}
