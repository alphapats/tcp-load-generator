/*
FILE NAME: client.cpp
CREATED BY:163054001 AMIT PATHANIA
			15305r007 Nivia Jatain
This files create a peer. File takes two commandline input arguments server-ip and server port.
Server IP and port will be used to connect the server and publish and subscribe events.
*/
#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <thread>
#include <chrono>  // for high_resolution_clock
#include <iomanip>      // std::setprecision
#include <ctime>
#include <vector>
using namespace std;

#define ERROR     -1			//returns -1 on error as message
#define MAX_BUFFER    1024      //this is max size of input and output MAX_BUFFER used to store send and recieve data
#define THREADSTACK  65536

#define username "amit"
#define password "amit"


const char alphabet[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
int NUM_THREADS; //number of threads
double RUN_TIME;
char* SERVER_PORT ; // server port
char* SERVER_IP ;// server IP address
char* REQUEST_TYPE;// type of request

//auto start = std::chrono::high_resolution_clock::now();
std::chrono::high_resolution_clock::time_point t_start ;


char file_name[MAX_BUFFER];



struct request {
	char uname[50],upassword[20],type[10];
};

struct result {
	long  comp;
	long  err;
	long login_unsuc;
	double data_tfred;
	double total_resp_time;
	
};

double fetch_file(int ,char*); //functionn to recieve file
double connect_to_server(void *,char*,char*);
void *user_work(void*);
char *randomString(int );


int main(int argc, char **argv)  //IP and port mentioned
{
	
	int id,rc;
	int  cnt = 0;
	pthread_attr_t attr;
	fstream result_file;
	result t_ret; 
	void *vptr;
	int error_counter = 0;
	long complete_counter = 0;
	double total_data_tfred=0;
	double total_resp_time=0;
	int login_unsucc_counter=0;
	//client takes two arguements SERVER IP ADDRESS,SERVER PORT 
	if (argc < 4)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, ENTER SERVER IP ADDRESS AND PORT AND REQUEST TYPE \n");
		exit(-1);
	}

	SERVER_PORT=argv[2];
	SERVER_IP=argv[1];//IP addr in ACSI form to network byte order converted using inet
	REQUEST_TYPE=argv[3];

	if (strcmp(REQUEST_TYPE,"new")!=0 && strcmp(REQUEST_TYPE,"get")!=0)    // check whether port number provided or not
	{ 
		fprintf(stderr, "ERROR, ENTER CORRECT REQUEST TYPE : new/get \n");
		exit(-1);
	}


	cout<<"ENTER NUMBER OF THREADS/USERS(N)   :" ;
	cin>>NUM_THREADS;


	cout<<"NUM OF REQUEST PER USER  :" ;
	cin>>RUN_TIME;
	//cout<<"ENTER FILENAME   :" ;
	//cin>>file_name;

	
	pthread_t threads[NUM_THREADS];
	
	
	// Initialize and set thread joinable
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

   
	// Record start time
  
t_start = std::chrono::high_resolution_clock::now();
  


	for(cnt = 0; cnt < NUM_THREADS; cnt++ ) {

      //cout << "Creating thread " << id << endl;
      rc = pthread_create(&threads[cnt], &attr, user_work, (void *)cnt);
      
      if (rc) {
      	cout<<cnt;
         perror("Error:unable to create thread" );
         break;
      }

   }

// free attribute and wait for the other threads
   rc=pthread_attr_destroy(&attr);
   if (rc != 0)
   {
   	    perror("pthread_attr_destroy");
   }
           

//block until all threads complete 
  for (id = 0; id < cnt; id++) {
    rc=pthread_join(threads[id], &vptr);
     if (rc) {
         perror("Error:unable to join thread");
         exit(-1);
      }
    t_ret = *((result *)vptr); 
    complete_counter=complete_counter+t_ret.comp;
    error_counter= error_counter+t_ret.err;
    total_data_tfred=total_data_tfred+t_ret.data_tfred;
    login_unsucc_counter=login_unsucc_counter+t_ret.login_unsuc;
    total_resp_time=total_resp_time+t_ret.total_resp_time;
    
 
  }
  free(vptr); 

   // Record end time
	auto finish = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> elapsed = finish - t_start;
  	

	cout << "Elapsed time: " << elapsed.count()<< " s\n";

	float data_exch=total_data_tfred/complete_counter;
	float mean_resp_time=(total_resp_time)/(complete_counter);
	double throughput=complete_counter/(elapsed.count());
	cout<<"Mean Response Time " <<mean_resp_time<<endl;
	cout<<"Total users in system " <<NUM_THREADS<<endl;
	cout<<"Total connection Errors  " << error_counter<<endl;
	cout<<"Total MYSQL error" <<login_unsucc_counter<<endl;
  	cout<<"Total requests completed  " <<complete_counter<<endl;
  	cout<<"Avg Number of requests completed per second  " << complete_counter/(elapsed.count()) <<endl;
  	cout<<"Total elapsed time(sec) /Num of requests " << elapsed.count()/complete_counter <<endl;
  	cout<<"Avg Bytes transfered per get request (in MB) "<<data_exch<<endl;
  	cout<<"Total data transfered (in MB) " <<total_data_tfred<<endl;
  	cout<<"Throughput(X)  " << throughput <<endl;
  	//cout<<"New Throughput  " << complete_counter*NUM_THREADS/total_resp_time <<endl;
  	  	//open the result file
  	
  	result_file.open ("final_result.txt", ios::out | ios::app );
  	result_file<<endl<<NUM_THREADS<<"\t \t"<<data_exch<<"\t \t"<<mean_resp_time<<"\t \t"<<throughput<<"\t \t"<<complete_counter
  	<<"\t \t"<<error_counter<<"\t \t"<<RUN_TIME<<"\t \t"<<total_data_tfred<<"\t \t"<<elapsed.count()<<"\t \t"<<REQUEST_TYPE<<"\t \t"<<NUM_THREADS/throughput;

  	 result_file.close();


     return EXIT_SUCCESS;
	
	//return (0);
}

