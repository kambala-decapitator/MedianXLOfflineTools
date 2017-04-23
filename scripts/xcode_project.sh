#!/bin/sh
QMAKE_PATH="$1"
PRO_FILE_PATH="$2"
"$QMAKE_PATH" -spec macx-xcode "$PRO_FILE_PATH"