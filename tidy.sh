#!/bin/sh
set -eou pipefail
uncrustify -c ./etc/uncrustify.cfg --replace include/*.c include/*.h src/*.c *.c
shfmt -w *.sh
find . -name "*unc-backup*" -type f | xargs -I % unlink %
rm *unc-backup*||true
#js-beautify -r clib.json
