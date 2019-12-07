#!/bin/bash
./meshtools msh/rectangle.msh 10
gprof meshtools gmon.out > rectangle10.txt
./meshtools msh/rectangle.msh 20
gprof meshtools gmon.out > rectangle20.txt
./meshtools msh/boxFace.msh 12
gprof meshtools gmon.out > boxFace12.txt
./meshtools msh/boxFace.msh 20
gprof meshtools gmon.out > boxFace20.txt
./meshtools msh/boxInternal.msh 4
gprof meshtools gmon.out > boxInternal4.txt
./meshtools msh/boxInternal.msh 8
gprof meshtools gmon.out > boxInternal8.txt
./meshtools msh/boxInternal.msh 12
gprof meshtools gmon.out > boxInternal12.txt
./meshtools msh/boxInternal.msh 20
gprof meshtools gmon.out > boxInternal20.txt
./meshtools msh/sphereTorus.msh 4
gprof meshtools gmon.out > sphereTorus4.txt
./meshtools msh/sphereTorus.msh 8
gprof meshtools gmon.out > sphereTorus8.txt
./meshtools msh/sphereTorus.msh 12
gprof meshtools gmon.out > sphereTorus12.txt
./meshtools msh/sphereTorus.msh 20
gprof meshtools gmon.out > sphereTorus20.txt
