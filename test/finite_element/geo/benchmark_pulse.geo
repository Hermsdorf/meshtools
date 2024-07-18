pts_box    = 0.05;


//+
Point(1) = {0, 0, 0, pts_box};
//+
Point(2) = {0, 10, 0, pts_box};
//+
Point(3) = {10, 10, 0, pts_box};
//+
Point(4) = {10, 0, 0, pts_box};


//+
Line(1) = {1, 4};
//+
Line(2) = {4, 3};
//+
Line(3) = {3, 2};
//+
Line(4) = {2, 1};

Curve Loop(1) = {4, 1, 2, 3};
//+
//+
Plane Surface(1) = {1};
//+
//+
Physical Curve("BOUND", 1) = {4, 3, 2, 1};
//+
Physical Surface("DOMAIN", 2) = {1};
//+

Transfinite Curve {4, 2, 1, 3} = 64 Using Progression 1;
//+
Transfinite Curve {4, 3, 2, 1} = 64 Using Progression 1;
//+
Transfinite Surface {1};
//+
Recombine Surface {1};
