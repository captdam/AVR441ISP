# ISP PC software

This software will open a TCP server to interactive with user and a TTY (UART) connection to interactive with ISP.

## Client-side

To compile this software, first build the client webpage using ```sh index.sh```. This will generate a HTTP response body using the client-side program in HTML file ```isp.html```.

This single HTML file contains everything we need for the client, including JSON to store AVR configuration, HTML and CSS to create UI, and JS code for the underlying logic. Dividing them into different files (like we do in web server) makes the server complicated, so we will have everything in the signle file.

The client can be accessed using web browser.

## Server

Compile with ```gcc *.c -O2 -o isp```. This will compile the server logic and embed the client program into the server. When the client is requesting the client-side software, the server can fetch the embedded text from memory instead of read the client-side program from disk.

The server-side program does not control the programming logic. In fact, it only translates data between HTTP and UART communication (and some communication error check).

## Execute

First, connect the ISP to PC.

Execute with ```./isp tty port``` where ```tty``` is the UART device and ```port``` is the port number to open server. Example: ```./isp /dev/ttyUSB0 10086```.

Use web browser (I use Firefox for dev) and go to ```localhost:port``` where ```port``` is same as the previous step. Example: ```localhost:10086```. You may open page inspector to see some logs in console. This will send a ```GET``` request to the server to get the client-side program from the server. A blank page then displayed in the web browser, the server-side program also says "Request index page".

Click any button on this page will send ```PUT``` request to the server. The server will translate the request from HTTP to UART and send it to the ISP. After ISP response, the data from ISP in UART will be translated into HTTP response body and returned back to the client.

The first step is to use "SPI Get" to enable programming and read signature bytes from the target AVR. If the signature matches with a known on in the configuration JSON, this program will display a table of the NVME ofthe target AVR. Use "X READ" to read each of them (example, read hfuse), the program will read the specific data from the target AVR and display them in the table.

To porperly close the program, use the "Halt". This will send a ```DELETE``` request which request the server to exit. After this operation, you must restart the server-side program and refresh the client-side page to use this software again.