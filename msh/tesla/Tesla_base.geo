SetFactory("OpenCASCADE");

Merge "Tesla_base.stp";

CarSurfaces() = Surface In BoundingBox {-1700, -1700, 0, 7800, 3000, 1500};

dx = 0;
dy = 0;
dz = 0;
//+//+   
Box(2) = {0-dx,-1080-dy,0-dz, 5000+dx,2200+dy, 1500+dz};


//+
Physical Surface("CAR") = {CarSurfaces()};

//Physical Surface("BOUND") = {-1800, -1800, 0, 8800, 1237, 1238};

