#!/bin/sh
cd ..
echo "Avvio il clientS..."
gcc clientS.c -o clientS
echo "***** CLIENTS *****"
./clientS
read
