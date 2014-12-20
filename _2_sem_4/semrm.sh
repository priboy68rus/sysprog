#!/bin/bash

sem=$(ipcs | grep "^s")
while read -r line; do
	a=($line)
	b=${a[1]}
	ipcrm -s $b
done <<< "$sem"
