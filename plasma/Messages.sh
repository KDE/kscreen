#!/usr/bin/env bash

$XGETTEXT `find . -name '*.cpp'` -o $podir/plasma_applet_kscreen.pot
$XGETTEXT `find . -name '*.qml'` -j -L Java -o $podir/plasma_applet_kscreen.pot
rm -f rc.cpp
