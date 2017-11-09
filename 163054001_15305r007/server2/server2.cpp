/*
FILE NAME: server2.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files creates a authentication server. Server takes one commandline input arguments ie own listning port.
Server1 will use Server IP and port to connect this server for authentication users.

*/
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>

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
#include <cstdlib>
#include <string>
#include <fcntl.h>
#include <unistd.h> 

#include <sys/wait.h> 


// For MySQL Connection
#include <mysql/mysql.h>
using namespace std;

#define ERROR     -1  // defines error message

#define MAX_BUFFER   1024 //used to set max size of buffer for send recieve data 

// Defining Constant Variables for database connections
#define SERVER "localhost"
#define USER "root"
#define PASSWORD "root"
#define DATABASE "ems_data"



time_t current_time; // variable to store current time

//funtions for database handling
void get_user_details(int,char*, char*);
int add_newuser(string, string);
int authenticate_user(string, string);
int fork (void);
void sig_chld(int);


struct user
{
    char uname[MAX_BUFFER];
    char upassword[MAX_BUFFER];
};


int main(int argc, char **argv)  
{
	int sock; // sock is socket desriptor for server 
	int new1; // socket descriptor for new client
	struct sockaddr_in server; //server structure 
	struct sockaddr_in client; //structure for server to bind to particular machine
	socklen_t sockaddr_len=sizeof (struct sockaddr_in);	//stores length of socket address
	int *new_sock;

	char buffer[MAX_BUFFER]; // Receiver buffer; 

	int success=0;

	//variable for authenticating existinguser operation
	char user_name[MAX_BUFFER];
	char user_password[MAX_BUFFER];
	int len;// variable to measure MAX_BUFFER of incoming stream from user
	
     
	pid_t child_pid;// to manage child process

	if (argc < 2)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, no port provided\n");
		exit(1);
	}


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

	printf ("SERVER STARTED........................\n\n");
	/*listen the incoming connections */
	if((listen(sock, 60000)) == ERROR) // listen for max connections
	{
		perror("listen");
		exit(-1);
	}

	signal (SIGCHLD, sig_chld);

	while(1)
	{
	
	
	if ( (new1 = accept(sock, (struct sockaddr *)&client, &sockaddr_len)) < 0 )
	{
		perror("accept error");
		break;
	}

	socklen_t sin_size = sizeof(client);
    int res = getpeername(new1, (struct sockaddr *)&client, &sin_size);
    char *client_ip ;
    client_ip=inet_ntoa(client.sin_addr);

	if ( (child_pid = fork()) == 0 ) 
	{
		close (sock);
		
		//printf("New client connected from port no %d and IP %s\n", ntohs(client.sin_port),inet_ntoa(client.sin_addr));
		client_ip = inet_ntoa(client.sin_addr); // return the IP
		
		
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
					printf("Peer %s hung up\n",inet_ntoa(client.sin_addr));
				}
				else //error
				{
					perror("ERROR IN RECIEVE");
				}
			close(new1);//closing this connection
			exit (0);
				}

			//ADD NEW USER OPERATION
		if(buffer[0]=='n' && buffer[1]=='e' && buffer[2]=='w') // check if user wants to publish a file
		{
	

		success=0;
		//strcpy(newreq.type, "new");
		int i,j;
		char str[512];
		bzero(str,512);
		j = 0;	
		for( i = 4;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
		buffer[j]=0;
		strcpy(user_name, str);
			//cout<<newreq.uname;

		for( i = j;	buffer[i] != ':' ; i++ )
			str[j++]=buffer[i];
		buffer[j]=0;
			
		strcpy(user_password, str);

		
		//get_user_details(new1, user_name, user_password);
		//cout<<user_name<<user_password;
        
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//INSERT CODE TO ADD USERS TO DATABASE

        success=add_newuser(user_name,user_password);
        //cout<<"Success "<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if (success)
		{
		char Report3[] = "Success. User added  "; 
		send(new1,Report3,sizeof(Report3),0);	
		
	
		}
		else
		{
		char Report3[] = "Unsuccessfull. ADDING USER FAILED.TRY ANOTHER USERNAME   "; 
		send(new1,Report3,sizeof(Report3),0);
		//printf("%s\n",Report3);	
		
		}
		//printf("Closing connection\n");
	   // printf("Client disconnected from port no %d and IP %s\n", ntohs(client.sin_port), 
	//inet_ntoa(client.sin_addr));
		client_ip = inet_ntoa(client.sin_addr); // return the IP
		
		//printf("Closing connection");
		close(new1); /* close connected socket*/
		exit(0);
	   
		}


		

		//AUTHENTICATE USER AND GET EVENTS
		else if(buffer[0]=='a' && buffer[1]=='u' && buffer[2]=='t') //check keyword 
		{
		success=0;
		int i,j;
		char str[512];
		bzero(str,512);
		j = 0;	
		for( i = 4;	buffer[i] != ':' ; i++ )
				str[j++]=buffer[i];
		buffer[j]=0;
		strcpy(user_name, str);
			//cout<<newreq.uname;

		for( i = j;	buffer[i] != ':' ; i++ )
			str[j++]=buffer[i];
		buffer[j]=0;
			
		strcpy(user_password, str);

		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		
		//INSERT CODE TO AUTHENTICATE USER FROM DATABASE
		success=authenticate_user(user_name,user_password);
		//cout<<"Success "<<success<<endl;

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if (success)
			{
				char Report3[] = "Successfull login  "; 
				send(new1,Report3,sizeof(Report3),0);
				
			}
		else
		{
			char Report3[] = "Unsuccessfull login.Try again   ";
			send(new1,Report3,sizeof(Report3),0);
			cout<<Report3;
		}
		
		//printf("Closing connection\n");
		close(new1); /* close connected socket*/
		exit(0); /* exit child process */

		}
		
	} /* CLOSE WHILE LOOP1*/
	} // end if loop
	close(new1); // close parent's copy
	} //end while loop 
	close(sock); // close listening socket
	return 0;
	
   }







