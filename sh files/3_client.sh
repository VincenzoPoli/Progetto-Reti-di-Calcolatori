#!/bin/sh
cd ..
echo "Avvio il client..."
gcc client.c -o client
echo "***** CLIENT *****"
./client
