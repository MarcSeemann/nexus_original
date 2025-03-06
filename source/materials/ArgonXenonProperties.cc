// ----------------------------------------------------------------------------
//  nexus | ArgonXenonProperties.cc
//
//  Functions to calculate relevant physical properties of xenon.
//
//  The NEXT Collaboration
// ----------------------------------------------------------------------------

#include "ArgonXenonProperties.h"
#include "ArgonGasProperties.h"
#include "XenonProperties.h"
#include "Interpolation.h"

#include <G4SystemOfUnits.hh>
#include <G4PhysicalConstants.hh>
#include <G4AnalyticalPolSolver.hh>
#include <G4MaterialPropertiesTable.hh>

using namespace nexus;


G4double GArXeDensity(G4double pressure)
{
  // Computes Xe (gas) density at T = 293 K
  // Values are taken from the reference file nexus/data/gxe_density_table.txt
  // (which, in turn, is downloaded from https://webbook.nist.gov/chemistry/fluid).
  // We assume a linear interpolation between any pair of values in the database.

  G4double density;

  const G4int n_pressures = 6;
  G4double data[n_pressures][2] = {{  1.0 * bar,   5.419 * kg/m3},
                                   {  5.0 * bar,  27.721 * kg/m3},
                                   { 10.0 * bar,  57.160 * kg/m3},
                                   { 13.5 * bar,  78.949 * kg/m3},
                                   { 20.0 * bar, 122.510 * kg/m3},
                                   { 30.0 * bar, 199.920 * kg/m3}};
  G4bool found = false;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "Incorrect pressure method " << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;

  for (G4int i=0; i<n_pressures-1; ++i) {
    if  (pressure >= data[i][0] && pressure < data[i+1][0]) {
      G4double x1 = data[i][0];
      G4double x2 = data[i+1][0];
      G4double y1 = data[i][1];
      G4double y2 = data[i+1][1];
      density = y1 + (y2-y1)*(pressure-x1)/(x2-x1);
      found = true;
      break;
    }
  }

  if (!found) {
    if (pressure == data[n_pressures-1][0]) {
      density = data[n_pressures-1][1];
    }
    else {
      throw "Unknown xenon density for this pressure!";
    }
  }

  return density;
}



G4double ArXeMassPerMole(G4int a)
{
  // Isotopic mass per mole taken from
  // http://rushim.ru/books/spravochniki/handbook-chemistry-and-physics.pdf

  G4double mass_per_mole_xenon = 135.907220*g/mole;
  G4double mass_per_mole_argon = 39.948*g/mole;
  G4int ppm = 1000;
  G4double mass_per_mole = 40*g/mole;

  // https://www.ausetute.com.au/massmole.html
  // Set if statement to always be true for now
  if (true){
    G4double mass_per_mole = (1 - (ppm/10^6)) * mass_per_mole_argon + (ppm/10^6) * mass_per_mole_xenon;
  }
//   G4double mass_per_mole = (1 - (ppm/10^6)) * mass_per_mole_argon + (ppm/10^6) * mass_per_mole_xenon;


//   if (a == 124) {
//     mass_per_mole = 123.9058958*g/mole;
//   } else if (a == 126) {
//     mass_per_mole = 125.904269*g/mole;
//   } else if (a == 128) {
//     mass_per_mole = 127.9035304*g/mole;
//   } else if (a == 129) {
//     mass_per_mole = 128.9047795*g/mole;
//   } else if (a == 130) {
//     mass_per_mole = 129.9035079*g/mole;
//   } else if (a == 131) {
//     mass_per_mole = 130.9050819*g/mole;
//   } else if (a == 132) {
//     mass_per_mole = 131.9041545*g/mole;
//   } else if (a == 134) {
//     mass_per_mole = 133.9053945*g/mole;
//   } else if (a == 136) {
//     mass_per_mole = 135.907220*g/mole;
//   }
  else {
    G4Exception("[XenonGaseousProperties]", "MassPerMole()", FatalException,
    "Unknown mass per mole for this isotope!");
  }

  return mass_per_mole;
}



