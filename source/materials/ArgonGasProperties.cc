// ----------------------------------------------------------------------------
// nexus | ArgonGasProperties.cc
//
// Relevant physical properties of gaseous argon.
//
// The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "ArgonGasProperties.h"

#include <G4AnalyticalPolSolver.hh>
#include <G4MaterialPropertiesTable.hh>

#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include <stdexcept>


namespace nexus {

  using namespace CLHEP;


  G4double ArgonDensity(G4double pressure)
  {
    //These values are for a temperature of 300 K
    // taken from http://www.nist.gov/srd/upload/jpcrd363.pdf
    // def mol_dm3_to_kg_m3(concentration_mol_dm3, molar_mass):
    //     """
    //     Convert concentration from mol/dm^3 to kg/m^3.

    //     Parameters:
    //         concentration_mol_dm3 (float): Concentration in mol/dm^3.
    //         molar_mass (float): Molar mass of the substance in g/mol.

    //     Returns:
    //         float: Concentration in kg/m^3.
    //     """
    //     # Convert mol/dm^3 to mol/m^3 (1 dm^3 = 0.001 m^3)
    //     concentration_mol_m3 = concentration_mol_dm3 * 1000

    //     # Convert mol/m^3 to kg/m^3 (1 g = 0.001 kg)
    //     concentration_kg_m3 = concentration_mol_m3 * (molar_mass / 1000)

    //     return concentration_kg_m3


    // # Example usage:
    // if __name__ == "__main__":
    //     # Input values
    //     concentration_mol_dm3 = 0.32226  # Example concentration in mol/dm^3
    //     molar_mass = 39.948  # Molar mass of argon in g/mol

    //     # Perform the conversion
    //     concentration_kg_m3 = mol_dm3_to_kg_m3(concentration_mol_dm3, molar_mass)

    //     print(f"{concentration_mol_dm3} mol/dm^3 is equivalent to {concentration_kg_m3:.4f} kg/m^3.")

    G4double density = 1.60279*kg/m3;

    if (pressure/bar > 0.9 && pressure/bar < 1.1)
      density = 1.60279*kg/m3;
    else if (pressure/bar > 1.1 && pressure/bar < 1.75)
      density = 2.4045*kg/m3;
    else if (pressure/bar > 1.75 && pressure/bar < 2.25)
      density = 3.2070*kg/m3;
    else if (pressure/bar > 2.25 && pressure/bar < 2.75)
      density = 4.0100*kg/m3;
    else if (pressure/bar > 2.75 && pressure/bar < 3.25)
      density = 4.8133*kg/m3;
    else if (pressure/bar > 3.25 && pressure/bar < 4.5)
      density = 6.4216*kg/m3;
    else if (pressure/bar > 4.5 && pressure/bar < 5.5)
      density = 8.0319*kg/m3;
    else if (pressure/bar > 5.5 && pressure/bar < 7.0)
      density = 9.6438*kg/m3;
    else if (pressure/bar > 7.0 && pressure/bar < 9.0)
      density = 12.8736*kg/m3;
    else if (pressure/bar > 9.0 && pressure/bar < 12.5)
      density = 16.1118*kg/m3;
    else if (pressure/bar > 14.9 && pressure/bar < 15.1)
      density = 24.2369 *kg/m3;
    else if (pressure/bar > 19.9 && pressure/bar < 20.1)
      density = 32.4066*kg/m3;
    else if (pressure/bar > 29.9 && pressure/bar < 30.1)
      density = 48.8708*kg/m3;
    else if (pressure/bar > 39.9 && pressure/bar < 40.1)
      density = 65.494*kg/m3;
    else
      G4Exception("[ArgonProperties]", "ArgonDensity()", FatalException,
                  "Unknown argon density for this pressure!");

    return density;
  }


  G4double ArgonELLightYield(G4double field_strength, G4double pressure)
  {
    // Empirical formula taken from
    // C.M.B. Monteiro thesis https://estudogeral.sib.uc.pt/bitstream/10316/14635/3/Tese_Doutoramento_Cristina%20Monteiro.pdf.

    // Y/x = (a E/p + b) p,
    // where Y/x is the number of photons per unit lenght (cm),
    // E is the electric field strength, p is the pressure, and a and b
    // are empirically determined constants:
    const G4double a = 81. / kilovolt;
    const G4double b = 47. / (bar*cm);

    G4double yield = (a * field_strength/pressure - b) * pressure;
    if (yield < 0.) yield = 0.;

    return yield;
  }


} // end namespace nexus
