
pts = 0.1;
xmin = 0.0;
xmax = 1.0;
ymin = 0.0;
ymax = 1.0;

ybc  = 0.2;

Point(1) = {xmin,ymin,0.0, pts};
Point(2) = {xmax,ymin,0.0,pts};
Point(3) = {xmax,ymax,0.0, pts};
Point(4) = {xmin, ymax, 0.0, pts};
Point(5) = {xmin, ybc, 0.0, pts};
Point(6) = {xmax, ybc, 0.0, pts};

//+
Line(1) = {1, 2};
//+
Line(2) = {2, 6};
//+
Line(3) = {6, 3};
//+
Line(4) = {3, 4};
//+
Line(5) = {4, 5};
//+
Line(6) = {5, 1};

//+
Curve Loop(1) = {5, 6, 1, 2, 3, 4};
//+
Plane Surface(1) = {1};
//+
//+
Transfinite Curve {4, 1} = 9 Using Progression 1;
//+
Transfinite Surface {1};
//+
Recombine Surface {1};
