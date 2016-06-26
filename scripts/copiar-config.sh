#!/bin/bash

# Print to stdout
 echo "1. Prueba 1"
 echo "2. Prueba 2"
 echo "3. Prueba 3"
 echo "4. Prueba 4"

echo "Ingrese el numero de Prueba [1,2,3 o 4] para cargar la config correspondiente \c"
echo "Ingrese 0 para cargar la configuracion predeterminada"

read numero

echo "Copiando las configuraciones para la prueba Nro - $numero "

cp -f ../configuraciones/config_prueba$numero/nucleo.cfg ../nucleo
cp -f ../configuraciones/config_prueba$numero/swap.cfg ../swap
cp -f ../configuraciones/config_prueba$numero/umc.cfg ../umc

echo "Fin"
