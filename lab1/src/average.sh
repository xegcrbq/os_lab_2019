#!/bin/bash

a=0 

b=$*

c=0

for var in $@

do

a=$($a+$var)


done

c=$($a/$b)

echo $c
