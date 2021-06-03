Merge "Plane_stp.STEP";
SetFactory("OpenCASCADE");

PlaneSurfaces() = Surface In BoundingBox {-1490, -290, -690, 1500, 600, 1300};
//+
Physical Surface("PLANE") = {PlaneSurfaces()};