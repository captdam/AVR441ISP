/** Start the HTTP server. 
 * @param port TCP port to host the server
 * @return 0 if success, errno if failed
*/
int http_init(int port);

/** Receive a HTTP request. Only the first line will be read and remaining data will be ignored. 
 * @param method Request method, NULL if cannot detect method, in most case request too long (note: buffer shared with data)
 * @param data Request data, NULL if cannot detect data, in most case request too long (note: buffer shared with method)
 * @return Transaction handler, return -1 if failed
*/
int http_receive(char** method, char** data);

/** Send HTTP data. The function will create HTTP header for the given data. 
 * @param transaction Transaction handler
 * @param code HTTP code to send, example "200 OK"
 * @param data Data to send
 * @param len Length of data (HTTP content length)
*/
void http_send(int transaction, char* code, void* data, int len);

/** Close the HTTP server (if opened). 
*/
void http_close();