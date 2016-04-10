#!/usr/bin/env bash
$EXTRACTRC `find . -name '*.ui'` >> rc.cpp
$XGETTEXT `find . -name '*.cpp'` -o $podir/kcm_displayconfiguration.pot
$XGETTEXT `find . -name '*.qml'` -j -L Java -o $podir/kcm_displayconfiguration.pot
rm -f rc.cpp
