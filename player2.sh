#!/bin/bash

# MQTT Broker IP
BROKER="35.233.231.192"

# Generate a random X and Y coordinate
x=$((RANDOM % 3))
y=$((RANDOM % 3))

# Send the move (X,Y) to the ESP32
mosquitto_pub -h $BROKER -t esp32/aiRound2 -m "$x,$y"

echo "Player 2 made a move: ($x,$y)"
