// All Units in SI
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

// Constants
const float pi = 3.1415;

// Generation Algorithm Parameters
const int airfoilCurvePointCardinality = 100; // Real Number of Generated Points - 2 (because of weird loop iteration)
int pointIndex = 1; // Point Index
int lineIndex = 1;

// Geometric Parameters (Solid)
const float T = 0.15; // Normalized Thickness of Airfoil
const float s = 0.03084; // Chord Length
const float l_airfoil = 0.0654; // Airfoil Length
const float r_hub = 0.067564; // Hub Radius

// Geometric Parameters (Fluid)
const float l_duct = 0.3; // Duct Length
const float r_duct = 0.20828; // Duct Radius

// Geometric Parameters (Calculated)
const float deltaX = s / ((airfoilCurvePointCardinality - 1) / 2); // Airfoil Generation delta(x)
const float x_intersect = (r_hub * std::sqrt(2 - std::sqrt(2))) / 2; // Hub x-axis Intersection

// // For Generation of Hub + Duct
class wall {

    public:

        static float hub_y(float x) { return 0.08;/*std::sqrt(pow(r_hub, 2) - pow(x, 2)) - r_hub * sin((3 * pi) / 8)*/;}

        static float duct_x() { return ((2 * r_hub * std::sqrt(2 - std::sqrt(2))) 
                                      + (pow(8, 0.25) * std::sqrt(2) * std::sqrt(std::sqrt(2) - 1) * (r_duct - r_hub))) / 4; }
                                      
        static float duct_y() { return ((r_duct - r_hub) * (std::sqrt(2 + std::sqrt(2)))) / 2; }

};

// Syntactic Point Constructor
string pointConstructor(float x, float y, float z, int index) {
    
    const string point = "Point(" + std::to_string(index) + ") = {" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + "};";

    return point;

}

// Airfoil Point as a Function of x and beth Generator
string airfoilPointGenerator(float x, int index, int beth) {

    const float z = beth * T * ((1.4845 * std::sqrt(s * (x + (s / 2))))
                                - (0.63 * (x + (s / 2)))
                                - ((1.758 * std::pow(x + (s / 2), 2)) / s) 
                                + ((1.4215 * std::pow(x + (s / 2), 3)) / pow(s, 2))
                                - ((0.518 * pow(x + (s / 2), 4)) / pow(s, 3))); // Airfoil Curve

    return pointConstructor(x, wall::hub_y(x), z, index);
    
}

// Generation of Gmsh Points
string pointGenerator(string domain) {

    string points = "\n";

    if (domain == "solid") {

        for (int i = 1; i < airfoilCurvePointCardinality - 1; i++) {

            const float iteratedX = (pointIndex <= airfoilCurvePointCardinality / 2) ? (s / 2) - ((pointIndex - 1) * deltaX) :
                                                                                      -(s / 2) + ((pointIndex - (airfoilCurvePointCardinality / 2)) * deltaX);

            points += (pointIndex <= airfoilCurvePointCardinality / 2) ? "\n" + airfoilPointGenerator(iteratedX, pointIndex, -1) :
                                                                         "\n" + airfoilPointGenerator(iteratedX, pointIndex, 1);

            pointIndex++;

        }

    } else if (domain == "fluid") {

        points += "\n" + pointConstructor(-x_intersect, 0, l_duct / 2, pointIndex); pointIndex++;
        points += "\n" + pointConstructor(x_intersect, 0, l_duct / 2, pointIndex); pointIndex++;
        points += "\n" + pointConstructor(wall::duct_x(), wall::duct_y(), l_duct / 2, pointIndex); pointIndex++;
        points += "\n" + pointConstructor(-wall::duct_x(), wall::duct_y(), l_duct / 2, pointIndex); pointIndex++;
        points += "\n" + pointConstructor(0, -std::sqrt(pow(r_hub, 2) - pow(x_intersect, 2)), l_duct / 2, pointIndex); pointIndex++;
        points += "\n" + pointConstructor(0, -std::sqrt(pow(r_hub, 2) - pow(x_intersect, 2)), l_duct / 2, pointIndex); pointIndex++;

    }

    return points;

}

// Generation of Gmsh Lines
class lineGenerator {


    public:

        string lines = "\n";
        
        static string airfoilLines() {

            string lines = "\n";

            for (int i = 1; i < airfoilCurvePointCardinality - 1; i++) {

                if (i < airfoilCurvePointCardinality - 2) {

                    lines += "\nLine(" + to_string(lineIndex) + ") = {" + to_string(lineIndex) + ", " + to_string(lineIndex + 1) + "};"; lineIndex++;

                } else {

                    lines += "\nLine(" + to_string(lineIndex) + ") = {" + to_string(lineIndex) + ", " + to_string(1) + "};"; lineIndex++;

                }

            }

            return lines; 

        }

        static string ductLines() {

            string lines = "\n\nLine(" + to_string(lineIndex) + ") = {100, 99};"; lineIndex++;
            lines += "\nLine(" + to_string(lineIndex) + ") = {100, 101};"; lineIndex++;
            lines += "\nLine(" + to_string(lineIndex) + ") = {101, 102};"; lineIndex++;
            lines += "\nLine(" + to_string(lineIndex) + ") = {102, 99};"; lineIndex++;

            return lines; 

        }
        
};

class loopGenerator {

    public:

        static string airfoilLoop() {

            string loop = "\n\nCurve Loop(1) = {";

            for (int i = 1; i < airfoilCurvePointCardinality - 1; i++) {

                if (i < airfoilCurvePointCardinality - 2) {

                    loop += to_string(i) + ", ";

                } else {

                    loop += to_string(i) + "};";

                }
                
            }

            return loop;

        }

        static string ductLoop() {

            return "\nCurve Loop(2) = {99, 100, 101, 102};";

        }
};

class surfaceGenerator {

    public:

        static string airfoilSurface() { return "\n\nSurface(1) = {1};"; }

        static string ductSurface() { return "\nSurface(2) = {2};"; }

};

class volumeGenerator {

    public:

        static string airfoilVolume() { return "\n\nExtrude {0, " + to_string(l_airfoil) + ", 0} {Surface{1}; Layers{1};}"; }

        static string ductVolume() { return "\nExtrude {0, 0, " + to_string(-l_duct) + "} {Surface{2}; Layers{10};} \nvolumes() = BooleanDifference { Volume{2}; Delete; }{ Volume{1}; Delete; };";}

};

int main() {

    remove("fluidDomain.geo");

    ofstream fluidDomainFile("fluidDomain.geo", std::ofstream::trunc);

    fluidDomainFile << "SetFactory(\"OpenCASCADE\");"; // Use OpenCASCADE Kernel

    fluidDomainFile << pointGenerator("solid");

    fluidDomainFile << pointGenerator("fluid");

    fluidDomainFile << lineGenerator::airfoilLines();

    fluidDomainFile << lineGenerator::ductLines();

    fluidDomainFile << loopGenerator::airfoilLoop();

    fluidDomainFile << loopGenerator::ductLoop();

    fluidDomainFile << surfaceGenerator::airfoilSurface();

    fluidDomainFile << surfaceGenerator::ductSurface();

    fluidDomainFile << volumeGenerator::airfoilVolume();

    fluidDomainFile << volumeGenerator::ductVolume();

    fluidDomainFile << "\n\nMesh 3;";

    fluidDomainFile.close();

}