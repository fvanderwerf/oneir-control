#!/bin/sh

AMQP_URL=amqp://guest:guest@127.0.0.1
QUEUE_NAME=oneir
ONEIR_SOCKET=/var/run/oneird.sock

if [[ $# > 1 ]]
then
    AMQP_URL="$1"
fi

shift

if [[ $# > 1 ]]
then
    QUEUE_NAME="$1"
fi

shift

if [[ $# > 1 ]]
then
    ONEIR_SOCKET="$1"
fi

amqp-declare-queue -u "$AMQP_URL" -q "$QUEUE_NAME"
amqp-consume -u "$AMQP_URL" -q "$QUEUE_NAME" socat - "UNIX-CONNECT:$ONEIR_SOCKET"