double fetch_file(int sock, char* file_name)
{
	ssize_t len, data_received, total_data_received; // variables to store daat recd
	int counter; //to count number of receives
	char send_buff[MAX_BUFFER]; // buffer to store sending msg
	char receive_buff[MAX_BUFFER]; //buffer to store recieved data
	size_t send_bufflen; //length of data sent
	
	int fp; //file descriptor

	char integer_string[32];
	



	//sending file name to server
	sprintf(send_buff, "%s\n", file_name); 
	send_bufflen = strlen(send_buff); 
	if( (len = send(sock, file_name, send_bufflen, 0)) < 0 ) {
		perror("Error sending filename");
		return -1;
	}

	char file_name_full[MAX_BUFFER];
	
	counter = 0; 
	total_data_received = 0; 

	//cout<<"waiting to read..." << file_counter;
	while ( (data_received = recv(sock, receive_buff, MAX_BUFFER, 0)) > 0 )
	{
		if (data_received  == -1){
        perror("RECV ERROR ");
        close(sock);
        return -1;
        //exit(1);
    	}

	counter++;
	total_data_received += data_received;
	
	}

	
	if (total_data_received == 0 )
	{
		cout<<"File not found";
		//remove(file_name_full);
		return -1;
	}
	

//cout<<"Total data received (file size):  " << total_data_received/1000.0 << " (Kbytes) in " <<counter<< " receive(s)"<<"filename"<< file_name<<endl;
return total_data_received/1000.0;
}		
	
double connect_to_server(void *threadid,char* REQUEST_TYPE, char* file_name)
{
	int sock; // sock is socket desriptor for connecting to remote server 
	struct sockaddr_in remote_server; // contains IP and port no of remote server
	char input[MAX_BUFFER];  //user input stored 
	char output[MAX_BUFFER]; //recd from remote server
	int len;//to measure length of recieved input stram on TCP
	
	long tid;
   	tid = (long)threadid;
   	double data_tfred=0;
  	bzero(input,MAX_BUFFER); 
   	
	//for connecting with server for publishing and search files
	if ((sock= socket(AF_INET, SOCK_STREAM, 0)) == ERROR)
	{ 
		perror("socket");  // error checking the socket
		//exit(-1);  
		return -1;
	} 
	  
	remote_server.sin_family = AF_INET; // family
	remote_server.sin_port =htons(atoi(SERVER_PORT)); // Port No and htons to convert from host to network byte order. atoi to convert asci to 		integer
	remote_server.sin_addr.s_addr = inet_addr((SERVER_IP)); 
	bzero(&remote_server.sin_zero, 8); //padding zeros
	
	if((connect(sock, (struct sockaddr *)&remote_server,sizeof(struct sockaddr_in)))  == ERROR) //pointer casted to sockaddr*
	{
		perror("connect");
		//exit(-1);
		return -1;
	}


	request newreq;
	strcpy(newreq.type,REQUEST_TYPE);

	//strcpy(file_name, "parsing.ppt");
	while(1)
	{
				
				strcpy(input, newreq.type);
					
				//temp="get"; // keyword to be send to server so that server knows it is a search operation
				

				
				
				
				if(strcmp(REQUEST_TYPE,"new")==0)
				{
					char *random_wd = randomString(6);
					strcpy(newreq.uname,random_wd);
					strcpy(newreq.upassword,random_wd);
				
					strcat(input,":");
					strcat(input,newreq.uname);
					strcat(input,":");
					strcat(input,newreq.upassword);



					if( (len = send(sock, input, sizeof(input) ,0)) < 0 ) 
					{
					perror("Error in send");
					return -1;
					}


					if ( (len = recv(sock, output, MAX_BUFFER, 0)) < 0) {
						perror("error in receive");
						//exit(0);
						return -1;
						}
					output[len] = '\0';
					//printf("%s\n" , output); // display confirmation message
					

					if(output[0]=='S' && output[1]=='u' && output[2]=='c') 
					{
						bzero(output,MAX_BUFFER); 
						if(close(sock) < 0)
						{
						perror("socket close error");
						return -1;
						//exit(EXIT_FAILURE);
						}
						return 1;
						break;
						
					}
					else
					{
						
						//printf("%s\n" , output);
						bzero(output,MAX_BUFFER); 
						if(close(sock) < 0)
						{
						perror("socket close error");
						return -1;
						//exit(EXIT_FAILURE);
						}
						close(sock);
						return 0;
						break;
					}

				
					

								
				}

				else if (strcmp(REQUEST_TYPE,"get")==0)
				{
					//cout<<REQUEST_TYPE;
					strcpy(newreq.uname,username);
					strcpy(newreq.upassword,password);
					strcat(input,":");
					strcat(input,newreq.uname);
					strcat(input,":");
					strcat(input,newreq.upassword);

					if( (len = send(sock, input, sizeof(input) ,0)) < 0 ) 
					{
						perror("Error in send");
						return -1;
					}


					if ( (len = recv(sock, output, MAX_BUFFER, 0)) < 0) {
							perror("error in receive");
							return -1;
							//exit(0);
							}
					output[len] = '\0';
									
					//cout<<output;
					if(output[0]=='S' && output[1]=='u' && output[2]=='c') 
					{
						bzero(output,MAX_BUFFER); 
						data_tfred=fetch_file(sock, file_name); 

						if(close(sock) < 0)
						{
							perror("socket close error");
							return -1;
							//exit(EXIT_FAILURE);
						}

						//cout<<"Connection terminated with server" <<endl;
						return data_tfred;	
						break;
					
					}
					else
					{	
						printf("%s\n" , output);
						bzero(output,MAX_BUFFER); 
						if(close(sock) < 0)
						{
						perror("socket close error");
						return -1;
						//exit(EXIT_FAILURE);
						}
						return 0;
						break;
						
					}
					

					


				}
				
				else
				{

					if(close(sock) < 0)
					{
						perror("socket close error");
						return -1;
						//exit(EXIT_FAILURE);
					}
					
					break;

				}
				
	} // terminates while loop 
		
   
}	

