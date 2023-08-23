
//+
SetFactory("OpenCASCADE");
//+
Box(1) = {0, 0, 0, 1, 1, 1};

//+
Sphere(2) = {0.35, 0.35, 0.35, 0.15, -Pi/2, Pi/2, 2*Pi};
//+

//+
Physical Surface("BOUND", 1) = {1, 5, 4, 6, 2, 3};
//+
Physical Volume("FLUID", 2) = {1, 2};


