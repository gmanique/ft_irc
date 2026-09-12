#!/bin/bash
# stress_fuzz.sh

PORT=${1:-6667}

echo "=== Envoi de donnees corrompues (Fuzzing) ==="

# 1. Envoie 100 Ko de charabia binaire
head -c 100000 /dev/urandom | nc 127.0.0.1 $PORT

# 2. Envoie une ligne gigantesque sans '\r\n'
python3 -c "print('A' * 100000)" | nc 127.0.0.1 $PORT

echo "=== Test termine ==="
