#! /bin/bash
#
# scanscript.sh - a scanscript used with scanadf
#
# Usage: scanscript.sh <imagefile>
#   where imagefile is the image data just scanned

if [ $# -lt 1 ]; then
echo "Usage: $0 <patchfile.patch>"
exit 1
fi

bzr send -o $1

