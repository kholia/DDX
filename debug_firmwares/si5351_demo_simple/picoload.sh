#!/usr/bin/env bash

if test -f "$1"; then
  sudo picotool load -f -x $1
  sleep 5
  sudo picotool load -f -x $1
else
  echo "Cannot find file '$1'"
fi
