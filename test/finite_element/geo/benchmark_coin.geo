pts_box    = 0.1;
pts_circle = 0.05;

//+
Point(1) = {0, 0, 0, pts_box};
//+
Point(2) = {0, 1, 0, pts_box};
//+
Point(3) = {1, 1, 0, pts_box};
//+
Point(4) = {1, 0, 0, pts_box};

//+
Point(5) = {0.5, 0.85, 0, pts_circle};
//+
Point(6) = {0.5, 0.75, 0, pts_circle};
//+
Point(7) = {0.5, 0.65, 0, pts_circle};
//+
Line(1) = {1, 4};
//+
Line(2) = {4, 3};
//+
Line(3) = {3, 2};
//+
Line(4) = {2, 1};
//+
Circle(5) = {7, 6, 5};
//+
Circle(6) = {5, 6, 7};
//+
Curve Loop(1) = {4, 1, 2, 3};
//+
Curve Loop(2) = {6, 5};
//+
Plane Surface(1) = {1, 2};
//+
Plane Surface(2) = {2};
//+
Physical Curve("BOUND", 1) = {4, 3, 2, 1};
//+
Physical Surface("FLUID", 2) = {1, 2};
