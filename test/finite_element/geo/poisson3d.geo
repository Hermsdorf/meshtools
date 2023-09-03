// Gmsh project created on Sat Jul 22 15:24:12 2023
SetFactory("OpenCASCADE");
//+
Box(1) = {0, 0, 0, 1, 1, 1};
//+
Physical Surface("CONTOUR", 1) = {1, 5, 4, 2, 6, 3};
//+
Physical Volume("FLUID", 2) = {1};
