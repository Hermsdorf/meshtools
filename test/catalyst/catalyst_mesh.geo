
lc = 0.25;
Point(1) = {0.0, 0.0, 0.0,lc};
Point(2) = {1.0, 0.0, 0.0,lc};
Point(3) = {1.0, 1.0, 0.0,lc};
Point(4) = {0.0, 1.0, 0.0,lc};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};


//+
Curve Loop(1) = {4, 1, 2, 3};
//+
Plane Surface(1) = {1};
//+

Physical Curve("BOUNDARY", 1) = {4, 3, 2, 1};
Physical Surface("DOMAIN", 2) = {1};


//+
Transfinite Curve {4, 1, 2, 3} = 5 Using Progression 1;
//+
Transfinite Surface {1};

//+
Recombine Surface {1};
//+

