
xmin = 0;
xmax = 1.0;
ymin = 0.0;
ymax = 1.0;
pc   = 0.1;


Point(1) = {xmin,ymin,0.0,pc};
Point(2) = {xmax,ymin,0.0,pc};
Point(3) = {xmax,ymax,0.0,pc};
Point(4) = {xmin,ymax,0.0,pc};




//+

//+
Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 1};
//+
Curve Loop(1) = {3, 4, 1, 2};
//+
Plane Surface(1) = {1};
//+
Transfinite Surface {1};
//+
Physical Curve("BC", 1) = {4, 3, 2, 1};
//+
Physical Surface("DOMAIN", 2) = {1};
