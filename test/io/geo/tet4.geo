
L1 = 1.0;
L2 = 1.0;
ps = 0.25;
L3 = 1.0;
ncells_x = 4;
ncells_y = 4;
ncells_z = 4;

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
Extrude {0, 0, L3} {
  Surface{1}; Layers{ncells_z};
}

Surface Loop(1) = {21, 1, 13, 17, 26, 25};

Volume(2) = {1};

//+
Physical Surface("bottom", 1) = {1};
//+
Physical Surface("top", 2) = {26};
//+
Physical Surface("left", 3) = {17};
//+
Physical Surface("right", 4) = {25};
//+
Physical Surface("back", 5) = {21};
//+
Physical Surface("front", 6) = {13};
//+

//+


Physical Volume("domain", 7) = {1};
//+