int add_newuser(string username,string password)
{
	MYSQL *connect;
    connect = mysql_init(NULL);

    if (!connect)
    {
        cout << "Mysql Initialization Failed";
        return 0;
    }

    connect = mysql_real_connect(connect, SERVER, USER, PASSWORD, DATABASE, 0,NULL,0);

    if (connect)
    {
       // cout << "Connection Succeeded\n";
    }
    else
    {
        cout << "Connection Failed...Unable to connect to Mysql DATABASE\n";
        return 0; 
    }

    MYSQL_RES *res_set;
    MYSQL_ROW row;
    //------------------------------------------------------------------------------------------------------
    /*Signing up at the server*/

    
    string query="INSERT INTO user(user_name,passwd) values('"+username+"','"+password+"');";

    if(mysql_query(connect,query.c_str()))
    {
    	mysql_close (connect);
    	return 0;
    }
    else
	{
		mysql_close (connect);
		return 1;
	}

    
}

int authenticate_user(string username,string password)
{
	MYSQL *connect;
    connect = mysql_init(NULL);

    if (!connect)
    {
        cout << "Mysql Initialization Failed";
        return 0;
    }

    connect = mysql_real_connect(connect, SERVER, USER, PASSWORD, DATABASE, 0,NULL,0);

    if (connect)
    {
       // cout << "Connection Succeeded\n";

    }
    else
    {
        cout << "Connection Failed...Unable to connect to Mysql DATABASE\n";
        return 0; 
    }

    MYSQL_RES *res_set;
    MYSQL_ROW row;
    //------------------------------------------------------------------------------------------------------
    /*Login in  at the server*/

        
    string query1="select * from user where user_name='"+username+"' and passwd='"+password+"'";

    if(mysql_query(connect,query1.c_str()))
    	{
        cout<<"query not executed"<<endl;
    	return 0;
		}

    res_set=mysql_store_result(connect);
    unsigned int numrows = mysql_num_rows(res_set);

    mysql_free_result(res_set);
    //cout<<numrows<<endl;
    if(numrows==1)
       {
       	mysql_close (connect);
       	return 1;
       }
    else
      {
      	mysql_close (connect);
      	return 0;
      }

    //----------------------------------------------------------------------------------------------------------
    
}




void get_user_details(int new_sock, char* user_name , char* user_password)
{
char recv_buff[MAX_BUFFER]; /* to store received string */
ssize_t len; /* bytes received from socket */
bzero(recv_buff,MAX_BUFFER); // clearing the buffer by padding

char ack[]="ACK";
send(new_sock, ack, sizeof(ack), 0); // recieve confirmation message from server

/*


/* read buffer */
if ( (len = recv(new_sock, recv_buff, MAX_BUFFER, 0)) < 0) {
perror("recv error");
return;
}
sscanf (recv_buff, "%s\n", user_name); /* discard CR/LF */


char Report1[] = "username recieved";
send(new_sock,Report1,sizeof(Report1),0);
user_name[len] = '\0';
//printf("%s\n",user_name);
bzero(recv_buff,MAX_BUFFER); // clearing the buffer by padding


if ( (len = recv(new_sock, recv_buff, MAX_BUFFER, 0)) < 0) {
perror("recv error");

return;
}
sscanf (recv_buff, "%s\n", user_password); /* discard CR/LF */
user_password[len] = '\0';
//printf("%s\n",user_password);
bzero(recv_buff,MAX_BUFFER); // clearing the buffer by padding


}


void sig_chld(int signo)
{
pid_t pid;
int stat;
while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0)
//printf("child %d terminated\n", pid);
return;
}