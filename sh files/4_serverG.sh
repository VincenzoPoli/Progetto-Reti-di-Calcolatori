#!/bin/sh
cd ..
echo "Avvio il serverG..."
gcc serverG.c -o serverG
echo "***** SERVER_G *****"
./serverG
read
