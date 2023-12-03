# ISP PC software

This software will open a TCP server to interactive with user and a TTY (UART) connection to interactive with ISP.

## Server

Compile with ```gcc *.c -O2 -o isp```.

Execute with ```./isp tty port``` where ```tty``` is the UART device and ```port``` is the port number to open server. Example: ```./isp /dev/ttyUSB0 10086```

Note: Make the client http data before compile the server code. This program will store client html (and http header) in the program's code segment to minimize file operation.

## Client

Source code in ```index.html```. This file contains html GUI, JS to interactive with server (XHR) and JSON to store AVR configs.

To make tyhe client, first ```cd``` to this directory, then execute ```index.sh```. This script will create http header using template and the size of the html file, then combine the http header and html body into http message ```index.http```, finally it will create a header file so it can be include into the server program.

To open the client, first start the server with command ```./isp tty port```, then use any browser (test with Firefox) and go to ```localhost:port``` where ```port``` is same as the port in ```./isp tty port``` (example: ```localhost:10086```). Since we fetch the webpage from the webserver, there is no CORS issue.