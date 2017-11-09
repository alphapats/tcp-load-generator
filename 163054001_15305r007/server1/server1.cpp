/*
FILE NAME: server1.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files creates a central server. Server takes one commandline input arguments ie own listning port.
Clients will use Server IP and port to connect this server for fetching files.

*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>
#include <thread>
#include <cstdlib>
#include <string>
#include <fcntl.h> 
#include <unistd.h> 
#include <future>
using namespace std;

#define ERROR     -1  // defines error message
#define MAX_BUFFER   1024 //used to set max size of buffer for send recieve data 
#define THREADSTACK  65536
// Defining Constant Variables for database connections
//#define AUTH_SERVER_IP "127.0.0.1"
//#define AUTH_SERVER_PORT 20000

char AUTH_SERVER_PORT[MAX_BUFFER]; // authentication server port
char AUTH_SERVER_IP[MAX_BUFFER]; // auth server IP address


time_t current_time; // variable to store current time

void *client_handler(void *);


int connect_to_auth_server(char*,char*,char*); // function to connect to authentication server
//string get_event(string,string);
int upload_file(int , char*);
void get_file_name(int, char*);
struct request {
	char uname[50],upassword[20],type[10];
};



int main(int argc, char **argv)  
{
	int sock; // sock is socket desriptor for server 
	int new1; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	socklen_t sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	int *new_sock;
	 
	int rc,c;

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no listening port provided\n");
		exit(1);
	}
	

	printf("Enter Authentication server IP   ");
	scanf(" %[^\t\n]s",AUTH_SERVER_IP);

	printf("Enter Authentication server listening port  ");
	scanf(" %[^\t\n]s",AUTH_SERVER_PORT);


	/*get socket descriptor */
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{ 
		perror("server socket error: ");  // error checking the socket
		exit(-1);  
	} 
	 
	/*server structure */ 
	server.sin_family = AF_INET; // protocol family
	server.sin_port =htons(atoi(argv[1])); // Port No and htons to convert from host to network byte order. atoi to convert asci to integer
	server.sin_addr.s_addr = INADDR_ANY;//INADDR_ANY means server will bind to all netwrok interfaces on machine for given port no
	bzero(&server.sin_zero, 8); //padding zeros


	// set SO_REUSEADDR on a socket to true (1):
	// kill "Address already in use" error message
	int opt;
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int)) == -1) {
    perror("setsockopt");
    exit(1);
	}
	
	
	/*binding the socket */
	if((bind(sock, (struct sockaddr *)&server, sockaddr_len)) == ERROR) //pointer casted to sockaddr*
	{
		perror("bind");
		exit(-1);
	}

	printf("SERVER STARTED...... \n ");
	/*listen the incoming connections */
	if((listen(sock, 60000)) == ERROR) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	c = sizeof(struct sockaddr_in);

	while((new1 = accept(sock, (struct sockaddr *)&client, (socklen_t*)&c)))
	{
		//printf("New client connection accepted \n");
		new_sock = new int(1);
        *new_sock = new1;

		pthread_t sniffer_thread;
		pthread_attr_t attr;
		// Initialize 
	   rc=pthread_attr_init(&attr);
	   if (rc) {
	         perror("Error:unable to initilaise thread attribute" );
	         exit(-1);
	      }
	   //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	   	rc = pthread_attr_setstacksize(&attr, THREADSTACK);
	    if (rc != 0)
	        {
	        	perror("pthread_attr_setstacksize");
	        } 

        
         
        if( pthread_create( &sniffer_thread , &attr ,  client_handler , (void*) new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
         
        //Now join the thread , so that we dont terminate before the thread
        pthread_join( sniffer_thread , NULL);
       // printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
	 	
       // printf("Handler assigned to new client");
         rc=pthread_attr_destroy(&attr);
		   if (rc != 0)
		   {
		   	    perror("pthread_attr_destroy");
		   }
           

    }

    if ((new1 < 0)) // accept takes pointer to variable containing len of struct
		{
			perror("ACCEPT.Error accepting new connection");
			exit(-1);
		}

	

     return 0;
   }


/*
 * This will handle connection for each client
 * 
 */
void *client_handler(void *socket_cl)
{
    //Get the socket descriptor
    int new1 = *(int*)socket_cl;
    char *temp;
    struct request newreq;

    //variables for add new user operation
	char buffer[MAX_BUFFER]; // Receiver buffer; 
	//char file_name[MAX_BUFFER];//Buffer to store filename,path and port recieved from client
	//char *client_ip;//variable to store IP address of client
	
	int success=0;

	//variable for authenticating existinguser operation
	char user_name[MAX_BUFFER];
	char user_key[MAX_BUFFER];
	char file_name[MAX_BUFFER];//file keyword for search by user
	int len;// variable to measure MAX_BUFFER of incoming stream from user
	
	pid_t child_pid;// to manage child process

     
    struct sockaddr_in client;
    socklen_t sin_size = sizeof(client);
    int res = getpeername(new1, (struct sockaddr *)&client, &sin_size);
    char *client_ip ;
    client_ip=inet_ntoa(client.sin_addr);
    //printf(client_ip);
    //strcpy(client_ip, inet_ntoa(client.sin_addr));
		
	
	 	
		while(1)
		{
		len=recv(new1, buffer , MAX_BUFFER, 0);
		buffer[len] = '\0';
		//printf("%s\n",buffer);

		//conenctionerror checking
			if(len<=0)//connection closed by client or error
				{
				if(len==0)//connection closed
				{
					printf("client %s hung up\n",inet_ntoa(client.sin_addr));
					
					return 0;
					
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
					return 0;
				}
			
			}//clos if loop



			//ADD NEW USER OPERATION
		if(buffer[0]=='n' && buffer[1]=='e' && buffer[2]=='w') // check if user wants to publish a file
		{

			strcpy(newreq.type, "new");
			int i,j;
			char str[512];
			j = 0;	
			for( i = 4;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
			buffer[j]=0;
			strcpy(newreq.uname, str);
			//cout<<newreq.uname;

			for( i = j;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
			buffer[j]=0;
			
			strcpy(newreq.upassword, str);
			//cout<<newreq.upassword;
		//char Report2[] = "Password recieved"; 
		//send(new1,Report2,sizeof(Report2),0);	

		//bzero(buffer,MAX_BUFFER);

		//strcpy(new_user.uname, login_name);
        //strcpy(new_user.upassword, login_password);
        
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//INSERT CODE TO ADD USERS TO DATABASE
        temp ="new";
        success = connect_to_auth_server(newreq.uname,newreq.upassword,temp);



        //cout<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if (success)
		{
		char Report3[] = "USER ADDED   "; 
		send(new1,Report3,sizeof(Report3),0);	
		//printf("%s\n",Report3);
		
		}
		else
		{
		char Report3[] = "ADDING USER FAILED.TRY ANOTHER USERNAME   "; 
		send(new1,Report3,sizeof(Report3),0);
		//printf("%s\n",Report3);	
		
		}
		//printf("Closing connection\n");
	    //printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 	inet_ntoa(client.sin_addr));
			client_ip = inet_ntoa(client.sin_addr); // return the IP
			
		close(new1); /* close connected socket*/
	    return 0;

		}


		

		//AUTHENTICATE USER AND GET EVENTS
		else if(buffer[0]=='g' && buffer[1]=='e' && buffer[2]=='t') //check keyword for search sent by client
		{
			strcpy(newreq.type, "get");
			int i,j;
			char str[512];
			j = 0;	
			for( i = 4;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
			buffer[j]=0;
			strcpy(newreq.uname, str);
			//cout<<newreq.uname<<endl;

			for( i = j;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
			buffer[j]=0;
			
			strcpy(newreq.upassword, str);
			//cout<<newreq.upassword;

			/*
			char ack[]="Request_received";
			send(new1, ack, sizeof(ack), 0); // recieve confirmation message from server
			success=0;
			bzero(buffer,MAX_BUFFER); // clearing the buffer by padding
			*/

				
			//printf("%s\n",file_name);

			///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
			temp="aut"; // keyword to be send to server so that server knows it is a publish operation

			//	thread t1(connect_to_auth_server,user_name,user_key,temp );
			// auto future = std::async(connect_to_auth_server,user_name,user_key,temp );
			//success = future.get(); 
			success = connect_to_auth_server(newreq.uname,newreq.upassword,temp);
			//	t1.join();
			

			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
		if (success)
			{
				char Report3[] = "Successfull login  "; 
				send(new1,Report3,sizeof(Report3),0);
				//strcat(Report3,user_name);
					

				get_file_name(new1, file_name);	
			    upload_file(new1, file_name);
			    		 

			}
		else
		{
			char Report3[] = "Unsuccessfull login.Try again   ";
			send(new1,Report3,sizeof(Report3),0);
			

		}
		
		
		
	   // printf("Closing connection\n");
	   // printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	//inet_ntoa(client.sin_addr));
			//client_ip = inet_ntoa(client.sin_addr); // return the IP
			
		close(new1); /* close connected socket*/
	    return 0;
		}// close get condition


		
		
			//TERMINATE OPERATION:when user want to disconnect from server
		else if(buffer[0]=='t' && buffer[1]=='e' && buffer[2]=='r')
		{

			char ack[]="Request_received";
			send(new1, ack, sizeof(ack), 0); // recieve confirmation message from server
			//printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	//inet_ntoa(client.sin_addr));
			client_ip = inet_ntoa(client.sin_addr); // return the IP
			
			//printf("Closing connection\n");
			close(new1); /* close connected socket*/
        
	    
			return 0;
			
		} //close terminate loop
	
	}// close while loop inside fork.server will keep listening client till disconnected
	
	
       close(new1);   
    //Free the socket pointer
    free(socket_cl);
     
    return 0;
}





int upload_file(int sock, char *file_name)
{

	ssize_t read_bytes, sent_bytes, sent_file_size; // to keep count of total data being sent
	int sent_counter; // to count number of packets transfered

	char send_buf[MAX_BUFFER]; //buffer to store the data read from file
	//char * errmsg_notfound = "File not found\n";
	int fp; //file descriptor

	sent_counter = 0;
	sent_file_size = 0;


	char file_name_full[MAX_BUFFER];
	strcpy(file_name_full,"uploads/");
	strcat(file_name_full,file_name);


	if( (fp = open(file_name_full, O_RDONLY)) < 0) 
	{
		
		perror(file_name_full );

	}
	else 
	{
		//printf("Sending file: %s\n", file_name_full);
		while( (read_bytes = read(fp, send_buf, MAX_BUFFER)) > 0 )
		{
			if( (sent_bytes = send(sock, send_buf, read_bytes, 0))< read_bytes )
			{
				perror("Error in send");
				return -1;
			}
		sent_counter++;
		sent_file_size += sent_bytes;
		}
	close(fp);
	//printf("File sent to client: %s\n", file_name_full);
	} //end loop
	
/*
	sync();

    std::ofstream ofs("/proc/sys/vm/drop_caches");
    ofs << "3" << std::endl;
*/
	//cout<<file_name << "sent "<<endl;
	//printf("Sent %zu (bytes) in %d send(s)\n\n",sent_file_size, sent_counter);
	return sent_counter;
}





int connect_to_auth_server(char* user_name, char* user_key, char* keyword)
{

	int auth_sock; // auth_sock is socket desriptor for connecting to remote server 
	struct sockaddr_in auth_server; // contains IP and port no of remote server
	char send_buff[MAX_BUFFER];  //user input stored 
	char receive_buff[MAX_BUFFER]; //recd from remote server
	int len;//to measure length of recieved input stram on TCP
	char *temp = keyword; // variable to store temporary values
	//char const *username1 =username.c_str();
	//char const *password1 = password.c_str();
	char* username1=user_name;
	char *password1=user_key;
	
	//printf("%s","Inside authenticating funtion");
	//for connecting with server for publishing and search files
	if ((auth_sock= socket(AF_INET, SOCK_STREAM, 0)) <0)
	{ 
		perror("Problem in creating socket");  // error checking the socket
		exit(-1);  
	} 
	  
	memset(&auth_server,0,sizeof(auth_server));
	auth_server.sin_family = AF_INET; // family
	auth_server.sin_port =htons(atoi(AUTH_SERVER_PORT)); // Port No and htons to convert from host to network byte order. 
	auth_server.sin_addr.s_addr = inet_addr((AUTH_SERVER_IP));//IP addr in ACSI form to network byte order converted using inet
	bzero(&auth_server.sin_zero, 8); //padding zeros
	

	if((connect(auth_sock, (struct sockaddr *)&auth_server,sizeof(auth_server)))  == ERROR) //pointer casted to sockaddr*
	{
		perror("Problem in connect");
		exit(-1);
	}
	//printf("%s","\n Connected to Authentication server\t \n");
	strcpy(send_buff, temp);
	strcat(send_buff,":");
	strcat(send_buff,user_name);
	strcat(send_buff,":");
	strcat(send_buff,user_key);

	send(auth_sock, send_buff, sizeof(send_buff) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	//printf("%s\n" , receive_buff); // display confirmation message
	//bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros


	//printf("Sending username and password...");
	/*
	send(auth_sock, username1, sizeof(username1) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	//printf("%s\n" , receive_buff); // display confirmation message
	bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros

	send(auth_sock, password1, sizeof(password1) ,0); // send input to server
	len = recv(auth_sock, receive_buff, MAX_BUFFER, 0); // recieve confirmation message from server
	receive_buff[len] = '\0';
	//printf("%s\n" , receive_buff); // display confirmation message
	*/
	//printf("Connection terminated with AUTHENTICATION server.\n");
	close(auth_sock);
	//cout<<numrows<<endl;
    if(receive_buff[0]=='S' && receive_buff[1]=='u' && receive_buff[2]=='c') 

       {
       	      	return 1;
       }
    else
      {
      	      	return 0;
      }

     // bzero(receive_buff,MAX_BUFFER); // pad MAX_BUFFER with zeros
}

void get_file_name(int sock, char* file_name)
{

	char receive_buff[MAX_BUFFER]; /* to store received string */
	ssize_t len; /* bytes received from socket */
	/* read name of requested file from socket */
	if ( (len = recv(sock, receive_buff, MAX_BUFFER, 0)) < 0) {
	perror("Error in recieve");
	}
	sscanf (receive_buff, "%s\n", file_name); /* discard CR/LF */

}