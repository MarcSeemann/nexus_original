// ----------------------------------------------------------------------------
// nexus | ArgonXenonProperties.h
//
// Functions to calculate relevant relevant physical properties of xenon.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#ifndef ARXE_PROPERTIES_H
#define ARXE_PROPERTIES_H

#include <globals.hh>
#include <vector>

class G4MaterialPropertiesTable;

G4double GArXeDensity(G4double pressure);
G4double ArXeMassPerMole(G4int a);

/// Return the refractive index of ArXe gas for a given photon energy
G4double ArXeRefractiveIndex(G4double energy, G4double density);

G4double GArXeScintillation(G4double energy, G4double pressure);

G4double GetGasDensity(G4double pressure, G4double temperature);

/// Electroluminescence yield of ArXe gas
G4double ArXeELLightYield(G4double field_strength, G4double pressure);

#endif
