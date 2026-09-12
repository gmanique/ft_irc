#!/bin/bash
# stress_conn.sh

PORT=${1:-6667}
PASS=${2:-"password"}
NB_CLIENTS=1025

echo "=== Lancement de $NB_CLIENTS connexions simultanees ==="

for i in $(seq 1 $NB_CLIENTS); do
    (
        {
            echo "CAP LS 302"
            echo "PASS $PASS"
            echo "NICK User_$i"
            echo "USER user_$i 0 * :Test User"
            echo "CAP END"
            sleep 5
            echo "QUIT :Bye"
        } | nc -q 1 127.0.0.1 $PORT > /dev/null 2>&1
    ) &
done

wait
echo "=== Test termine ==="
