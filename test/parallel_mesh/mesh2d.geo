// Gmsh project created on Sat Feb  5 15:57:59 2022

xmin = 0.0;
xmax = 1.0;
ymin = 0.0;
ymax = 1.0;

ps   = 0.25;

Point(1) = {xmin,ymin,0.0,ps};
Point(2) = {xmax,ymin,0.0,ps};
Point(3) = {xmax,ymax,0.0,ps};
Point(4) = {xmin,ymax,0.0,ps};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};


//+
Curve Loop(1) = {4, 1, 2, 3};
//+
Plane Surface(1) = {1};
//+
Transfinite Surface {1};
//+
Physical Curve("FACE_B", 1) = {1};
//+
Physical Curve("FACE_R", 2) = {2};
//+
Physical Curve("FACE_T", 3) = {3};
//+
Physical Curve("FACE_L", 4) = {4};
//+
Physical Surface("SURFACE", 5) = {1};
