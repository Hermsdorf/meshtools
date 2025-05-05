// Gmsh project created on Wed Feb 19 15:51:35 2025
SetFactory("OpenCASCADE");
//+
Sphere(1) = {0, 0, 0, 50, -Pi/2, Pi/2, 2*Pi};
//+
Box(2) = {-100, -100, 0, 400, 200, -200};
//+
BooleanDifference{ Volume{2}; Delete; }{ Volume{1}; Delete; }
//+
Rotate {{25, 0, 0}, {0, 0, 0}, Pi} {
  Duplicata { Volume{2}; }
}
//+
Recursive Delete {
  Point{12}; Point{4}; Point{2}; Point{1}; Point{14}; Point{3}; Point{20}; Point{10}; Point{15}; Point{7}; Point{9}; Point{5}; Point{6}; Point{19}; 
}
//+
Recursive Delete {
  Point{15}; 
}
//+
Delete {
  Point{15}; 
}
