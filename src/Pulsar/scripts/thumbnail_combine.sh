#!/bin/ksh 

#find all the th.png files and convert them into a list to paste together using "montage".
#generally used in the Pulsar Pipeline, at the top level OBSID_red (processing location)
#optional input argument is the pipeline logfile, where STDOUT statements are repeated

log="NONE"

if [ $# -eq 1 ]                    # get the log file name
then
   log=$1
fi

if [ $log != "NONE" ]
then
   echo "thumbnail_combine.sh script is writing to the following log file: $log"
   echo "thumbnail_combine.sh script is writing to the following log file: $log" >> $log
fi

if [ -f combined.th.png ]
then 
   echo "WARNING: deleting previous version of results: combined.th.png"
   if [ $log != "NONE" ]
   then
      echo "WARNING: deleting previous version of results: combined.th.png" >> $log
   fi
   rm combined.th.png
fi

if [ -f combined.png ]
then 
   echo "WARNING: deleting previous version of results: combined.png"
   if [ $log != "NONE" ]
   then
      echo "WARNING: deleting previous version of results: combined.png" >> $log
   fi
   rm combined.png
fi

# find all the paths to the th.png files
find ./ -name "*.th.png" -print | sort  > /tmp/$$_combine_col1.txt

find ./ -name "*prepout" -print -exec grep -i squared {} \; | sed 's/prepout/prepout \\/g' | sed -e :a -e '/\\$/N; s/\\\n//; ta' | sort | sed 's/^.*= //g'  > /tmp/$$_combine_col3.txt

# find all the paths to the th.png files and print just the RSP? location and the Pulsar name to the output file
#find ./ -name "*.th.png" -print  | sed -e 's/\// /g' -e 's/^.* //g' -e 's/.*_RSP/RSP/g' -e 's/\..*//g'  -e 's/_PSR//g' > /tmp/$$_combine_col2.txt

# find all the paths to the th.png files and print just the IS (incoherentstokes) or CS (stokes) 
#   and RSP? location and the Pulsar name to the output file


find ./ -name "*.th.png" -print | sort | sed -e 's/\/incoherentstokes\//_IS_/g' -e 's/\/stokes\//_CS_/g' -e 's/\//_/g' -e 's/^.*_IS_/IS_/g' -e 's/^.*_CS_/CS_/g' -e 's/_L20.*//g' -e 's/_RSP._/&\\n/g' -e 's/_beam_./\\n&\\n/g'  > /tmp/$$_combine_col2.txt

paste /tmp/$$_combine_col1.txt /tmp/$$_combine_col2.txt /tmp/$$_combine_col3.txt | awk '{print "-label \""$2"\\nChiSq = " $3"\" "$1" "}' | tr -d '\n' | awk '{print "montage -background none "$0" combined.png"}' > combine_png.sh
rm /tmp/$$_combine_col1.txt /tmp/$$_combine_col2.txt /tmp/$$_combine_col3.txt
wc_convert=`wc -l combine_png.sh | awk '{print $1}'`

if [[ $wc_convert > 0 ]]
then
   chmod 777 combine_png.sh
   echo "Executing the following comamnd: "

   if [ $log != "NONE" ]
   then
      echo "Executing the following comamnd: " >> $log
      cat combine_png.sh >> $log
   fi

   cat combine_png.sh
   ./combine_png.sh
   convert -resize 200x140 -bordercolor none -border 150 -gravity center -crop 200x140-0-0 +repage combined.png combined.th.png
   chmod 775 combined.png combined.th.png
   echo ""
   echo "Results:  combined.png (large scale) and combined.th.png (thumbnail for the web summaries)"
   echo ""

   if [ $log != "NONE" ]
   then
      echo "Results:  combined.png (large scale) and combined.th.png (thumbnail for the web summaries)" >> $log
   fi

else
   echo ""
   echo "No thumbnail (*.th.png) files were found to combine."
   echo ""

   if [ $log != "NONE" ]
   then
      echo "No thumbnail (*.th.png) files were found to combine." >> $log
   fi

fi

exit 0
