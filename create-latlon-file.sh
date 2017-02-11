#!/bin/sh
grep "^throu\|^rea\|^from" "$1"  | cut -d ' ' -f 3 | sed 's/,/\ /g;s/(/\ /g;s/)/\ /;'
