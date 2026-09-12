#!/bin/bash

PORT=${1:-6667}
PASS=${2:-"password"}
USER=${3:-"user"}

echo "=== Envoi massif de commandes rapides ==="

{
    echo "CAP LS 302"
    echo "PASS $PASS"
    echo "NICK $USER"
    echo "USER spammer 0 * :Spam User"
    echo "CAP END"
    echo "JOIN #spamchan"
    
    for i in $(seq 1 1000); do
        echo "PRIVMSG #spamchan :Message de spam numéro $i"
        echo "PING :spam_$i"
    done
    
    echo "QUIT :Fini"
} | nc 127.0.0.1 $PORT > /dev/null

echo "=== Test termine ==="
