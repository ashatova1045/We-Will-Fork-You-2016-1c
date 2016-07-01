#!/bin/bash


echo "Borrando logs"

<<<<<<< HEAD
cd ../
find -name '*.log' -delete
cd -
=======
rm ../swap/logSwap.log
rm ../nucleo/logEstados.log
rm ../nucleo/logNucleo.log
cd ../umc/
find -name '*.log' -delete
cd -
rm ../cpu/logCPU.log
rm ../tests/consola.log
>>>>>>> 1d25d9f845fb3f407ba4bbcc289aea40b4a25cc6

echo "Fin"
