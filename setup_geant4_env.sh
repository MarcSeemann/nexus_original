#!/bin/bash
# Complete Geant4 environment setup script

# Base Geant4 installation path
export G4INSTALL=/home/investigator/Documents/simulation_software/geant4-v11.0.2/install

# Library paths
export LD_LIBRARY_PATH=$G4INSTALL/lib:/home/investigator/Qt/5.15.2/gcc_64/lib:./build:$LD_LIBRARY_PATH

# Geant4 data directories
export G4NEUTRONHPDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4NDL4.6
export G4LEDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4EMLOW8.0
export G4LEVELGAMMADATA=$G4INSTALL/share/Geant4-11.0.2/data/PhotonEvaporation5.7
export G4RADIOACTIVEDATA=$G4INSTALL/share/Geant4-11.0.2/data/RadioactiveDecay5.6
export G4PARTICLEXSDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4PARTICLEXS4.0
export G4PIIDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4PII1.3
export G4REALSURFACEDATA=$G4INSTALL/share/Geant4-11.0.2/data/RealSurface2.2
export G4SAIDXSDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4SAIDDATA2.0
export G4ABLADATA=$G4INSTALL/share/Geant4-11.0.2/data/G4ABLA3.1
export G4INCLDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4INCL1.0
export G4ENSDFSTATEDATA=$G4INSTALL/share/Geant4-11.0.2/data/G4ENSDFSTATE2.3

# Echo environment status
echo "✅ Geant4 environment set up successfully!"
echo "   G4INSTALL: $G4INSTALL"
echo "   G4LEDATA: $G4LEDATA"
echo "   G4ENSDFSTATEDATA: $G4ENSDFSTATEDATA"
echo ""
