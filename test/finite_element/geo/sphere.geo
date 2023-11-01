
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
Physical Surface("BOUND", 1)  = {12, 13, 11, 10, 8, 9};
//+
Physical Volume("SPHERE", 2)   = {2};
//+
Physical Volume("OUTSPHERE", 3) = {3};

//+
Physical Surface("SPHERE_SURFACE", 4) = {7};

//+ sphere
MeshSize {10, 9} = 0.01;

Mesh.MeshSizeMin = 0.01;
Mesh.MeshSizeMax = 0.01;
Mesh.Algorithm   = 6;





