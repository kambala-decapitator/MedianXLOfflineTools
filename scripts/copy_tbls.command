#!/usr/bin/env bash
scriptDir=$(dirname "$0")
destDir="$scriptDir/../utils/txt_parser/tbl"
srcDir="/Volumes/hdd/games/d2/medianxl-rus/tbl/sigma"
cp -f "$srcDir"/en/*.txt "$destDir"/en
cp -f "$srcDir"/ru/*.txt "$destDir"/ru
