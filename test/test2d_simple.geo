pc = 1.0;
cp = 0.1;

Point(1) = {-1.0, -1.0, 0.0, pc};
Point(2) = { 1.0, -1.0, 0.0, pc};
Point(3) = { 1.0,  1.0, 0.0, pc};
Point(4) = {-1.0,  1.0, 0.0, pc};

Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {1, 4};

//+
Physical Curve("bounday", 1) = {4, 1, 2, 3};
//+
Curve Loop(1) = {3, -4, 1, 2};
//+
Plane Surface(1) = {1};
//+
Physical Surface("surface", 2) = {1};
