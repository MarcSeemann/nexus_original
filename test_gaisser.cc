// Test program to verify Gaisser energy generation
#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <algorithm>

// Simplified version of the Gaisser energy generation
double GenerateRealisticMuonEnergy() {
    double E_min = 1.0;  // GeV
    double E_max = 50.0; // GeV 
    double epsilon_star = 115.0; // GeV
    
    // Find maximum value of Gaisser function in range for normalization
    double max_value = 0.0;
    for(double E_test = E_min; E_test <= E_max; E_test += 0.1) {
        double value = std::pow(E_test, -2.7) * std::pow(1.0 + E_test/epsilon_star, -3.64);
        if(value > max_value) max_value = value;
    }
    
    // Rejection sampling
    std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);
    
    double energy;
    double gaisser_value;
    do {
        energy = E_min + (E_max - E_min) * dis(gen);
        gaisser_value = std::pow(energy, -2.7) * std::pow(1.0 + energy/epsilon_star, -3.64);
    } while (dis(gen) * max_value > gaisser_value);
    
    return energy;
}

int main() {
    std::vector<double> energies;
    
    std::cout << "Testing Gaisser energy generation..." << std::endl;
    
    // Generate 1000 energies
    for(int i = 0; i < 1000; i++) {
        energies.push_back(GenerateRealisticMuonEnergy());
    }
    
    // Calculate statistics
    double sum = 0;
    for(double e : energies) {
        sum += e;
    }
    double mean = sum / energies.size();
    
    std::sort(energies.begin(), energies.end());
    double median = energies[energies.size()/2];
    
    int low_count = 0;
    int high_count = 0;
    for(double e : energies) {
        if(e < 5.0) low_count++;
        if(e > 30.0) high_count++;
    }
    
    std::cout << "Results for " << energies.size() << " samples:" << std::endl;
    std::cout << "Mean energy: " << mean << " GeV" << std::endl;
    std::cout << "Median energy: " << median << " GeV" << std::endl;
    std::cout << "Min energy: " << energies[0] << " GeV" << std::endl;
    std::cout << "Max energy: " << energies.back() << " GeV" << std::endl;
    std::cout << "Fraction < 5 GeV: " << (double)low_count/energies.size() << std::endl;
    std::cout << "Fraction > 30 GeV: " << (double)high_count/energies.size() << std::endl;
    std::cout << std::endl;
    
    std::cout << "Expected for uniform (1-50 GeV):" << std::endl;
    std::cout << "Mean: 25.5 GeV, Median: 25.5 GeV, Frac<5GeV: 0.082, Frac>30GeV: 0.408" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Expected for Gaisser:" << std::endl;
    std::cout << "Mean: <10 GeV, Median: <5 GeV, Frac<5GeV: >0.7, Frac>30GeV: <0.1" << std::endl;
    
    return 0;
}
