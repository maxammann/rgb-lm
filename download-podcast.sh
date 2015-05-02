#!/bin/bash

wget -O podcasts.xml http://www.deutschlandfunk.de/podcast-nachrichten.1257.de.podcast.xml 

urls=`grep -o 'http:\/\/[^"]*\.mp3' podcasts.xml | sort -u`


trap "exit" INT
for url in $urls; do
	echo "$url";
	ffmpeg -y -loglevel warning -i $url "${url##*/}.ogg" >/dev/null 2>&1;
done
