#!/bin/bash


echo "Borrando logs"

rm ../swap/logSwap.log
rm ../nucleo/logEstados.log
rm ../nucleo/logNucleo.log
cd ../umc/
find -name '*.log' -delete
cd -
rm ../cpu/logCPU.log
rm ../tests/consola.log

echo "Fin"
