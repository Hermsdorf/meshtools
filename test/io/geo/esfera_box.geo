// Gmsh project created on Wed Feb 19 15:51:35 2025
SetFactory("OpenCASCADE");
//+
Sphere(1) = {0, 0, 0, 50, -Pi/2, Pi/2, 2*Pi};
//+
Box(2) = {-100, -100, 0, 400, 200, -200};
//+
BooleanDifference{ Volume{2}; Delete; }{ Volume{1}; Delete; }
//+
Rotate {{25, 0, 0}, {0, 0, 0}, Pi} {
  Duplicata { Volume{2}; }
}
//+
Physical Volume("VOLUME") = {2, 3};
//+
Physical Surface("SHELL", 1) = {9, 4, 5, 2, 11, 12};
//+
Physical Surface("FRONT", 2) = {1, 8};
//+
Physical Surface("BACK", 3) = {6, 13};
//+
Physical Surface("ESPERA", 4) = {7, 14};
