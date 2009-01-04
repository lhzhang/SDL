#!/bin/sh
#
# Print the current source revision, if available

srcdir=`dirname $0`/..

if [ -d $srcdir/.svn ]; then
    cd $srcdir
    svnversion -c | sed -e 's,[0-9]*:\([0-9]*\)[A-Z]*,\1,'
fi