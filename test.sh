#!/bin/bash

set -e -u


echo -e "\n\tC O M P I L A T I O N\n"

make mrproper 
make -j4 all

outdir="out"
rmdir="rm -rf"
mkdir="mkdir -p"

if [ -e bin/lang -a -e sample/sample ]
then
    echo -e "\n\tE X E C U T I O N\n"

	if [ -e $outdir ] ; then $rmdir $outdir; fi
	$mkdir $outdir
	path=`pwd`
	
	cd $outdir
	
    $path/bin/lang < $path/sample/sample

    if [ -e out.dot ]
    then
		echo -e "\n\t GENERATING PNG FROM DOT FILE\n"
        dot out.dot -Tpng -o out.png
    fi
	
	cd -
fi
