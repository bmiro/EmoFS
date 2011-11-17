#!/usr/bin/env bash
if [ ! -e 'src' ]; then
  mkdir src
fi
for i in `ls ../*.[hc]`
do
    cat $i |sed 's/à/a/g'| sed 's/á/a/g'| sed 's/é/e/g'| sed 's/è/e/g'| sed 's/í/i/g'| sed 's/ó/o/g'| sed 's/ò/o/g'| sed 's/ú/u/g'| sed 's/ç/c/g'| sed 's/À/A/g'| sed 's/Á/A/g'| sed 's/É/E/g'| sed 's/È/E/g'| sed 's/Í/I/g'| sed 's/Ó/O/g'| sed 's/Ò/O/g'| sed 's/Ú/U/g'| sed 's/Ç/C/g' | sed 's/ñ/n/g' > "src/a/$i"
done
