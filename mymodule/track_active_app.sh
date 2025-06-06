#!/bin/bash

DEVICE=/dev/mydevice
last_window=""

while true; do
  current_window=$(xdotool getwindowfocus getwindowname 2>/dev/null)
  if [ "$current_window" != "$last_window" ] && [ ! -z "$current_window" ]; then
    echo "$current_window" | tee "$DEVICE"
    last_window="$current_window"
  fi
  sleep 1
done

