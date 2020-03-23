#!/bin/sh

./sortedtest a b c
./sortedtest a c b
./sortedtest b c a
./sortedtest b a c
./sortedtest c a b
./sortedtest c b a
./sortedtest a b b
./sortedtest a bb b
./sortedtest a b bb
./sortedtest a bb ba
./sortedtest a bb bc
./sortedtest a ba bb
./sortedtest a bc bb

