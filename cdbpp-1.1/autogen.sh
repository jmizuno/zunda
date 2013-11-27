#!/bin/sh
# $Id: autogen.sh 2 2009-07-08 15:15:22Z naoaki $

if [ "$1" = "--force" ];
then
    FORCE=--force
    NOFORCE=
    FORCE_MISSING=--force-missing
else
    FORCE=
    NOFORCE=--no-force
    FORCE_MISSING=
fi

aclocal $FORCE || {
    echo "aclocal failed!"
    exit 1
}

autoheader $FORCE || {
    echo "autoheader failed!"
    exit 1
}

automake -a -c $NOFORCE || {
    echo "automake failed!"
    exit 1
}

autoconf $FORCE || {
    echo "autoconf failed!"
    exit 1
}
