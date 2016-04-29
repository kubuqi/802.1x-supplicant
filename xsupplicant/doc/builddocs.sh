#!/bin/sh

# this shouldn't be hard coded, but it is for now :-)  perhaps the
# best option is just to require users of this script to define
# the variable on their own?  not quite sure, I guess that's why
# it's hard coded for now...
JADE_PUB=/usr/share/doc/openjade-1.3.2/pubtext

mkdir -p html
mkdir -p txt
mkdir -p pdf

# make sure that jade is in the user's path
JADE_PATH=`which jade 2> /dev/null`
if [ x$JADE_PATH == x ]
then
    echo "jade not found in your path...exiting!"
    exit 1
fi

# create the html docs
echo -n "Building the HTML docs..."
$JADE_PATH -V nochunks -t sgml -i html -d ./ldp-dsssl/ldp.dsl\#html \
    $JADE_PUB/xml.dcl xml-userguide/userguide.xml > html/Open1x-UserGuide.html
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
else
    echo "Done."
fi

# create the txt docs
echo -n "Building the TXT docs..."
lynx -dump -nolist html/Open1x-UserGuide.html > txt/Open1x-UserGuide.txt
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
else
    echo "Done."
fi

# create the pdf docs
echo -n "Building the PDF docs..."
cd pdf/
$JADE_PATH -t tex -d ../ldp-dsssl/ldp.dsl\#print $JADE_PUB/xml.dcl \
    ../xml-userguide/userguide.xml > /dev/null
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
fi
mv ../xml-userguide/userguide.tex .
pdfjadetex userguide.tex > /dev/null
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
fi
pdfjadetex userguide.tex > /dev/null
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
fi
pdfjadetex userguide.tex > /dev/null
if [ $? -ne 0 ]
then
    echo "Error!  Build failed!"
    exit 1
fi
rm -f userguide.tex userguide.log userguide.aux userguide.out
mv userguide.pdf Open1x-UserGuide.pdf
cd ../
echo "Done."
