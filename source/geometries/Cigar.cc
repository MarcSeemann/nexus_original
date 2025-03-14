// ----------------------------------------------------------------------------
// nexus | Cigar.cc
//
// Box containing optical fibers
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "Cigar.h"

#include "FactoryBase.h"
#include "OpticalMaterialProperties.h"
#include "Visibilities.h"
#include "GenericSquarePhotosensor.h"
#include "GenericCircularPhotosensor.h"
#include <G4GenericMessenger.hh>
#include <G4OpticalSurface.hh>
#include <G4LogicalSkinSurface.hh>
#include <G4LogicalVolume.hh>
#include <G4PVPlacement.hh>
#include <G4NistManager.hh>
#include <G4Box.hh>
#include <G4Tubs.hh>
#include <G4VisAttributes.hh>
#include <G4SubtractionSolid.hh>
#include <G4LogicalBorderSurface.hh>
#include "G4Sphere.hh"
#include "IonizationSD.h"
#include <G4SDManager.hh>
#include "G4ParticleTable.hh"



using namespace nexus;

REGISTER_CLASS(Cigar, GeometryBase)

namespace nexus {

  Cigar::Cigar():
    GeometryBase(),
    cigar_length_ (400 * mm),
    cigar_width_ (30 * mm),
    fiber_diameter_(1 * mm),
    gas_("Ar"),
    pressure_(1. * bar),
    coating_ ("TPB"),
    fiber_type_ ("Y11"),
    coated_(true)
  {
    msg_ = new G4GenericMessenger(this, "/Geometry/Cigar/",
      "Control commands of geometry Cigar.");

    G4GenericMessenger::Command&  width_cmd =
      msg_->DeclareProperty("cigar_width", cigar_width_,
                            "Barrel fiber radius");
    width_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  length_cmd =
      msg_->DeclareProperty("cigar_length", cigar_length_,
                            "Barrel fiber length");
    length_cmd.SetUnitCategory("Length");

    G4GenericMessenger::Command&  fiber_diameter_cmd =
      msg_->DeclareProperty("fiber_diameter", fiber_diameter_,
                            "Fiber diameter");
    fiber_diameter_cmd.SetUnitCategory("Length");

    msg_->DeclareProperty("gas", gas_, "Gas (Ar or Xe)");

    G4GenericMessenger::Command&  pressure_cmd =
      msg_->DeclareProperty("pressure", pressure_,
                            "Vessel pressure");
    pressure_cmd.SetUnitCategory("Pressure");

    msg_->DeclareProperty("coating", coating_, "Fiber coating (TPB or PTH)");
    msg_->DeclareProperty("fiber_type", fiber_type_, "Fiber type (Y11 or B2)");
    msg_->DeclareProperty("coated", coated_, "Coat fibers with WLS coating");

    // Separate Messenger for ParticleName()
    particle_msg_ = new G4GenericMessenger(this, "/Generator/SingleParticle/",
                                           "Commands for single particle generator.");

    // particle_msg_->DeclareMethod("particle", &Cigar::ParticleName, "Set particle to be generated.");


  }



  Cigar::~Cigar()
  {
    delete msg_;
    delete particle_msg_;
  }

  // void Cigar::ParticleName(G4String name)
  // {
  //     auto particle_def = G4ParticleTable::GetParticleTable()->FindParticle(name);
  //     if (particle_def) {
  //         std::cout << "Particle name set to: " << name << std::endl;
  //         // Store the particle definition, e.g., in a class variable if needed.
  //         particle_definition_ = particle_def;
  //     } else {
  //         std::cerr << "Error: Particle " << name << " not found in G4ParticleTable." << std::endl;
  //     }
  // }



