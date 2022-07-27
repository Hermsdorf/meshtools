
pd = 0.2;

Point(1) = {0.0, 0.0, 0.0, pd};
Point(2) = {1.0, 0.0, 0.0, pd};
Point(3) = {1.0, 1.0, 0.0, pd};
Point(4) = {0.0, 1.0, 0.0, pd};

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};



//+
Curve Loop(1) = {4, 1, 2, 3};
//+
Plane Surface(1) = {1};
//+
//Transfinite Curve {1, 3} = 3 Using Progression 1;
//+
//Transfinite Curve {4, 2} = 3 Using Progression 1;
//+
Transfinite Surface {1};

//+
Physical Curve("CONTORNO", 1) = {4, 1, 2, 3};

//+
Physical Surface("SURFACE", 2) = {1};
