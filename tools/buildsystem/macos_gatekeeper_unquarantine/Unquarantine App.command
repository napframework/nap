#!/bin/bash
cd -- "$(dirname "$0")"
echo
echo "This script will now run a command to remove the app from 'quarantine', allowing it to be used without interference from macOS' Gatekeeper"
echo
read -p "Press [return] to continue"
xattr -d -r com.apple.quarantine .

# Useful for users who won't have Terminal windows set to auto-close on a clean exit code
echo "Command completed. You can now safely close this window."
