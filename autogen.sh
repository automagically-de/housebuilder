#!/bin/sh

set -x

aclocal -I m4/
autoheader
automake --gnu --add-missing --copy
autoconf
