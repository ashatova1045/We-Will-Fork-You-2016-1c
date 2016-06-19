#!/bin/bash
command -v make >/dev/null 2>&1 || { echo >&2 "Necesito make, pero no esta!!"; exit 1; }
command -v sudo >/dev/null 2>&1 || { echo >&2 "Necesito sudo, pero no esta!!"; exit 1; }

echo "Compilando Swap"
cd ../swap/Debug/
make clean all
cd -

echo "Compilando UMC"
cd ../umc/Debug/
make clean all
cd -

echo "Compilando Nucleo"
cd ../nucleo/Debug/
make clean all
cd -

echo "Compilando CPU"
cd ../cpu/Debug/
make clean all
cd -

echo "Compilando Consola"
cd ../consola/Debug/
make clean all
cd -


echo "Fin.."
