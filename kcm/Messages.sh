#!/usr/bin/env bash

$XGETTEXT `find . -name '*.cpp'` -o $podir/kcm_displayconfiguration.pot
$XGETTEXT `find . -name '*.qml'` -j -L Java -o $podir/kcm_displayconfiguration.pot
rm -f rc.cpp