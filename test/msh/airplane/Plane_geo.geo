Merge "Plane_stp.STEP";
SetFactory("OpenCASCADE");

PlaneSurfaces() = Surface In BoundingBox {-1490, -290, -690, 1500, 600, 1300};
//+
Physical Surface("BOUND") = {PlaneSurfaces()};

dx = 400;
dy = 200;
dz = 400;
//+
Box(2) = {-1350-dx,-90-dy,-520-dz, 1360+(2*dx), 370+(2*dy), 1040+(2*dz)};
//+
Diff() = BooleanDifference{Volume{2}; Delete;}{Volume{1}; Delete}; // NAO TA FAZENDO O DIFF
//+
Physical Volume("FLUID") = {Diff()};