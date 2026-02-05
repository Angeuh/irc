#!/bin/bash

gnome-terminal --title="IRC SERVER" -- bash -c "./SERVER 8080 123; bash"
sleep 2

xterm -e "irssi -n opUser -c localhost -p 8080" &
sleep 2
xdotool type "/RAWLOG OPEN debugOp.log"
xdotool key Return
sleep 2
xdotool type "/join #test"
xdotool key Return

xterm -e "irssi -n riri -c localhost -p 8080" &
sleep 2
xdotool type "/RAWLOG OPEN debugRiri.log"
xdotool key Return
sleep 2
xdotool type "/join #test"
xdotool key Return
