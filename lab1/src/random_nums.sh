#!/bin/bash
> numbers.txt
for i in {1..150}
do
echo $(( $(od -An -N1 /dev/random))) >> numbers.txt # -An Display Without Offsets ,,,,, Octal Dump
done
