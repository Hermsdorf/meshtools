
pc = 0.25;
cp = 0.1;

Point(1) = {-1.0, -1.0, 0.0, pc};
Point(2) = { 1.0, -1.0, 0.0, pc};
Point(3) = { 1.0,  1.0, 0.0, pc};
Point(4) = {-1.0,  1.0, 0.0, pc};
Point(5) = {0.0,  0.0 , 0.0, cp};
Point(6) = {0.0, -0.25, 0.0, cp};
Point(7) = {0.0,  0.25, 0.0, cp};
//+
Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {1, 4};
//+
Circle(5) = {7, 5, 6};
//+
Circle(6) = {6, 5, 7};
//+
Curve Loop(1) = {4, -3, -2, -1};
//+
Curve Loop(2) = {5, 6};
//+
Plane Surface(1) = {1, 2};
//+
Physical Curve("left", 1) = {4};
//+
Physical Curve("right", 2) = {2};
//+
Physical Curve("bottom", 3) = {1};
//+
Physical Curve("top", 4) = {3};
//+
Physical Curve("circle", 5) = {6, 5};
//+
Physical Surface("fluid", 6) = {1};
