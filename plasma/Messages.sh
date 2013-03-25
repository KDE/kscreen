#!/usr/bin/env bash

$XGETTEXT `find . -name '*.cpp'` -o $podir/plasma_applet_org.kde.plasma.kscreen.pot
$XGETTEXT `find . -name '*.qml'` -j -o $podir/plasma_applet_org.kde.plasma.kscreen.pot
rm -f rc.cpp
