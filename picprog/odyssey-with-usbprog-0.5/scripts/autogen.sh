#!/bin/sh

ACLOCAL_FLAGS="-I scripts"
AUTOMAKE_FLAGS="--foreign --add-missing --copy"

echo "Generating build information using autoheader, automake, and autoconf"
echo "This may take a while ..."

# Touch the timestamps on all the files since CVS messes them up
touch configure.ac

# Regenerate configuration files
#rm -f acinclude.m4
#for FILE in scripts/*.m4; do
#	cat $FILE >> acinclude.m4
#done
aclocal ${ACLOCAL_FLAGS} && autoheader ${AUTOHEADER_FLAGS} && automake ${AUTOMAKE_FLAGS} && autoconf ${AUTOCONF_FLAGS}

rm -f config.cache
rm -rf autom4te.cache

# Run configure for this platform
#./configure $*
echo "Now you are ready to run ./configure"
