#!/bin/bash
cd -- "$(dirname "$0")"
echo "This script will now run a command to remove the framework directory from 'quarantine', allowing it to be used without interference from macOS' Gatekeeper"
echo
read -p "Press [return] to continue"
xattr -d -r com.apple.quarantine ..
