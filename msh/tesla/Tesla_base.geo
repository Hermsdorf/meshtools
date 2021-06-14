SetFactory("OpenCASCADE");

Merge "Tesla_base.stp";

CarSurfaces() = Surface In BoundingBox {-1700, -1700, 0, 7800, 3000, 1500};

dx = 800;
dy = 800;
dz = 200;
//+
Box(2) = {0-dx,-1080-dy,0-dz, 5000+(2*dx),2200+(2*dy), 1500+(2*dz)};
//+
Diff() = BooleanDifference{Volume{2}; Delete;}{Volume{1}; Delete}; // NAO TA FAZENDO O DIFF

//+
Physical Surface("BOUND") = {CarSurfaces()};
//+
Physical Volume("FLUID") = {Diff()};
