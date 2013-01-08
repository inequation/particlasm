#!/bin/sh

# This little makefile builds an image for the disc I have to turn in together
# with the thesis printout. Requires mkisofs/genisoimage.

# choose based on what's on your OS
#MKISOFS=mkisofs
MKISOFS=genisoimage

command -v ${MKISOFS} >/dev/null 2>&1 || { echo >&2 "Missing mkisofs/genisoimage: ${MKISOFS}"; exit 1; }

${MKISOFS} -fRJ -V "leszgod081_185498_2013" -o disc-image.iso ./disc