G4double ArXeRefractiveIndex(G4double energy, G4double density)
{
  // Formula for the refractive index taken from
  // A. Baldini et al., "Liquid Xe scintillation calorimetry
  // and Xe optical properties", arXiv:physics/0401072v1 [physics.ins-det]

  // The Lorentz-Lorenz equation (also known as Clausius-Mossotti equation)
  // relates the refractive index of a fluid with its density:
  // (n^2 - 1) / (n^2 + 2) = - A · d_M,     (1)
  // where n is the refractive index, d_M is the molar density and
  // A is the first refractivity viral coefficient:
  // A(E) = \sum_i^3 P_i / (E^2 - E_i^2),   (2)
  // with:
  G4double P[3] = {71.23, 77.75, 1384.89}; // [eV^3 cm3 / mole]
  G4double E[3] = {8.4, 8.81, 13.2};       // [eV]

  // Note.- Equation (1) has, actually, a sign difference with respect
  // to the one appearing in the reference. Otherwise, it yields values
  // for the refractive index below 1.

  // Let's calculate the virial coefficient.
  // We won't use the implicit system of units of Geant4 because
  // it results in loss of numerical precision.

  energy = energy / eV;
  G4double virial = 0.;

  for (G4int i=0; i<3; i++)
  virial = virial + P[i] / (energy*energy - E[i]*E[i]);

  // Need to use g/cm3
  density = density / g * cm3;

  G4double mol_density = density / 131.29;
  G4double alpha = virial * mol_density;

  // Isolating now the n2 from equation (1) and taking the square root
  G4double n2 = (1. - 2*alpha) / (1. + alpha);
  if (n2 < 1.) {
    //      G4String msg = "Non-physical refractive index for energy "
    // + bhep::to_string(energy) + " eV. Use n=1 instead.";
    //      G4Exception("[XenonProperties]", "RefractiveIndex()",
    // 	  JustWarning, msg);
    n2 = 1.;
  }

  return sqrt(n2);
}



G4double GArXeScintillation(G4double energy, G4double pressure)
{
  // FWHM and peak of emission extracted from paper:
  // Physical review A, Volume 9, Number 2,
  // February 1974. Koehler, Ferderber, Redhead and Ebert.
  // Pressure must be in atm = bar
  // XXX Check if there is some newest results.

  pressure = pressure / atmosphere;

  G4double Wavelength_peak  = (0.05 * pressure + 169.45) * nm;

  G4double Wavelength_sigma = 0.;
  if (pressure < 4.)
  Wavelength_sigma = 14.3 * nm;
  else
  Wavelength_sigma = (-0.117 * pressure + 15.16) * nm / (2.*sqrt(2*log(2)));

  G4double Energy_peak  = (h_Planck * c_light / Wavelength_peak);
  G4double Energy_sigma = (h_Planck * c_light * Wavelength_sigma / pow(Wavelength_peak,2));

  G4double intensity = exp(-pow(Energy_peak/eV-energy/eV,2) /
  (2*pow(Energy_sigma/eV, 2))) /
  (Energy_sigma/eV*sqrt(pi*2.));

  return intensity;
}

G4double ArXeScintillation(G4double energy, G4double pressure)
{
  G4int ppm = 1000/10^6;  // Parts per million
  // Convert pressure to atm
  pressure = pressure / atmosphere;

  // Define the absorption fraction of Ar light by Xe (pressure-dependent)
  // TEMPORARY!!!!!
  G4double alpha = 0.15;  // Constant dependent on data (temporary!!!!!!)
  G4double f_Xe = 1.0 - exp(-alpha * pressure * ppm);  // Xe fraction modifies absorption

  // Argon Primary Scintillation (Peak ~128 nm)
  G4double Wavelength_Ar_peak = 128.0 * nm;
  G4double Wavelength_Ar_sigma = 5.0 * nm;  // Typical width of Ar scintillation

  G4double Energy_Ar_peak = (h_Planck * c_light / Wavelength_Ar_peak);
  G4double Energy_Ar_sigma = (h_Planck * c_light * Wavelength_Ar_sigma / pow(Wavelength_Ar_peak, 2));

  // Xenon Secondary Scintillation (Peak ~172 nm)
  G4double Wavelength_Xe_peak = (0.05 * pressure + 169.45) * nm;
  G4double Wavelength_Xe_sigma = (pressure < 4.) ? 14.3 * nm : (-0.117 * pressure + 15.16) * nm / (2.*sqrt(2*log(2)));

  G4double Energy_Xe_peak = (h_Planck * c_light / Wavelength_Xe_peak);
  G4double Energy_Xe_sigma = (h_Planck * c_light * Wavelength_Xe_sigma / pow(Wavelength_Xe_peak, 2));

  // Argon scintillation intensity (before absorption)
  G4double intensity_Ar = exp(-pow(Energy_Ar_peak/eV - energy/eV, 2) / (2 * pow(Energy_Ar_sigma/eV, 2))) /
                          (Energy_Ar_sigma/eV * sqrt(pi * 2.));

  // Xenon re-emitted intensity (from absorbed Argon light)
  G4double intensity_Xe = exp(-pow(Energy_Xe_peak/eV - energy/eV, 2) / (2 * pow(Energy_Xe_sigma/eV, 2))) /
                          (Energy_Xe_sigma/eV * sqrt(pi * 2.));

  // Final intensity = unabsorbed Argon + re-emitted Xenon
  G4double intensity = (1.0 - f_Xe) * intensity_Ar + f_Xe * intensity_Xe;

  return intensity;
}




