#!/bin/bash

PORT=${1:-6667}
PASS=${2:-"password"}
USER=${3:-"user"}

echo "=== Envoi massif de commandes rapides ==="

    for i in $(seq 1 1000); do
		sh test2.sh 6667 password h_$i &
	done

echo "=== Test termine ==="
