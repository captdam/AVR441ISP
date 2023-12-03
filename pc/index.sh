#!/bin/bash

FILE_HTML=./index.html
FILE_HTTP=./index.http
FILE_HEAD=./index.h

if ! [ -r $FILE_HTML ]; then
	echo "Cannot read HTML file " $FILE_HTML
	exit 1
fi

echo "Read HTML file " $FILE_HTTP
size=$(cat $FILE_HTML | wc -c)
echo "HTML file " $FILE_HTML " size: " ${size}

echo "Write HTTP header to " $FILE_HTTP
printf "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: ${size}\r\n\r\n" > $FILE_HTTP

echo "Write HTML payload to " $FILE_HTTP
cat $FILE_HTML >> $FILE_HTTP
printf "\0" >> $FILE_HTTP

echo "Generate header file for C include to " $FILE_HTTP
xxd -i $FILE_HTTP > $FILE_HEAD

echo "Done!"