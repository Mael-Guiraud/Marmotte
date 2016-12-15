#!/bin/bash
 make compil

for i in `seq 1 7`;
do

	make &
done

