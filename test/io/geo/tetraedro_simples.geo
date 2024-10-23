SetFactory("OpenCASCADE");

Physical Surface("bottom") = {21};
Physical Surface("sides") = {22,23,24};
Physical Volume("tetra") = {41};

Point(1) = {0, 0, 0, 0.1};
Point(2) = {0, 0, 2, 0.1};
Point(3) = {2, 0, 0, 0.1};
Point(4) = {0, 4, 0, 0.1};

Line(1) = {1, 2};
Line(2) = {2, 3};
Line(3) = {3, 1};
Line(4) = {1, 4};
Line(5) = {2, 4};
Line(6) = {3, 4};

Line Loop(11) = {1, 2, 3};
Line Loop(12) = {2, 5, 6};
Line Loop(13) = {3, 4, 6};
Line Loop(14) = {1, 4, 5};


Plane Surface(21) = {11};
Plane Surface(22) = {12};
Plane Surface(23) = {13};
Plane Surface(24) = {14};

Surface Loop(31) = {21, 22, 23, 24};
Volume(41) = {31};




