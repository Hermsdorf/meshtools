// Gmsh project created on Tue Dec  3 09:45:32 2019

lc = 0.1;
lcs = 0.05;

xmin = -1.0;
ymin = -1.0;
zmin = -1.0;
xmax = 1.0;
ymax = 1.0;
zmax = 1.0;

xc = 0.0;
yc = 0.0;
zc = 0.0;
raio = 0.25;

// Cube geometry
Point(1) = {xmin, ymin, zmin, lc};
Point(2) = {xmax, ymin, zmin, lc};
Point(3) = {xmax, ymax, zmin, lc};
Point(4) = {xmin, ymax, zmin, lc};

Point(5) = {xmin, ymin, zmax, lc};
Point(6) = {xmax, ymin, zmax, lc};
Point(7) = {xmax, ymax, zmax, lc};
Point(8) = {xmin, ymax, zmax, lc};


Point(9) = {xc,yc,zc, lcs};
Point(10) = {xc+raio,yc,zc, lcs};
Point(11) = {xc-raio,yc,zc, lcs};
Point(12) = {xc,yc+raio,zc, lcs};
Point(13) = {xc,yc-raio,zc, lcs};
Point(14) = {xc,yc,zc+raio, lcs};
Point(15) = {xc,yc,zc-raio, lcs};


//+
Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 1};
//+
Line(5) = {5, 6};
//+
Line(6) = {6, 7};
//+
Line(7) = {7, 8};
//+
Line(8) = {8, 5};
//+
Line(9) = {1, 5};
//+
Line(10) = {2, 6};
//+
Line(11) = {3, 7};
//+
Line(12) = {4, 8};
//+

//+
Circle(13) = {10, 9, 15};
//+
Circle(14) = {15, 9, 11};
//+
Circle(15) = {11, 9, 14};
//+
Circle(16) = {14, 9, 10};
//+
Circle(17) = {11, 9, 13};
//+
Circle(18) = {13, 9, 14};
//+
Circle(19) = {13, 9, 10};
//+
Circle(20) = {13, 9, 15};
//+
Circle(21) = {14, 9, 12};
//+
Circle(22) = {12, 9, 10};
//+
Circle(23) = {12, 9, 15};
//+
Circle(24) = {12, 9, 11};
//+
Curve Loop(1) = {4, 1, 2, 3};
//+
Plane Surface(1) = {1};
//+
Curve Loop(2) = {8, 5, 6, 7};
//+
Plane Surface(2) = {2};
//+
Curve Loop(3) = {9, 5, -10, -1};
//+
Plane Surface(3) = {3};
//+
Curve Loop(4) = {10, 6, -11, -2};
//+
Plane Surface(4) = {4};
//+
Curve Loop(5) = {11, 7, -12, -3};
//+
Plane Surface(5) = {5};
//+
Curve Loop(6) = {12, 8, -9, -4};
//+
Plane Surface(6) = {6};
//+
Curve Loop(7) = {24, 15, 21};
//+
Surface(7) = {7};
//+
Curve Loop(8) = {19, -16, -18};
//+
Surface(8) = {8};
//+
Curve Loop(9) = {17, 20, 14};
//+
Surface(9) = {9};
//+
Curve Loop(10) = {15, -18, -17};
//+
Surface(10) = {10};
//+
Curve Loop(11) = {20, -13, -19};
//+
Surface(11) = {11};
//+
Curve Loop(12) = {21, 22, -16};
//+
Surface(12) = {12};
//+
Curve Loop(13) = {22, 13, -23};
//+
Surface(13) = {13};
//+
Curve Loop(14) = {23, 14, -24};
//+
Surface(14) = {14};
//+
Surface Loop(1) = {6, 5, 4, 3, 2, 1};
//+
Surface Loop(2) = {13, 12, 7, 14, 9, 10, 8, 11};
//+
Volume(1) = {1, 2};
//+
Physical Surface("BOTTOM", 1) = {1};
//+
Physical Surface("TOP", 2) = {2};
//+
Physical Surface("LEFT", 3) = {6};
//+
Physical Surface("RIGHT", 4) = {4};
//+
Physical Surface("BACK", 5) = {3};
//+
Physical Surface("FRONT", 6) = {5};
//+
Physical Surface("SPHERE", 7) = {10, 8, 12, 7, 9, 14, 13, 11};
//+
Physical Volume("FLUID", 8) = {1};
