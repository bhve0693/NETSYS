FILE TRANSFER BETWEEN SERVER AND CLIENT USING UDP SOCKETS- 9/24/17

Files and Paths:
---------------
Withing Folder PA1_NETSYS_Bhallaji:

1. Client Folder: 
	udp_client.c
	Makefile
2. Server Folder:
	udp_server.c
	Makefile

General Usage Notes:
-------------------

1. Run make in the Client and Server Folder to generate the executables 'client' and 'server' in the respective folders
2. To start the client application, run the following command in one terminal:
	./client [server IP] [port-number]
3. The following options get displayed on running the 'client' executable':
	1. get [file_name]
	2. put [file_name]
	3. delete [file_name]
	4. ls
	5. exit
4. In another terminal, run the following command:
	./server [port-number]
5. Now, choose an option on client terminal.
6. On choosing an option at the client end, server responds to the client with appropriate messages. 
7. Run exit to complete execution

Code Brief on Mandatory Requirements:
------------------------------------

1. udp_client.c
	The code provides support to execute the following commands:
	1. get    : To get a file from the server
	2. put    : To push a file to a server
	3. delete : To delete a specific file
	4. ls	  : To list out the files
	5. exit   : To terminate the server gracefully
	
   	Care is taken for each invalid command at the client end wherein the server simply repeats the command back to the client with no 		modification, states that the given command was not understood
	
	In addition to this, a Stop and Wait based Reliability mechanism is also implemented. A second assurance of reliability is performed
	using a timeout based retransmission facilitated with the macro: FRAME_KIND_ERR_RETRAS
		

2. udp_server.c
	The code handles the following commands from the client:
	1. get    : The server transmits the requested file to the client 
	2. put    : The server receives the transmitted file by the client and stores it locally 
	3. delete : The server delete file if it exists. Otherwise nothing is done
	4. ls     : Searches all the filesin its local directory and send a list of all these to the client. 
	5. exit   : Exits gracefully
	

Code Brief on Extra Credits:
---------------------------
1. udp_client.c
	An Encryption based on XOR Mechanism using a secret XOR mask private key is implemented

2. udp_server.c
	A Decryption based on XOR Mechanism using a secret XOR mask private key is implemented
