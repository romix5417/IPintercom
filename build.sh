#!/bin/sh

Ver=$1

if [ ${Ver}x != ""x ];
then
   make clean
   make
   rm ESTW_IPintercom_v*
   mv meshcom ESTW_IPintercom_v${Ver}
else
   echo "Not have version number!"   
fi