void *user_work(void *threadid)
{
	//struct result ret;
	result *ret = (result *)malloc(sizeof(result)); 
	std::vector<double> resp_time;  
	float t_duration=0; 
	long  t_complete_counter=0;
	long  t_err_counter=0;
	long t_unsucc_counter=0;
	int flag=0;
	srand((long)threadid);
	char file_name[32];
	char count[32];
	auto th_start = std::chrono::high_resolution_clock::now();
	auto th_end = std::chrono::high_resolution_clock::now();
	//std::chrono::duration<double> elapsed = current_time- t_start;
	//time_t current_time ; 
	int num_req=0;
	double data_tfred=0;
	double total_resp_time=0;
	
	
while(1)
{
	
	
	//elapsed = current_time- t_start;
	//t_duration = difftime(current_time,t_start);

	int r =(long(threadid)*1000) +num_req+1;
	//int r=(long(threadid)* 5) +num_req+1;
	strcpy(file_name,"100KB");
	sprintf(count, "%d", r);
	strcat(file_name,count);
	strcat(file_name,".txt");

	
	//cout<<long(threadid)<<endl;
	th_start= std::chrono::high_resolution_clock::now();
	
	if(num_req < RUN_TIME)
	{
		flag=connect_to_server(threadid,REQUEST_TYPE,file_name);
	}
	else 
	{
		//cout<<"exiting thread  "<<(long)threadid<<endl;
		break;
	}
	th_end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> th_elapsed = th_end -th_start;

	if(flag==-1)
	{
		t_err_counter++;

	}
	else if(flag !=0)
	{
		t_complete_counter++;
		data_tfred=data_tfred+flag;
		resp_time.push_back(th_elapsed.count());
		//cout<<flag<<" dt "<<data_tfred<<"id"<<long(threadid)<<"fn"<<file_name<<endl;
	}
	else if(flag==0 && strcmp(REQUEST_TYPE,"get")==0)
	{
		t_unsucc_counter++;

	}
	
	if(strcmp(REQUEST_TYPE,"new")==0)
	{
		if(flag==0)
			t_complete_counter++;
			resp_time.push_back(th_elapsed.count());
	}


	num_req++;

}


// use iterator to access the values
   vector<double>::iterator v = resp_time.begin();
   while( v != resp_time.end()) {
   		//cout<<*v<<endl;
   		total_resp_time=total_resp_time+ *v;
          v++;
   }
//cout<<total_resp_time/num_req<<endl;
ret->total_resp_time=total_resp_time;
ret->comp=t_complete_counter;
ret->err=t_err_counter;
ret->data_tfred=data_tfred/1000.0;
ret->login_unsuc=t_unsucc_counter;
//cout<<unsuccessful_counter;
 pthread_exit((void*)ret);


}

char *randomString(int len) {
	char *rstr  = (char*)malloc((len + 1) * sizeof(char));
  srand(time(0));
    int i;
  for (i = 0; i < len; i++) {
    rstr[i] = alphabet[rand() %(strlen(alphabet))];
  }
  rstr[len] = '\0';
  return rstr;
}
