#!/bin/bash

echo "Seteando IPs"
echo "IP SWAP:"
read ip_swap
find ../configuraciones/* -type f -exec \
    sed -i 's/^IP_SWAP=.*$/IP_SWAP='$ip_swap'/g' {} +

echo "IP UMC:"
read ip_umc
find ../configuraciones/* -type f -exec \
    sed -i 's/^IP_UMC=.*$/IP_UMC='$ip_umc'/g' {} +

echo "IP NUCLEO:"
read ip_nucleo
find ../configuraciones/* -type f -exec \
    sed -i 's/^IP_NUCLEO=.*$/IP_NUCLEO='$ip_nucleo'/g' {} +


sed -i 's/^IP_KERNEL=.*$/IP_KERNEL='$ip_nucleo'/g' ../consola/consola.cfg

sed -i 's/^IP_UMC=.*$/IP_UMC='$ip_umc'/g' ../cpu/cpu.cfg
sed -i 's/^IP_NUCLEO=.*$/IP_NUCLEO='$ip_nucleo'/g' ../cpu/cpu.cfg

echo "Fin"
