#!/bin/bash

# Contiki directory
CONTIKI=$1

# Simulation file
BASENAME=07-native-border-router-cooja

bash test-native-border-router.sh $CONTIKI $BASENAME fd00::204:4:4:4 60 600 2
