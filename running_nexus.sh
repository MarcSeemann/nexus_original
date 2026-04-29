#!/bin/bash

# Geant4 Path, edit G4Install path to where the main geant4 code folder your downloaded
export G4INSTALL=/home/investigator/Documents/simulation_software//geant4-v11.0.2/install;
export PATH=$G4INSTALL/bin:$PATH;
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$G4INSTALL/lib;
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$G4INSTALL/lib;

echo geant4;

cd $G4INSTALL/bin;
source geant4.sh;
cd -;

echo g4finished;

export QT_PATH=/home/investigator/Qt/5.15.2/gcc_64;
export PATH=$QT_PATH/bin:$PATH;
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$QT_PATH/lib;
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$QT_PATH/lib;

echo QT;

# Path to GSL
export GSL_PATH=/usr/local;
export PATH=$GSL_PATH/bin:$PATH;
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$GSL_PATH/lib;
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$GSL_PATH/lib;

echo gsl;

# Path to HDF5
export HDF5_PATH=/usr/local/hdf5;  # Set the installation path of HDF5
export HDF5_LIB=/usr/local/hdf5/lib;  # Set the path to HDF5 libraries
export HDF5_INC=/usr/local/hdf5/include;  # Set the path to HDF5 headers

echo hdf5;

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HDF5_LIB;
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:$HDF5_LIB;

echo sourcing;

# Source the default nexus setup file to set the nexusdir environmental variable
source /home/$USER/Documents/simulation_software/nexus_original/scripts/nexus_setup.sh;

echo sourced;

# Add the nexus exe to the path
export PATH=$PATH:/home/$USER/Documents/simulation_software/nexus_original/bin;

# IMPORTANT: Add the local nexus library path to ensure RealisticMuonGenerator is available
export LD_LIBRARY_PATH=/home/$USER/Documents/simulation_software/nexus_original/lib:$LD_LIBRARY_PATH;
export DYLD_LIBRARY_PATH=/home/$USER/Documents/simulation_software/nexus_original/lib:$DYLD_LIBRARY_PATH;

echo "Added local nexus library path";

#cmake compilation
#cmake_compile () {
#cd /home/$USER/Documents/simulation_software/nexus_original
#rm -r /home/$USER/Documents/simulation_software/nexus_original/build
#cd /home/$USER/Documents/simulation_software/nexus_original
#mkdir -p build    
#cmake --build build --target clean
#cmake --build build --target install
#cd build;
#cmake ..;
#cmake --build . -j 32;
#cd ..;
#}

compile_function() {
    echo "=========================================="
    echo "Building NEXUS..."
    echo "=========================================="
    
    cd /home/$USER/Documents/simulation_software/nexus_original/build
    make -j4
    local make_status=$?
    
    if [ $make_status -eq 0 ]; then
        echo ""
        echo "=========================================="
        echo "Installing libraries and executables..."
        echo "=========================================="
        
        # Copy library from build to lib directory
        if [ -f "libnexus.so" ]; then
            echo "  Copying libnexus.so to lib/"
            cp libnexus.so /home/$USER/Documents/simulation_software/nexus_original/lib/libnexus.so
            echo "  ✓ Library installed"
        else
            echo "  ✗ Warning: libnexus.so not found!"
        fi
        
        # Copy executable from build to bin directory
        if [ -f "nexus" ]; then
            echo "  Copying nexus executable to bin/"
            cp nexus /home/$USER/Documents/simulation_software/nexus_original/bin/nexus
            chmod +x /home/$USER/Documents/simulation_software/nexus_original/bin/nexus
            echo "  ✓ Executable installed"
        else
            echo "  ✗ Warning: nexus executable not found!"
        fi
        
        # Copy test executable if it exists
        if [ -f "nexus-test" ]; then
            echo "  Copying nexus-test executable to bin/"
            cp nexus-test /home/$USER/Documents/simulation_software/nexus_original/bin/nexus-test
            chmod +x /home/$USER/Documents/simulation_software/nexus_original/bin/nexus-test
            echo "  ✓ Test executable installed"
        fi
        
        echo ""
        echo "=========================================="
        echo "✓ Build and installation complete!"
        echo "=========================================="
    else
        echo ""
        echo "✗ Build failed!"
        return 1
    fi
    
    cd /home/$USER/Documents/simulation_software/nexus_original/
}

echo "Setup Nexus is complete!"
echo "To compile and install, run: compile_function"
echo "This will:"
echo "  1. Build with 'make -j4'"
echo "  2. Copy libnexus.so to lib/"
echo "  3. Copy nexus executable to bin/"
echo ""
echo "NOTE: This ensures your code changes are properly compiled and installed!"
