#!/bin/sh
#
# Display a list of processes on the system whose bounding set does
# not include all capabilities.
#
# Get capabilities of init(1), which should have a complete bounding set.
# (This future-proofs the script against the day when further capabilities
# are added to Linux.)
#
init_caps=$(awk '$1 == "CapBnd:" {print $2}' < /proc/1/status)

# Get list of PIDs whose bounding set does not match that of init(1).

pids=$(grep CapBnd /proc/*/status | grep -v "$init_caps" | 
		awk -F/ '{print $3}' | sort -n)

# Display PID, PPID, command, and capabilities of each process

for p in $pids; do
    ps -o 'pid ppid comm' h $p
    caps=$(awk '$1 == "CapBnd:" {print $2}' < /proc/$p/status)
    capsh --decode=$caps | sed 's/=/ = { /; s/$/ }/; s/,/ /g' | fold -s -w64 | sed "s/^/\t\t/"
done
