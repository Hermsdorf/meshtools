//+
Point(1) = {0, 0, 0, 1.0};
//+
Point(2) = {1, 0, 0, 1.0};
//+
Point(3) = {1, 1, 0, 1.0};
//+
Point(4) = {0, 1, 0, 1.0};
//+
Point(5) = {0, 0.33, 0, 1.0};
//+
Line(1) = {1, 2};
//+
Line(2) = {2, 3};
//+
Line(3) = {3, 4};
//+
Line(4) = {4, 5};
//+
Line(5) = {5, 1};
//+
Curve Loop(1) = {4, 5, 1, 2, 3};
//+
Plane Surface(1) = {1};
//+
Physical Curve("BOUND1", 2) = {4};
//+
Physical Curve("BOUND0", 1) = {5};
//+
Physical Curve("BOUND0", 1) += {1};
//+
Physical Surface("SURFACE", 6) = {1};
//+
Physical Curve("CURVE", 7) = {3, 2};
