#!/bin/sh

AMQP_URL=amqp://guest:guest@127.0.0.1
QUEUE_NAME=oneir
ONEIR_SOCK=/tmp/oneird.sock

amqp-consume -u "$AMQP_URL" -q "$QUEUE_NAME" socat - "UNIX-CONNECT:$ONEIR_SOCK"

