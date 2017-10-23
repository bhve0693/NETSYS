A HTTP-BASED WEB SERVER TO HANDLE MULTIPLE SIMULTANEOUS REQUESTS FROM USERS-10/22/17
Author: Bhallaji Venkatesan

Files and Paths:
---------------
Within Folder PA2:

1. www Folder: Document root
	-> index.html
	-> jquery-1.4.3.min.js
	-> Other folders-css, fancybox, files, graphics, images - referenced from sample ws.conf file provided in ECEN5273-CS Moodle
2. Makefile
3. webserver.c and webserver executable
4. ws.conf - referenced from sample ws.conf file provided in ECEN5273-CS Moodle


General Usage Notes:
-------------------

1. Run make in the PA2 folder
2. Configure the webserver settings in ws.conf - port number, document root, default webpages and different contents the server handles
3. To start the webserver, run ./webserver 
4. Press CTRL+C to gracefully exit the program by closing the socket 

Code Brief on Mandatory Requirements:
------------------------------------

1. webserver.c
	-> Implemented handlingg of multiple client requests using multiprocess creation (fork())
	-> Implemented handling of GET request for all the content types in ws.conf
	-> Error checking for invalid HTTP Requests as mentioned in error handling 

	
Error Handling:
--------------

	1. Status Code "400" - This error takes care of situations when invalid method or invalid HTTP version is provided in the request
	2. Status Code "404" - Not found. When file requested does not exist
	3. Status Code "500" - Internal server error. When any error such as malloc failure occurs 
	4. Status Code "501" - Not implemented error. When requests other than GET or POST is made.
	5. ws.conf	     - Care is also taken that the web server does not start and program exits when ws.conf is not found
	6. Port Error	     - If port number lies in the reserved region of 1024, the server does not start in such a case


Code Brief on Extra Credits:
---------------------------
POST Method Support: 
	The code written handles POST Requests for .html files. It displays the POST Data in the server's response along with requested URL contents.