G4double ArXeELLightYield(G4double field_strength, G4double pressure)
{
  // Empirical formula taken from
  // C.M.B. Monteiro et al., JINST 2 (2007) P05001.

  // Y/x = (a E/p - b) p,
  // where Y/x is the number of photons per unit lenght (cm),
  // E is the electric field strength, p is the pressure
  // a and b are empirically determined constants depending on pressure.
  // Values for different pressures are found in: Freitas-2010
  // Physics Letters B 684 (2010) 205–210

  const G4double b = 116. / (bar*cm);
  G4double a = 140. / kilovolt;

  // Updating the slope
  if (pressure >= 2. * bar) a = 141. / kilovolt;
  if (pressure >= 4. * bar) a = 142. / kilovolt;
  if (pressure >= 5. * bar) a = 151. / kilovolt;
  if (pressure >= 6. * bar) a = 161. / kilovolt;
  if (pressure >= 8. * bar) a = 170. / kilovolt;

  // Getting the yield
  G4double yield = (a * field_strength/pressure - b) * pressure;
  if (yield < 0.) yield = 0.;

  return yield;
}


G4double GetGasDensity(G4double pressure, G4double temperature, G4int ppm)
{
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "Correct pressure method " << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  G4cout << "------------------------------------" << G4endl;
  // Interpolate to calculate the density
  // at a given pressure and temperature
  G4double density = 5.324 * kg/m3;
  std::vector<std::vector<G4double>> data;
  std::pair<G4int, G4int> nkeys = MakeXeDensityDataTable(data);
  G4int npressures = nkeys.first;
  G4int ntemps     = nkeys.second;

  // Find correct interval and use bilinear interpolation
  G4bool found = false;
  G4int tcount = 0;
  G4int pcount = 0;
  G4int count = 0;
  G4double t1, t2, p1, p2, d11, d12, d21, d22, d1, d2;
  G4double argon_density, xenon_density;

  while (tcount < ntemps-1) {
    t1 = data[count][0];
    t2 = data[count+npressures][0];

    if (temperature >= t1 && temperature < t2) {
      while (pcount < npressures-1) {
        p1 = data[count][1];
        p2 = data[count+1][1];

        if (pressure >= p1 && pressure < p2) {
          d11 = data[count][2];
          d12 = data[count+1][2];
          d21 = data[count+npressures][2];
          d22 = data[count+npressures+1][2];
          argon_density = ArgonDensity(pressure);
          xenon_density = BilinearInterpolation(temperature, t1, t2,
            pressure, p1, p2, d11, d12, d21, d22);
          density = ((1 - ppm) * argon_density) + (ppm * xenon_density);
          found = true;
        break;
        }
        pcount++;
        count++;
      }

      if (!found) {
        if (pressure == data[count][1]) {
          d1 = data[count][2];
          d2 = data[count+npressures][2];
          argon_density = ArgonDensity(pressure);
          xenon_density = LinearInterpolation(temperature, t1, t2, d1, d2);
          density = ((1 - ppm) * argon_density) + (ppm * xenon_density);
          found = true;
        } else {
          throw "Unknown xenon density for this pressure!";
        }
      }

      break;
    }
    tcount++;
    pcount = 0;
    count += npressures;
  }

  if (!found) {
    if (temperature == data[count][0]) {
      while (pcount< npressures-1) {
        p1 = data[count][1];
        p2 = data[count+1][1];

        if (pressure >= p1 && pressure < p2){
          d1 = data[count][2];
          d2 = data[count+1][2];
          argon_density = ArgonDensity(pressure);
          xenon_density = LinearInterpolation(pressure, p1, p2, d1, d2);
          density = ((1 - ppm) * argon_density) + (ppm * xenon_density);
          found = true;
          break;
        }
        pcount++;
        count++;
      }
      if (!found) {
        if (pressure == data[count][1]) {
          argon_density = ArgonDensity(pressure);
          xenon_density = data[count][2];
          density = ((1 - ppm) * argon_density) + (ppm * xenon_density);
          found = true;
        } else {
          throw "Unknown ArXe density for this pressure!";
        }
      }
    } else {
        throw "Unknown ArXe density for this temperature";
    }
  }
  return density;
}
