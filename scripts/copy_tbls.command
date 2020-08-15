#!/usr/bin/env bash
scriptDir=$(dirname "$0")
destDir="$scriptDir/../utils/txt_parser/tbl"
srcDir=~/dev/diablo/medianxl-rus/tbl/sigma
cp -f "$srcDir"/en/*.txt "$destDir"/en
cp -f "$srcDir"/ru/*.txt "$destDir"/ru
