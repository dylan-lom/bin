#!/bin/sh
#
# remmina(1) wrapper.
# usage: rdp <name|server>

mtime() {
	stat -c "%Y" "$1"
}

files="$HOME/.local/share/remmina/*"

target=""
for f in $(egrep -li "^(server|name)=.*$1.*$" $files); do
	# Prefer the most recently used connection if we got multiple matches,
	# there is probably a "better" way to do this but I think this is fine (TM)
	test -z "$target" && target="$f" # first loop iteration

	if [ `mtime $f` -gt `mtime $target` ]; then
		target="$f"
	fi
done

if [ -z "$target" ]; then
    echo "ERROR: Could not find a saved connection for $1" > /dev/stderr
    exit 1
fi

remmina "$target"