  void Cigar::Construct()
  {

    G4cout << "[Cigar] *** Cigar geometry ***" << G4endl;
    G4cout << "[Cigar] Using " << fiber_type_ << " fibers";
    if (coated_)
      G4cout << " with " << coating_ << " coating";
    G4cout << G4endl;
    std::ifstream file("macros/Cigar.init.mac");
    std::string line;
    while (std::getline(file, line)) {
        G4cout << "Macro line: [" << line << "]" << G4endl;
    }
    G4cout << "Cigar gas: [" << gas_ << "]" << G4endl;





    G4RotationMatrix *temp_rot = new G4RotationMatrix();
    temp_rot->rotateY(0 * deg);

    // Choose source position

    double chamber_diameter = 105 * mm;
    G4double panel_width = 2.5 * mm;
    G4double generic_cigar_shift = 3.5*cm;

    // Kr position
    // inside_cigar_ = new BoxPointSampler(cigar_width_/2 + 2.5 * mm, cigar_width_/2 + 2.5 * mm, cigar_length_/2, 0, G4ThreeVector(0.,0.,0-generic_cigar_shift));

    // Generic source in centre of Cigar
    // inside_cigar_ = new BoxPointSampler(1*mm, 1*mm, 1*mm, 0, G4ThreeVector(0.,0.,0.));

    // Alpha source position at end of chamber opposite to SiPMs
    // Inside cigar at the hole
    // inside_cigar_ = new BoxPointSampler(1*mm, 1*mm, 1*mm, 0, G4ThreeVector(0.,0., -cigar_length_/2 + panel_width));
    // Outside cigar at the hole
    // inside_cigar_ = new BoxPointSampler(1*mm, 1*mm, 1*mm, 0, G4ThreeVector(0.,0., -cigar_length_/2 - panel_width - 0.5*mm));
    double source_position_cylinder_x = 0.0;
    double source_position_cylinder_y = 0.0;
    // Outside
    // double source_position_cylinder_z = -cigar_length_/2 - panel_width - 1.5*mm;
    // Inside
    double source_position_cylinder_z = -cigar_length_/2 - panel_width + 4.5*mm;
    // Source placement
    inside_cigar_ = new CylinderPointSampler(7.5*mm/2, 0.1*mm, 0, 0, G4ThreeVector(source_position_cylinder_x,source_position_cylinder_y, source_position_cylinder_z-generic_cigar_shift), temp_rot);

    


    // Inside cigar at the centre
    // inside_cigar_ = new BoxPointSampler(1*mm, 1*mm, 1*mm, 0, G4ThreeVector(0.,0., 0));

    // // Na22 source position
    // G4RotationMatrix *y_rot_180 = new G4RotationMatrix();
    // y_rot_180->rotateY(0 * deg);
    // inside_cigar_ = new BoxPointSampler(1*mm, 1*mm, 1*mm, 0, G4ThreeVector(0, chamber_diameter+3*mm, 0), y_rot_180);

    // Fiber dimensions

    world_z_ = cigar_length_ * 20;
    world_xy_ = cigar_width_ * 20;

    G4Material *this_fiber = materials::PS();
    G4MaterialPropertiesTable *this_fiber_optical = nullptr;
    if (fiber_type_ == "Y11") {
      this_fiber_optical = opticalprops::Y11();
    } else if (fiber_type_ == "B2") {
      this_fiber_optical = opticalprops::B2();
    } else {
      G4Exception("[Cigar]", "Construct()",
                  FatalException, "Invalid fiber type, must be Y11 or B2");
    }

    G4Material *this_coating = nullptr;
    G4MaterialPropertiesTable *this_coating_optical = nullptr;
    if (coated_) {
      if (coating_ == "TPB") {
        this_coating = materials::TPB();
        this_coating_optical = opticalprops::TPB();
      } else if (coating_ == "TPH") {
        this_coating = materials::TPH();
        this_coating_optical = opticalprops::TPH();
      } else {
        G4Exception("[Cigar]", "Construct()",
                    FatalException, "Invalid coating, must be TPB or TPH");
      }
    }

    fiber_ = new GenericWLSFiber(fiber_type_, true, true, fiber_diameter_, cigar_length_ + 7 * cm, true, coated_, this_fiber, this_coating, true);

    // WORLD /////////////////////////////////////////////////

    // G4Material* world_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_AIR");
    // if (gas_ == "Ar") {
    //   world_mat->SetMaterialPropertiesTable(opticalprops::Vacuum());
    // } else if (gas_ == "Xe") {
    //   world_mat->SetMaterialPropertiesTable(opticalprops::Vacuum());

    G4Material* world_mat = nullptr;
    if (gas_ == "Ar") {
      world_mat = materials::GAr(pressure_);
      world_mat->SetMaterialPropertiesTable(opticalprops::GAr(10/keV));
    } else if (gas_ == "Xe") {
      world_mat = materials::GXe(pressure_);
      world_mat->SetMaterialPropertiesTable(opticalprops::GXe(pressure_, 273.15, 10/keV, 1000.*ms));
    } else if (gas_ == "ArXe") {
      world_mat = materials::GXeAr(pressure_, 273.15, 0.01);
      world_mat->SetMaterialPropertiesTable(opticalprops::GArXe(10/keV, 1000.*ms, 1000, pressure_));
    } else if (gas_ == "GTest") {
      world_mat = materials::GXe(pressure_);
      world_mat->SetMaterialPropertiesTable(opticalprops::GTest(pressure_, 273.15, 10/keV, 1000.*ms));
    } else {
      G4cout << "Invalid gas = " << gas_ << G4endl;
      G4Exception("[Cigar]", "Construct()",
            FatalException, "Invalid gas, must be Ar or Xe dsjkdjsk");
    }

    G4Box* world_solid_vol =
     new G4Box("WORLD", world_xy_/2., world_xy_/2., world_z_/2.);

    G4LogicalVolume* world_logic_vol =
      new G4LogicalVolume(world_solid_vol, world_mat, "WORLD");
    world_logic_vol->SetVisAttributes(G4VisAttributes::GetInvisible());
    GeometryBase::SetLogicalVolume(world_logic_vol);


        // // Vacuum chamber

    G4Tubs* vacuum_chamber =
      new G4Tubs("VACUUM_CHAMBER_CYLINDER", chamber_diameter - 0.8*cm, chamber_diameter, cigar_length_*3/4, 0,2*pi);
    G4Material* steel = materials::Steel();

    G4LogicalVolume* vacuum_chamber_logic =
      new G4LogicalVolume(vacuum_chamber, steel, "CHAMBER");


    vacuum_chamber_logic->SetVisAttributes(nexus::DarkGrey());
    // vacuum_chamber_logic->SetVisAttributes(G4VisAttributes::GetInvisible());

    new G4PVPlacement(0, G4ThreeVector(0, 0, 0),
                      vacuum_chamber_logic, "VAC_CHAMBER", world_logic_vol,
                      true, 0, false);
    
    // Create a disk object to close the ends of the cylinder

    G4Tubs* vacuum_chamber_end =
      new G4Tubs("VACUUM_CHAMBER_END", 0, chamber_diameter+5*cm, 2*cm, 0, 2*pi);
    G4LogicalVolume* vacuum_chamber_end_logic =
      new G4LogicalVolume(vacuum_chamber_end, steel, "CHAMBER_END");

    vacuum_chamber_end_logic->SetVisAttributes(nexus::DarkGrey());
    // vacuum_chamber_end_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
  
    new G4PVPlacement(0, G4ThreeVector(0, 0, cigar_length_*3/4+2.0*cm),
                      vacuum_chamber_end_logic, "VAC_CHAMBER_END_FRONT", world_logic_vol,
                      true, 0, false);
    
    new G4PVPlacement(0, G4ThreeVector(0, 0, -cigar_length_*3/4-2.0*cm),
                      vacuum_chamber_end_logic, "VAC_CHAMBER_END_BACK", world_logic_vol,
                      true, 0, false);

    // Optical surface for vacuum chamber
    G4OpticalSurface* opsur_vac_chamber =
      new G4OpticalSurface("VAC_CHAMBER_OPSURF", unified, ground, dielectric_metal);
    opsur_vac_chamber->SetMaterialPropertiesTable(opticalprops::Steel());

    new G4LogicalSkinSurface("VAC_CHAMBER_OPSURF", vacuum_chamber_logic, opsur_vac_chamber);
    new G4LogicalSkinSurface("VAC_CHAMBER_OPSURF", vacuum_chamber_end_logic, opsur_vac_chamber);



    // Create gas material for inside Cigar

    G4Material* cigar_mat = nullptr;
    
    if (gas_ == "Ar") {
      cigar_mat = materials::GAr(pressure_);
      cigar_mat->SetMaterialPropertiesTable(opticalprops::GAr(10/keV));
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GAr(1. / (6 * eV)));
      // https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=1263927
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GAr(19841/MeV));
      std::cout << "Cigar Ar gas pressure: " << pressure_ << " bar" << std::endl;
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GAr(2500000));
    } else if (gas_ == "Xe") {
      cigar_mat = materials::GXe(pressure_);
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GXe(pressure_));
      cigar_mat->SetMaterialPropertiesTable(opticalprops::GXe(pressure_, 273.15, 10/keV, 1000.*ms));
      std::cout << "Cigar Xe gas pressure: " << pressure_ << " bar" << std::endl;
    } else if (gas_ == "ArXe") {
      cigar_mat = materials::GXeAr(pressure_, 273.15, 0.1);
      cigar_mat->SetMaterialPropertiesTable(opticalprops::GArXe(10/keV, 1000.*ms, 1000, pressure_));
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GArXe(19841/MeV));
      std::cout << "Cigar ArXe gas pressure: " << pressure_ << " bar" << std::endl;
      
    } else if (gas_ == "GTest") {
      cigar_mat = materials::GXe(pressure_);
      // cigar_mat->SetMaterialPropertiesTable(opticalprops::GXe(pressure_));
      cigar_mat->SetMaterialPropertiesTable(opticalprops::GTest(pressure_, 273.15, 10/keV, 1000.*ms));
      std::cout << "Cigar Xe gas pressure: " << pressure_ << " bar" << std::endl;
    } else {
      G4Exception("[Cigar]", "Construct()",
            FatalException, "Invalid gas, must be Ar or Xe or ArXe");
    }

    // Create subtraction volume gas for inside vacuum chamber but outside cigar
    G4Tubs* cigar_mat_solid_big =
      new G4Tubs("CigarGasCylinder", 0, chamber_diameter - 0.81*cm, cigar_length_*3/4, 0,2*pi);

    G4Box* cigar_mat_solid_inside = new G4Box("CigarGasBox", cigar_width_ / 2 + 5*mm + panel_width, cigar_width_ / 2 + 5*mm + panel_width, cigar_length_/2+panel_width+3.8*cm);

    // Subtraction volume
    G4SubtractionSolid* cigar_mat_solid_outside = new G4SubtractionSolid("CigarGasSolid", cigar_mat_solid_big, cigar_mat_solid_inside, 0, G4ThreeVector(0, 0, 0));


    // G4Box* cigar_mat_solid = new G4Box("CigarGasBox", chamber_diameter-4*cm, chamber_diameter-4*cm, cigar_length_/2);
    G4LogicalVolume* cigar_mat_outside_logic = new G4LogicalVolume(cigar_mat_solid_outside, cigar_mat, "CigarGasLogic");

    // cigar_mat_outside_logic->SetVisAttributes(nexus::DarkGreen());
    cigar_mat_outside_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    new G4PVPlacement(0, G4ThreeVector(0, 0, 0), cigar_mat_outside_logic, "CigarGas", world_logic_vol, false, 0, true);


    G4LogicalVolume* cigar_mat_inside_logic = new G4LogicalVolume(cigar_mat_solid_inside, cigar_mat, "CigarGasLogic");
    IonizationSD* ionization_sd_gas = new IonizationSD("/Cigar/GasIonInside");
    cigar_mat_inside_logic->SetSensitiveDetector(ionization_sd_gas);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionization_sd_gas);
    cigar_mat_inside_logic->SetVisAttributes(G4VisAttributes::GetInvisible());
    // cigar_mat_inside_logic->SetVisAttributes(nexus::Blue());
    G4VPhysicalVolume *gas_inside = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), cigar_mat_inside_logic, "CigarGas", world_logic_vol, false, 0, true);
    G4cout << "Creating CigarGas volume with sensitive detector: " << ionization_sd_gas->GetName() << G4endl;





    // TEFLON PANELS ///////////////////////////////////////////
    G4double extra_width = (5) * mm;
    // G4Box* teflon_panel_top =
    //   new G4Box("TEFLON_PANEL_TOP", cigar_width_ / 2 + extra_width + panel_width, panel_width / 2, cigar_length_ / 2);
    G4Box* teflon_panel_top =
      new G4Box("TEFLON_PANEL_TOP", cigar_width_ / 2 + extra_width + panel_width, panel_width / 2, cigar_length_ / 2);
    G4Box* teflon_panel_side =
      new G4Box("TEFLON_PANEL_SIDE", cigar_width_ / 2 + extra_width, panel_width / 2, cigar_length_ / 2);
    G4Box* teflon_panel_close =
      new G4Box("TEFLON_PANEL_CLOSE", cigar_width_ / 2 + extra_width+panel_width, cigar_width_ / 2 + extra_width+panel_width, panel_width / 2);
    // Create a subtraction solid in the telfon panel close for the source
    G4Box* teflon_panel_close_source = new G4Box("TEFLON_PANEL_CLOSE_SOURCE", 1*mm, 1*mm, panel_width);
    G4SubtractionSolid* teflon_panel_close_subtracted = new G4SubtractionSolid("TEFLON_PANEL_CLOSE_SUBTRACTED", teflon_panel_close, teflon_panel_close_source, 0, G4ThreeVector(0, 0, 0));

    
    G4Material* teflon = G4NistManager::Instance()->FindOrBuildMaterial("G4_TEFLON");
    teflon->SetMaterialPropertiesTable(opticalprops::PTFE());
    G4LogicalVolume* teflon_logic_top =
      new G4LogicalVolume(teflon_panel_top, teflon, "TEFLON");
    G4LogicalVolume* teflon_logic_side =
      new G4LogicalVolume(teflon_panel_side, teflon, "TEFLON");
    G4LogicalVolume* teflon_logic_close =
      new G4LogicalVolume(teflon_panel_close_subtracted, teflon, "TEFLON_CLOSE");

    IonizationSD* ionization_teflon_close = new IonizationSD("/Cigar/TeflonIonClose");
    teflon_logic_close->SetSensitiveDetector(ionization_teflon_close);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionization_teflon_close);
    teflon_logic_close->SetVisAttributes(nexus::White());
    G4cout << "Creating Teflon volume with sensitive detector: " << ionization_teflon_close->GetName() << G4endl;

    IonizationSD* ionization_teflon_side = new IonizationSD("/Cigar/TeflonIonSide");
    teflon_logic_side->SetSensitiveDetector(ionization_teflon_side);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionization_teflon_side);
    teflon_logic_side->SetVisAttributes(nexus::White());
    G4cout << "Creating Teflon volume with sensitive detector: " << ionization_teflon_side->GetName() << G4endl;


    IonizationSD* ionization_teflon_top = new IonizationSD("/Cigar/TeflonIonTop");
    teflon_logic_top->SetSensitiveDetector(ionization_teflon_top);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionization_teflon_top);
    teflon_logic_top->SetVisAttributes(nexus::White());

    G4cout << "Creating Teflon volume with sensitive detector: " << ionization_teflon_top->GetName() << G4endl;



    G4VPhysicalVolume *teflon_1 = new G4PVPlacement(0, G4ThreeVector(0, cigar_width_/2+panel_width/2+fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4,0-generic_cigar_shift),
                      teflon_logic_top, "TEFLON1", cigar_mat_inside_logic,
                      true, 0, false);

    G4VPhysicalVolume *teflon_2 = new G4PVPlacement(0, G4ThreeVector(0, -cigar_width_/2-panel_width/2-fiber_diameter_-(+extra_width/2+panel_width/2+fiber_diameter_/4), 0-generic_cigar_shift),
                      teflon_logic_top, "TEFLON2", cigar_mat_inside_logic,
                      true, 1, false);

    // new G4PVPlacement(0, G4ThreeVector(0, 0, cigar_length_/2+panel_width/2),
    //                   teflon_logic_close, "TEFLON_FRONT", cigar_mat_logic,
    //                   true, 1, false);

    G4VPhysicalVolume *teflon_back = new G4PVPlacement(0, G4ThreeVector(0, 0, -cigar_length_/2-panel_width/2-1*mm-generic_cigar_shift),
                      teflon_logic_close, "TEFLON_BACK", cigar_mat_inside_logic,
                      true, 1, false);

    G4RotationMatrix *rot_x = new G4RotationMatrix();
    rot_x->rotateZ(90 * deg);
    G4VPhysicalVolume *teflon_3 = new G4PVPlacement(G4Transform3D(*rot_x, G4ThreeVector(cigar_width_ / 2 + panel_width / 2 + fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4, 0, 0-generic_cigar_shift)),
                      teflon_logic_side, "TEFLON3", cigar_mat_inside_logic,
                      true, 1, false);

    G4VPhysicalVolume *teflon_4 = new G4PVPlacement(G4Transform3D(*rot_x, G4ThreeVector(-cigar_width_ / 2 - panel_width / 2 - fiber_diameter_-extra_width/2-panel_width/2-fiber_diameter_/4, 0, 0-generic_cigar_shift)),
                      teflon_logic_side, "TEFLON4", cigar_mat_inside_logic,
                      true, 1, false);









    // // TPB coating teflon ///////////////////////////////////////

    // G4Box* TPB_coating_top =
    //   new G4Box("TPB_COATING_TOP", extra_width/2, 3*micrometer, (cigar_length_ / 2) - panel_width);
    // G4Box* TPB_coating_rot =
    //   new G4Box("TPB_COATING_ROT", extra_width/2 - 3*micrometer, 3*micrometer, (cigar_length_ / 2) - panel_width);
    
    // G4Material* tpb = materials::TPB();
    // tpb->SetMaterialPropertiesTable(opticalprops::TPB());


    // G4LogicalVolume* tpb_logic_top =
    //   new G4LogicalVolume(TPB_coating_top, tpb, "TPB_TOP");
    // tpb_logic_top->SetVisAttributes(nexus::Blue());

    // G4LogicalVolume* tpb_logic_rot =
    //   new G4LogicalVolume(TPB_coating_rot, tpb, "TPB_ROT");
    // tpb_logic_rot->SetVisAttributes(nexus::Blue());


    // G4VPhysicalVolume *tpb_left_1 = new G4PVPlacement(0, G4ThreeVector(15*mm + extra_width/2, cigar_width_/2+panel_width/2+fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4 -panel_width/2-3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_top, "TPB_TEFLON1_LEFT", cigar_mat_inside_logic,
    //                   true, 0, false);
    
    // G4VPhysicalVolume *tpb_right_1 = new G4PVPlacement(0, G4ThreeVector(-(15*mm + extra_width/2), cigar_width_/2+panel_width/2+fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4 -panel_width/2-3*micrometer, 0-generic_cigar_shift),
    //               tpb_logic_top, "TPB_TEFLON1_RIGHT", cigar_mat_inside_logic,
    //               true, 0, false);



    // G4VPhysicalVolume *tpb_left_2 = new G4PVPlacement(0, G4ThreeVector(15*mm + extra_width/2, -cigar_width_/2-panel_width/2-fiber_diameter_-(+extra_width/2+panel_width/2+fiber_diameter_/4) +panel_width/2+3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_top, "TPB_TEFLON2_LEFT", cigar_mat_inside_logic,
    //                   true, 0, false);
    
    // G4VPhysicalVolume *tpb_right_2 = new G4PVPlacement(0, G4ThreeVector(-(15*mm + extra_width/2), -cigar_width_/2-panel_width/2-fiber_diameter_-(+extra_width/2+panel_width/2+fiber_diameter_/4) +panel_width/2+3*micrometer, 0-generic_cigar_shift),
    //               tpb_logic_top, "TPB_TEFLON2_RIGHT", cigar_mat_inside_logic,
    //               true, 0, false);


    // G4RotationMatrix* rotZ90 = new G4RotationMatrix();
    // rotZ90->rotateZ(90. * deg);


    // // Rotated placements (90 degrees around Z-axis)
    // G4VPhysicalVolume *tpb_left_3 = new G4PVPlacement(rotZ90, G4ThreeVector(cigar_width_/2+panel_width/2+fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4 -panel_width/2-3*micrometer, 15*mm + extra_width/2-3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_rot, "TPB_TEFLON3_LEFT", cigar_mat_inside_logic,
    //                   true, 1, false);

    // G4VPhysicalVolume *tpb_right_3 = new G4PVPlacement(rotZ90, G4ThreeVector(-cigar_width_/2-panel_width/2-fiber_diameter_-extra_width/2-panel_width/2-fiber_diameter_/4 +panel_width/2+3*micrometer, 15*mm + extra_width/2-3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_rot, "TPB_TEFLON3_RIGHT", cigar_mat_inside_logic,
    //                   true, 1, false);

    // G4VPhysicalVolume *tpb_left_4 = new G4PVPlacement(rotZ90, G4ThreeVector(cigar_width_/2+panel_width/2+fiber_diameter_+extra_width/2+panel_width/2+fiber_diameter_/4 -panel_width/2-3*micrometer, -15*mm - extra_width/2+3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_rot, "TPB_TEFLON4_LEFT", cigar_mat_inside_logic,
    //                   true, 1, false);

    // G4VPhysicalVolume *tpb_right_4 = new G4PVPlacement(rotZ90, G4ThreeVector(-cigar_width_/2-panel_width/2-fiber_diameter_-extra_width/2-panel_width/2-fiber_diameter_/4 +panel_width/2+3*micrometer, -15*mm - extra_width/2+3*micrometer, 0-generic_cigar_shift),
    //                   tpb_logic_rot, "TPB_TEFLON4_RIGHT", cigar_mat_inside_logic,
    //                   true, 1, false);
    








    // // FIBER ////////////////////////////////////////////////////
    
    fiber_->SetCoreOpticalProperties(this_fiber_optical);
    fiber_->SetCoatingOpticalProperties(this_coating_optical);

    fiber_->Construct();
    G4LogicalVolume *fiber_logic = fiber_->GetLogicalVolume();
    if (fiber_type_ == "Y11") {
        fiber_logic->SetVisAttributes(nexus::LightGreenAlpha());
    } else if (fiber_type_ == "B2") {
        fiber_logic->SetVisAttributes(nexus::LightBlueAlpha());
    }

    G4int n_fibers = floor(cigar_width_ / fiber_diameter_);
    G4cout << "[Cigar] Box with " << n_fibers << " fibers each side" << G4endl;

    // // DETECTOR /////////////////////////////////////////////
    G4double photosensor_thickness = 0.4 * mm;
    GenericSquarePhotosensor *sipm  = new GenericSquarePhotosensor("FIBER_SENSOR", fiber_diameter_, n_fibers * fiber_diameter_, photosensor_thickness);
    G4int sipm_entries = 24;
    G4double sipm_energy[] = {
      h_Planck * c_light / (866.20 * nm), h_Planck * c_light / (808.45 * nm),
      h_Planck * c_light / (766.20 * nm), h_Planck * c_light / (721.13 * nm),
      h_Planck * c_light / (685.92 * nm), h_Planck * c_light / (647.89 * nm),
      h_Planck * c_light / (623.94 * nm), h_Planck * c_light / (597.18 * nm),
      h_Planck * c_light / (573.24 * nm), h_Planck * c_light / (545.07 * nm),
      h_Planck * c_light / (518.31 * nm), h_Planck * c_light / (502.82 * nm),
      h_Planck * c_light / (454.93 * nm), h_Planck * c_light / (421.13 * nm),
      h_Planck * c_light / (395.77 * nm), h_Planck * c_light / (378.87 * nm),
      h_Planck * c_light / (367.61 * nm), h_Planck * c_light / (359.15 * nm),
      h_Planck * c_light / (349.30 * nm), h_Planck * c_light / (340.85 * nm),
      h_Planck * c_light / (336.62 * nm), h_Planck * c_light / (332.39 * nm),
      h_Planck * c_light / (326.76 * nm), h_Planck * c_light / (319.72 * nm)
    };

    G4double sipm_efficiency[] = {
      5.75, 9.38,
      12.48, 16.37,
      20.18, 24.34,
      28.67, 33.01,
      37.26, 41.50,
      45.93, 48.32,
      50.00, 47.61,
      43.54, 39.03,
      34.25, 29.91,
      25.40, 20.62,
      16.02, 11.50,
      6.81, 3.36
    };
    for (G4int i=0; i < sipm_entries; i++) {
      sipm_efficiency[i] /= 100;
    }
    G4double energy[]       = {opticalprops::optPhotMinE_, opticalprops::optPhotMaxE_};
    G4double reflectivity[] = {0.0     , 0.0     };
    G4double efficiency[]   = {1.      , 1.      };
    G4MaterialPropertiesTable* photosensor_mpt = new G4MaterialPropertiesTable();
    photosensor_mpt->AddProperty("REFLECTIVITY", energy, reflectivity, 2);
    photosensor_mpt->AddProperty("EFFICIENCY",   sipm_energy, sipm_efficiency, sipm_entries);
    sipm->SetOpticalProperties(photosensor_mpt);
    sipm->SetVisibility(true);
    sipm->SetSensorDepth(1);
    sipm->SetTimeBinning(50 * ns);
    sipm->SetWindowRefractiveIndex(opticalprops::OptCoupler()->GetProperty("RINDEX"));
    sipm->Construct();
    G4LogicalVolume* sipm_logic = sipm->GetLogicalVolume();
    // // ALUMINIZED ENDCAP

    G4Material *fiber_end_mat = G4NistManager::Instance()->FindOrBuildMaterial("G4_Al");

    G4double fiber_end_z = 350 * nm;

    G4Tubs *fiber_end_solid_vol =
        new G4Tubs("fiber_end", 0, fiber_diameter_ / 2, fiber_end_z, 0, 2 * M_PI);

    G4LogicalVolume *fiber_end_logic_vol =
        new G4LogicalVolume(fiber_end_solid_vol, fiber_end_mat, "FIBER_END");
    G4OpticalSurface *opsur_al =
        new G4OpticalSurface("AL_OPSURF", unified, ground, dielectric_metal);

    opsur_al->SetPolish(0.75);
    opsur_al->SetMaterialPropertiesTable(opticalprops::PolishedAl());

    new G4LogicalSkinSurface("AL_OPSURF", fiber_end_logic_vol, opsur_al);

    // // PLACEMENT /////////////////////////////////////////////
    G4RotationMatrix *rot_y = new G4RotationMatrix();
    G4RotationMatrix *rot_z = new G4RotationMatrix();
    rot_y->rotateY(180 * deg);
    rot_z->rotateZ(90 * deg);
    new G4PVPlacement(G4Transform3D(*rot_y, G4ThreeVector(cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4, 0, cigar_length_ / 2 + photosensor_thickness / 2 + 7 * cm-generic_cigar_shift)),
                      sipm_logic, "SIPM1", cigar_mat_inside_logic,
                      true, 1, false);

    new G4PVPlacement(G4Transform3D(*rot_y, G4ThreeVector(- cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), 0, cigar_length_ / 2 + photosensor_thickness / 2 + 7 * cm-generic_cigar_shift)),
                      sipm_logic, "SIPM2", cigar_mat_inside_logic,
                      true, 2, false);
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, - cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), cigar_length_ / 2 + photosensor_thickness / 2 + 7 * cm-generic_cigar_shift)),
                      sipm_logic, "SIPM3", cigar_mat_inside_logic,
                      true, 3, false);
    new G4PVPlacement(G4Transform3D(*rot_z, G4ThreeVector(0, cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4, cigar_length_ / 2 + photosensor_thickness / 2 + 7 * cm-generic_cigar_shift)),
                      sipm_logic, "SIPM4", cigar_mat_inside_logic,
                      true, 4, false);

    // Teflon special closing panel
    G4Box* teflon_closing_panel =
      new G4Box("TEFLON_PANEL_FRONT_TEMP", cigar_width_ / 2 + extra_width+panel_width, cigar_width_ / 2 + extra_width+panel_width, panel_width / 2);
    // G4LogicalVolume* teflon_logic_front =
    //   new G4LogicalVolume(teflon_closing_panel, teflon, "TEFLON");
    // teflon_logic_front->SetVisAttributes(nexus::White());
    G4VSolid* temp_solid = teflon_closing_panel;
    G4double fiber_radius = fiber_diameter_/1.1;
    G4double fiber_length = fiber_->GetLength();
    G4VSolid* fiber_solid = new G4Tubs("FIBER_SOLID", 0, fiber_radius / 2, fiber_length / 2, 0, 2 * pi);
    G4SubtractionSolid* subtracted_solid = nullptr;

    IonizationSD* ionization_sd_fibers = new IonizationSD("/Cigar/FiberIonization");
    fiber_logic->SetSensitiveDetector(ionization_sd_fibers);
    G4SDManager::GetSDMpointer()->AddNewDetector(ionization_sd_fibers);


    for (G4int ifiber = 0; ifiber < n_fibers; ifiber++)
    {

      std::string label = std::to_string(ifiber);

      G4PVPlacement* fiber_placement1 = new G4PVPlacement(0, G4ThreeVector(cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4, -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, 3.5 * cm-generic_cigar_shift),
                                                          fiber_logic, "FIBER1-" + label, cigar_mat_inside_logic,
                                                          true, ifiber, false);

      G4PVPlacement* fiber_placement2 = new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2,  3.5 * cm-generic_cigar_shift),
                                                          fiber_logic, "FIBER2-" + label, cigar_mat_inside_logic,
                                                          true, ifiber * 2, false);

      G4PVPlacement* fiber_placement3 = new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, -cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4),  3.5 * cm-generic_cigar_shift),
                                                          fiber_logic, "FIBER3-" + label, cigar_mat_inside_logic,
                                                          true, ifiber * 3, false);

      G4PVPlacement* fiber_placement4 = new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, cigar_width_ / 2 + fiber_diameter_ / 2+(extra_width/2+panel_width/2+fiber_diameter_/4),  3.5 * cm-generic_cigar_shift),
                                                          fiber_logic, "FIBER4-" + label, cigar_mat_inside_logic,
                                                          true, ifiber * 4, false);

      new G4PVPlacement(0, G4ThreeVector(cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4, -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, -cigar_length_ / 2 - fiber_end_z-generic_cigar_shift),
                        fiber_end_logic_vol, "ALUMINUM1-" + label, cigar_mat_inside_logic,
                        true, ifiber, false);

      new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, -cigar_length_ / 2 - fiber_end_z-generic_cigar_shift),
                        fiber_end_logic_vol, "ALUMINUM2-" + label, cigar_mat_inside_logic,
                        true, ifiber, false);

      new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, -cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), -cigar_length_ / 2 - fiber_end_z-generic_cigar_shift),
                        fiber_end_logic_vol, "ALUMINUM3-" + label, cigar_mat_inside_logic,
                        true, ifiber, false);

      new G4PVPlacement(0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, cigar_width_ / 2 + fiber_diameter_ / 2+(extra_width/2+panel_width/2+fiber_diameter_/4), -cigar_length_ / 2 - fiber_end_z-generic_cigar_shift),
                        fiber_end_logic_vol, "ALUMINUM4-" + label, cigar_mat_inside_logic,
                        true, ifiber, false);

      // Create a cylinder solid to represent the fiber

      subtracted_solid = new G4SubtractionSolid("SUBTRACTED_SOLID", temp_solid, fiber_solid, rot_z, G4ThreeVector(cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4, -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, 3.5 * cm));
      temp_solid = subtracted_solid;
      subtracted_solid = new G4SubtractionSolid("SUBTRACTED_SOLID", temp_solid, fiber_solid, rot_z, G4ThreeVector(-cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4), -cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2,  3.5 * cm));
      temp_solid = subtracted_solid;
      subtracted_solid = new G4SubtractionSolid("SUBTRACTED_SOLID", temp_solid, fiber_solid, 0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, -cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4),  3.5 * cm));
      temp_solid = subtracted_solid;
      subtracted_solid = new G4SubtractionSolid("SUBTRACTED_SOLID", temp_solid, fiber_solid, 0, G4ThreeVector(-cigar_width_ / 2 + ifiber * fiber_diameter_ + fiber_diameter_ / 2, cigar_width_ / 2 + fiber_diameter_ / 2+(extra_width/2+panel_width/2+fiber_diameter_/4),  3.5 * cm));
      temp_solid = subtracted_solid;
    }



    // Simpler Teflon Close

    G4VSolid* teflon_closing_panel_temp =
      new G4Box("TEFLON_PANEL_FRONT_TEMP", cigar_width_ / 2 + extra_width+panel_width, cigar_width_ / 2 + extra_width+panel_width, panel_width / 2);
    G4VSolid* fiber_space =
      new G4Box("TEFLON_PANEL_FRONT_TEMP", fiber_diameter_+0.03*cm, cigar_width_ / 2, panel_width / 2);
    G4SubtractionSolid* subtracted_solid_test = new G4SubtractionSolid("SUBTRACTED_SOLID", teflon_closing_panel_temp, fiber_space, rot_z, G4ThreeVector(0,cigar_width_ / 2 + fiber_diameter_ / 2+extra_width/2+panel_width/2+fiber_diameter_/4,0));
    subtracted_solid_test = new G4SubtractionSolid("SUBTRACTED_SOLID", subtracted_solid_test, fiber_space, rot_z, G4ThreeVector(0,-cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4),0));
    rot_y->rotateY(90 * deg);
    subtracted_solid_test = new G4SubtractionSolid("SUBTRACTED_SOLID", subtracted_solid_test, fiber_space, rot_y, G4ThreeVector(-cigar_width_ / 2 - fiber_diameter_ / 2-(extra_width/2+panel_width/2+fiber_diameter_/4),0,0));
    subtracted_solid_test = new G4SubtractionSolid("SUBTRACTED_SOLID", subtracted_solid_test, fiber_space, rot_y, G4ThreeVector(+cigar_width_ / 2 + fiber_diameter_ / 2+(extra_width/2+panel_width/2+fiber_diameter_/4),0,0));
    
    G4LogicalVolume* teflon_closing_panel_logic_temp = new G4LogicalVolume(subtracted_solid_test, teflon, "TEFLON_PANEL");

    


    teflon_closing_panel_logic_temp->SetVisAttributes(nexus::White());
    rot_x->rotateZ(90 * deg);
    G4VPhysicalVolume *teflon_front = new G4PVPlacement(rot_x, G4ThreeVector(0, 0, cigar_length_/2+panel_width/2 -generic_cigar_shift), teflon_closing_panel_logic_temp, "TEFLON_FRONT", cigar_mat_inside_logic, true, 1, false);


    // Optical surface on teflon
    G4OpticalSurface* opsur_teflon =
      new G4OpticalSurface("TEFLON_OPSURF", unified, ground, dielectric_metal, .01);
    opsur_teflon->SetMaterialPropertiesTable(opticalprops::PTFE());

    // Teflon skin surface

    new G4LogicalSkinSurface("TEFLON_OPSURF", teflon_logic_top, opsur_teflon);
    new G4LogicalSkinSurface("TEFLON_OPSURF", teflon_logic_side, opsur_teflon);
    new G4LogicalSkinSurface("TEFLON_OPSURF", teflon_logic_close, opsur_teflon);


        // Optical surface between gas and TPB 

    G4OpticalSurface* opsur_teflon_tpb =
        new G4OpticalSurface("TEFLON_TPB", unified, groundteflonair, dielectric_metal, .01);
    opsur_teflon_tpb->SetMaterialPropertiesTable(opticalprops::TPB());

    // // TPB skin surface
    // // Check github for NEXT-100 to understand why TPB uses logical bounderay surface and not skin surface

    // new G4LogicalSkinSurface("TEFLON_TPB", tpb_logic_top, opsur_teflon_tpb);
    // new G4LogicalSkinSurface("TEFLON_TPB", tpb_logic_rot, opsur_teflon_tpb);

    




    // // Na22 position check

    // G4Box* na_position =
    //   new G4Box("NA_SOURCE", 1*cm, 1*mm, 1*cm);
    // G4Material* steel = materials::Steel();

    // G4LogicalVolume* na_position_logic =
    //   new G4LogicalVolume(na_position, steel, "NA_SOURCE_LOGIC");

    // na_position_logic->SetVisAttributes(nexus::Blue());
    
    // new G4PVPlacement(0, G4ThreeVector(0, chamber_diameter+3*mm, 0),
    //                   na_position_logic, "NA_SOURCE_LOGIC", cigar_mat_logic,
    //                   true, 0, false);

    // Cylindrical metal plate behind the source

    // // DETECTOR /////////////////////////////////////////////
    GenericCircularPhotosensor *source_plate  = new GenericCircularPhotosensor("SOURCE_PLATE", 12.5*mm, 0.5*mm);
    G4int source_plate_entries = 2;
    G4double source_plate_energy[] = {
      h_Planck * c_light / (1000 * nm), h_Planck * c_light / (90 * nm)
    };

    G4double source_plate_efficiency[] = {
      100, 100
    };
    for (G4int i=0; i < source_plate_entries; i++) {
      source_plate_efficiency[i] /= 100;
    }
    G4double energy_plate[]       = {opticalprops::optPhotMinE_, opticalprops::optPhotMaxE_};
    G4double reflectivity_plate[] = {0.0     , 0.0     };
    G4double efficiency_plate[]   = {1.      , 1.      };
    G4MaterialPropertiesTable* photosensor_mpt_plate = new G4MaterialPropertiesTable();
    photosensor_mpt_plate->AddProperty("REFLECTIVITY", energy_plate, reflectivity_plate, 2);
    photosensor_mpt_plate->AddProperty("EFFICIENCY",   source_plate_energy, source_plate_efficiency, source_plate_entries);
    source_plate->SetOpticalProperties(photosensor_mpt_plate);
    source_plate->SetVisibility(true);
    source_plate->SetSensorDepth(1);
    source_plate->SetTimeBinning(50 * ns);
    source_plate->SetWindowRefractiveIndex(opticalprops::OptCoupler()->GetProperty("RINDEX"));
    source_plate->Construct();
    G4LogicalVolume* source_plate_logic = source_plate->GetLogicalVolume();

    new G4PVPlacement(0, G4ThreeVector(source_position_cylinder_x, source_position_cylinder_y, source_position_cylinder_z-0.251*mm -generic_cigar_shift),
                      source_plate_logic, "SOURCEPLATE1", cigar_mat_inside_logic,
                      false, 5, false);

        // Optical surface for vacuum chamber
    G4OpticalSurface* opsur_source =
      new G4OpticalSurface("SOURCE_OPSURF", unified, ground, dielectric_metal);
    opsur_source->SetMaterialPropertiesTable(opticalprops::Steel());

    new G4LogicalSkinSurface("SOURCE_OPSURF", source_plate_logic, opsur_source);



    // G4Tubs* source_plate =
    //   new G4Tubs("SOURCE_PLATE", 0, 12.5*mm, 0.5*mm, 0, 2*pi);
    // G4LogicalVolume* source_plate_logic =
    //   new G4LogicalVolume(source_plate, steel, "SOURCE_PLATE_LOGIC");
    // source_plate_logic->SetVisAttributes(nexus::BloodRedAlpha());
    // IonizationSD* ionization_sd_source_plate = new IonizationSD("/Cigar/PlateSource");
    // source_plate_logic->SetSensitiveDetector(ionization_sd_source_plate);
    // G4SDManager::GetSDMpointer()->AddNewDetector(ionization_sd_source_plate);

    // new G4PVPlacement(0, G4ThreeVector(source_position_cylinder_x, source_position_cylinder_y, source_position_cylinder_z-0.5*mm), source_plate_logic, "PlateSource", cigar_mat_inside_logic, false, 0, true);





  }


  G4ThreeVector Cigar::GenerateVertex(const G4String& region) const
  {
    G4ThreeVector vertex(0., 0., 0.);

    // WORLD
    if (region == "INSIDE_CIGAR") {
        return inside_cigar_->GenerateVertex("INSIDE");
    }
    else {
      G4Exception("[Cigar]", "GenerateVertex()", FatalException,
                  "Unknown vertex generation region!");
    }
    return vertex;
  }


} // end namespace nexus