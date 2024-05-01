
L1 = 1.0;
L2 = 1.0;
ps = 0.25;
ncells_x = 4;
ncells_y = 4;

Point(1) = {0,0,0,ps};
Point(2) = {L1,0,0,ps};
Point(3) = {L1, L2, 0, ps};
Point(4) = {0, L2,0,ps};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};

//+
Transfinite Curve {3, 1} = ncells_x Using Progression 1;
//+
Transfinite Curve {4, 2} = ncells_y Using Progression 1;
//+
Curve Loop(1) = {3, 4, 1, 2};
//+
Plane Surface(1) = {1};
//+
Transfinite Surface {1};



//+
Physical Curve("botton", 1) = {1};
//+
Physical Curve("top", 2) = {3};
//+
Physical Curve("left", 3) = {4};
//+
Physical Curve("right", 4) = {2};
//+
Physical Surface("domain", 5) = {1};
//+
Recombine Surface {1};
