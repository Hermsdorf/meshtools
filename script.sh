#!/bin/bash
# export OMP_NUM_THREADS=1
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/serial/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv MULTI__* testes_papi/tesla/serial
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/serial/plane
# export OMP_NUM_THREADS=2
# export OMP_SCHEDULE="STATIC"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/static256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv MULTI__* testes_papi/tesla/2threads/static
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/static256/plane
# export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/static256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv MULTI__* testes_papi/tesla/4threads/static
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/static256/plane
# export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/static256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv MULTI__* testes_papi/tesla/8threads/static
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/static256/plane
# export OMP_NUM_THREADS=2
# export OMP_SCHEDULE="STATIC,512"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/static512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/2threads/static512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/static512/plane
# export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/static512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/4threads/static512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/static512/plane
# export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/static512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/8threads/static512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/static512/plane
export OMP_NUM_THREADS=2
export OMP_SCHEDULE="DYNAMIC"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/dynamic256/art
./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
mv MULTI__* testes_papi/tesla/2threads/dynamic-nowait-barrier
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/dynamic256/plane
export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/dynamic256/art
./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
mv MULTI__* testes_papi/tesla/4threads/dynamic-nowait-barrier
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/dynamic256/plane
export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/dynamic256/art
./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
mv MULTI__* testes_papi/tesla/8threads/dynamic-nowait-barrier
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/dynamic256/plane
# export OMP_NUM_THREADS=2
# export OMP_SCHEDULE="DYNAMIC,512"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/dynamic512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/2threads/dynamic512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/dynamic512/plane
# export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/dynamic512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/4threads/dynamic512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/dynamic512/plane
# export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/dynamic512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/8threads/dynamic512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/dynamic512/plane
# export OMP_NUM_THREADS=2
# export OMP_SCHEDULE="GUIDED,256"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/guided256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/2threads/guided256/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/guided256/plane
# export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/guided256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/4threads/guided256/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/guided256/plane
# export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/guided256/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/8threads/guided256/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/guided256/plane
# export OMP_NUM_THREADS=2
# export OMP_SCHEDULE="GUIDED,512"
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/2threads/guided512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/2threads/guided512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/2threads/guided512/plane
# export OMP_NUM_THREADS=4
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/4threads/guided512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/4threads/guided512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/4threads/guided512/plane
# export OMP_NUM_THREADS=8
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/arteria/art_msh.msh 2
# mv profile* tempos/8threads/guided512/art
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/tesla/tesla_msh.msh 2
# mv profile* tempos/8threads/guided512/tesla
# ./meshtools /home/guilherme/Documents/TCC\ ICE/Geometrias/plane/plane_msh.msh 2
# mv profile* tempos/8threads/guided512/plane