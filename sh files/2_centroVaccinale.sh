#!/bin/sh
cd ..
echo "Avvio il centro vaccinale..."
gcc centroVaccinale.c -o centroVaccinale
echo "***** CENTRO_VACCINALE *****"
./centroVaccinale
read
