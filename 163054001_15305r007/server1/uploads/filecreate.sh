#! /bin/bash
for n in {1..500}; do
    #dd if=/dev/urandom of=file$( printf %03d "$n" ).bin bs=1 count=$(( RANDOM + 1024 ))
    base64 /dev/urandom|head -c 50000000 >  50MB$n.txt
done
