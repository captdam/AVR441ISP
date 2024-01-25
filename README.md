This branch is for reference only! OUTDATED!

A quick note here.

The first design used Tiny25 (the smallest and cheapest AVR in DIP package) in raw mode. There is no logic in the AVR, it did nothing but translate signal from UART (from PC) to SPI (to target), then translate signal from SPI (from target) to UART (to PC). In another word, the PC was resbonsible for handling the SPI protocol.

Tiny25 only has one USI interface and no UART interface. The UART communication between ISP and PC must be done in software, the SPI communication between the ISP and the traget must be done in semi-software style.

It first seems reasonable to use scheme. <del>PC is powerful enough to handle the protocol control, programming process does not require high speed. Atleast, this is true for hobby purpose.</del>

Then, there is the problem:

First, the speed.

According to the AVR SPI protocol, sending 1 data bytes requires 4 SPI byte transaction between the ISP and the target. Furturemore, because the PC handles the SPI protoocol, the PC needs to send 4 bytes to ISP before the actual SPI transaction between the ISP and the target; after the SPI transaction, the ISP replys the 4 bytes back to PC in UART.

In another word, 8 UART bytes for 1 data byte. SPI communication is fast, not a big issue; however, the UART is a big hit.

For most PC, 115200 BAUD is the maximum. If we use 1 start bit, 8 data bit and 1 stop bit, 1 margin bit (in case clock on ISP is too slow), that will be 10472 UART byte every second. 8 UART bytes for 1 data byte, plus some waiting time for SPI communication and internal processing, that is 1000 data byte every second. Not mention the time for PC context switching when performing syscall (send UART, read UART).

During experiment, the throughput is less than 80 data byte every second.

Second, error rate.

At the beginning, server-client method is used. The client runs in web browser controls the display and the protocol, data to send and to receive is packed into HTTP request and send to a local server. The local server uses termios to access the UART interface. This method works without any issue, except it is not supportted on Windows. WSL can be used, but sometimes the server-side in WSL freezed.

To simplfy the process and elimilate the server side, Web serial API is used. Maybe this only applies to my PC, I some times lost receiver data. More specificly, I randomly lost the first byte in each 4-byte package.

The solution:

Raw mode is no longer an optimized solution. The PC should pack multiple data bytes into a packet to ISP. The ISP then decode the packet, perform error checking of the packet, then send the decoded data to target.

This method should increase data density, from 1/8 in raw mode, to X/X+N where X is packet layload and N is header length. This method should also decrease error rate, since less packet will be sent.

Tiny25 is also abandoned. Another chip, the Tiny441/841 provides 2 hardware UART interface and one SPI interface. It allows faster processing, higher reliability than software UART/SPI, and less strict timing. This chip is not in DIP, but SOP package can be used.
