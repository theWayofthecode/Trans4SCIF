#!/bin/bash

make

if [ $? -ne 0 ]; then
	echo "make failed";
	exit $?;
fi

./main > host.output &
micnativeloadex mainmmic > xeon_phi.output
wait $!
echo "==================HOST=============="
cat host.output
echo "==================Xeon Phi=============="
cat xeon_phi.output