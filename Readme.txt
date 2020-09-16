To compile the files, run the following commands on terminal:
	g++ server.cpp -o server
	g++ client.cpp -o client

To run the program, follow the following steps:
	1. ./server <port no> 
	2. run another terminal for running a client 1.
	3. ./client 127.0.0.1 <port no>  on this new terminal.
	4. repeat 2 and 3 for new clients concurrent clients.

Some points regarding running this client-server program.
1. After establishing a tdp connection, client may send as many msgs as it wants. To get the udp connection it has to send "getudp" to server.
2. To end this connection , client must send "quitudp" as message to server, and then server will close this connection and free the udp port.
