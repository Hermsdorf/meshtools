SetFactory("OpenCASCADE");

Merge "Tesla_base.stp";

CarSurfaces() = Surface In BoundingBox {-1700, -1700, 0, 7800, 3000, 1500};

//+
Physical Surface("CAR") = {CarSurfaces()};

Physical Surface("BOUND") = {-1800, -1800, 0, 8800, 1237, 1238};
//+