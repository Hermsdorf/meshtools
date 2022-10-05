
xmin = 0;
xmax = 10.0;
ymin = 0.0;
ymax = 2.0;
pc   = 0.1;

xc   = 1.0;
yc   = 1.0;

Point(1) = {xmin,ymin,0.0,pc};
Point(2) = {xmax,ymin,0.0,pc};
Point(3) = {xmax,ymax,0.0,pc};
Point(4) = {xmin,ymax,0.0,pc};

Point(5) = {xc-0.5,yc-0.5,0.0,pc};
Point(6) = {xc+0.5,yc-0.5,0.0,pc};
Point(7) = {xc+0.5,yc+0.5,0.0,pc};
Point(8) = {xc-0.5,yc+0.5,0.0,pc};




//+
Line(1) = {1, 2};
//+
Line(2) = {3, 2};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 1};
//+
Line(5) = {8, 5};
//+
Line(6) = {5, 6};
//+
Line(7) = {6, 7};
//+
Line(8) = {7, 8};
//+
Curve Loop(1) = {3, 4, 1, -2};
//+
Curve Loop(2) = {7, 8, 5, 6};
//+
Plane Surface(1) = {1, 2};
//+
Plane Surface(2) = {2};
//+
Physical Curve("CONTORNO") = {3, 4, 1, 2};
//+
Physical Surface("IC1", 2) = {2};
//+
Physical Surface("IC2", 3) = {1};
