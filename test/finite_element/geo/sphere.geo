
//+
SetFactory("OpenCASCADE");
//+
Box(1) = {0, 0, 0, 1, 1, 1};

//+
// Sphere(2) = {0.35, 0.35, 0.35, 0.15, -Pi/2, Pi/2, 2*Pi};
//+

Sphere(2) = {0.35, 0.35, 0.35, 0.15};



//+
BooleanFragments{ Volume{1}; Volume{2}; Delete; }{ }

//+
Physical Surface("BOUND", 1) = {1, 5, 4, 6, 2, 3};
//+
Physical Volume("FLUID", 2) = {1, 2};


//+ sphere
MeshSize {10, 9} = 0.01;

Mesh.MeshSizeMin = 0.005;
Mesh.MeshSizeMax = 0.025;

