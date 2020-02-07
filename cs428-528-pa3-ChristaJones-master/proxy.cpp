#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <sys/types.h> 
#include <sys/time.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <stdlib.h>
#include <limits.h>
#include <vector>
#include <iostream> 
#include <algorithm>
#include <iterator> // for back_inserter 
#include <tuple>
#include <regex>
#define SIZE 300000

using namespace std;

//struct 
struct CacheLine{
	string query;
	string data;
	int contentlength = 0;
	clock_t time = 0;
};

bool IsHost (string s) {
  std::size_t found = s.find("Host");
  return found!=string::npos;
}

bool ContentLength(string s){
  std::size_t found = s.find("Content-Length");
  return found!=string::npos;
}

/*int hasPort (string str)
{
    regex reg(":\\d+");
    smatch match;
    string result;
    int port;
    if(regex_search(str, match, reg))
    {
        result = match.str();
        port = stoi(result.substr(1, result.size()-1));
        //cout<<port<<endl;
        return port;
    }
    else
    {
     		return 0;  
    }
    
}*/

int getcontentlength(char * get)
{

	char  buffer[SIZE]={0};
	memcpy(buffer, get, strlen(get)+1);			  
							  
	char * pch;
	vector<string> tokens;
	pch = strtok (buffer,"\r\n");
							  //printf("pch %s\n",pch);
	string re;
	int i = 0;
	while (pch != NULL)
	{
		tokens.push_back(pch);
		pch = strtok (NULL, "\r\n");
			//i++;
	}
   vector<string>::iterator it = find_if (tokens.begin(), tokens.end(), ContentLength);
	string temp = *it;
    regex reg("\\d+");
    smatch match;
    string result;
    if(regex_search(temp, match, reg))
    {
        result = match.str();
        cout<<result<<": result\n";
        re = (result.substr(0, result.size()));
    }
		
	return stoi(re);
}


int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    //result->tv_sec = diff / 1000000;
    result->tv_usec = diff;

    return (diff<0);
}

long int timeval_print( struct timeval *tv)
{
    return (tv->tv_usec/1000);
}

bool compareCache(CacheLine c1, CacheLine c2) 
{ 
    return (c1.time > c2.time); 
}



char * LRU(vector<CacheLine> &c, int servsd, sockaddr_in serv, char * servhost, socklen_t serv_size, int msize, string subject, char * g, string ip, int * msg2size, char * r)
{
	string hit = "CACHE_MISS";
	long time;
	//char r[SIZE] = {0};
	struct timeval tvDiff, tvStart, tvEnd;
	gettimeofday(&tvStart, NULL);
	int cl;
	regex rg("(http://[[:graph:]]+)(.+)");
    smatch match;
    string q;
	
      if (std::regex_search(subject, match, rg)) {
        q = match.str(1);
      }
	auto it = std::find_if(c.begin(), c.end(), [q](const CacheLine& e) {return e.query == q;});
	if(it != c.end())/*cache hit*/
	{
		memcpy(r, (*it).data.c_str(), (*it).data.length());
		(*it).time = clock();		
	}
	else/*cache miss*/
	{
		//Look up response data
		
		servsd = socket(AF_INET, SOCK_STREAM, 0);
		struct hostent *h3  = gethostbyname(servhost);
		memcpy(&serv.sin_addr.s_addr, h3->h_addr, h3->h_length);
		connect(servsd, (struct sockaddr *)&serv, serv_size);				
		send(servsd,g,msize, 0);   
		*msg2size = read(servsd, r, SIZE);
		close(servsd);
		cl = getcontentlength(r);	
		 CacheLine newentry  = {q, r, cl,clock()};
		 long space = 0;
		//Look for space
		for(int x = 0; x<c.size(); x++)
		{
			space += c[x].contentlength;
		}
		
		//Space in cache
		if(space<c.size())
		{
			c.push_back(newentry);
		}

		//No Space left in cache; Must remove lru entry
		else
		{
			c.erase(c.end()-1);
			c.push_back(newentry);
		}

	}
		sort(c.begin(), c.end(), compareCache);
		gettimeofday(&tvEnd, NULL);
		timeval_subtract(&tvDiff, &tvEnd, &tvStart);
		time = timeval_print( &tvDiff);

		cout<<ip<<"|"<<q<<"|"<<hit<<"|"<<cl<<"|"<<time<<endl;
	return r;
}


int main(int argc, char *argv[]) {
 	 
	 if(argc != 2){
		cout<<"Usage Error: missing cache size"<<endl;
	}

	else{
	 int cache_size = atoi(argv[1]);
	 /*Set Up TCP Server Socket*/	
    int proxysd,serversd; /* socket descriptor */
    char * proxyhost; /* full name of this proxy */
    ushort proxyport; /* port assigned to this proxy */
	 char * clienthost; /* host name of the client */
	 ushort clientport; /* port number of the client */  
	 char * serverhost; /* host name of the server */
	 ushort serverport; /* port number of the server */ 

	 char *msg; 
	 fd_set livesdset, copy; /* set of live client sockets */
    int livesdmax; /* largest live client socket descriptor */ 
	 int clientsd;
	 char get[SIZE] = {0};
	 char response[SIZE] = {0};

	 //Cache
	 vector<CacheLine> cache;

	 proxysd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in proxy;
    proxy.sin_family = AF_INET;		
    proxy.sin_port = htons(0);
    proxy.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(proxysd,  (struct sockaddr *) &proxy, sizeof(proxy));
 
    listen(proxysd, 5);

    proxyhost = (char*) malloc(128 * sizeof(char));
    gethostname(proxyhost, 128);
    struct hostent *h = gethostbyname(proxyhost);
    memcpy(&proxy.sin_addr.s_addr, h->h_addr, h->h_length);
	 memcpy(proxyhost, h->h_name, strlen(h->h_name)+1);

    socklen_t len = sizeof(struct sockaddr);
    getsockname(proxysd, (struct sockaddr *) &proxy, &len);
    proxyport = ntohs(proxy.sin_port);

    printf("admin: started server on '%s' at '%hu'\n", proxyhost, proxyport);
	 printf("%s\n", h->h_name);
     //free(servhost);

	 /*Client*/
    struct sockaddr_in client;
    client.sin_family = AF_INET; 
    socklen_t client_size = sizeof(struct sockaddr_in);

	 /*Server*/
    struct sockaddr_in server;
    server.sin_family = AF_INET; 
    socklen_t server_size = sizeof(struct sockaddr_in);
	

	 FD_ZERO(&livesdset);
	 FD_ZERO(&copy);
	 FD_SET(proxysd, &livesdset);
	 livesdmax= proxysd;
	  //client_list[0]= servsock;

    /* receive requests and process them */
    while (1) {
        int frsock; /* loop variable */
		  copy = livesdset;
        /*
         FILL HERE
         wait using select() for
         messages from existing clients and
         connect requests from new clients
         */
			select(livesdmax+1, &copy, NULL, NULL, NULL);


		 if (FD_ISSET(proxysd, &copy) ) { 
		 
		 	clientsd = accept(proxysd, (struct sockaddr*)&client, &client_size);

			 /* if accept is fine? */
			 if (clientsd != -1)
		 	 {
			 	struct hostent *h = gethostbyaddr(&client.sin_addr, sizeof(client.sin_addr), AF_INET);
			 	clienthost = h->h_name;
			 	clientport = ntohs(client.sin_port);
			 	printf("admin: connect from '%s' at '%hu'\n", clienthost, clientport);
			 }

			FD_SET(clientsd, &livesdset);
		   FD_SET(clientsd, &copy);	
				if(clientsd > livesdmax)
		    	{
					livesdmax = clientsd;
				}

		}
        for (frsock = 3; frsock <= livesdmax; frsock++) {
            /* skip the listen socket */
            /* this case is covered separately */
            if (frsock == proxysd)
                continue;

				if (FD_ISSET(frsock, &copy)) {
					/*getpeername(clientsd, (struct sockaddr *) &client, &client_size);
					struct hostent *h2 = gethostbyaddr(&client.sin_addr.s_addr, client_size ,AF_INET);
					memcpy(clienthost, h2->h_name, 64);
					clientport = ntohs(client.sin_port);*/

		             // read the message 
		         int msgsize = read(frsock, get, SIZE);
		         if (!get) {
		                 // disconnect from client 
				 			  cout<<"admin: disconnect from '"<<clienthost<<"("<<clientport<<")'\n";
		                 FD_CLR(frsock, &livesdset);

		                 // close the socket 
		                 close(frsock);
		             } else {
				
							 char  buffer[SIZE]={0};
							  memcpy(buffer, get, strlen(get)+1);			  
							  
							  char * pch;
							  vector<string> tokens;
							  pch = strtok (buffer,"\r\n");
							  //printf("pch %s\n",pch);
								int i = 0;
						  	  while (pch != NULL)
						     {
							  	tokens.push_back(pch);
							 	//printf ("%s\n",pch);
								//cout<<tokens[i]<<endl;
							 	pch = strtok (NULL, "\r\n");
								//i++;
						     }
							 string getname = tokens.at(0);
						    vector<string>::iterator it = find_if (tokens.begin(), tokens.end(), IsHost);
						    (*it).erase((*it).begin(), (*it).begin()+6);
							 serverhost = (char*) malloc((*it).length());
							 serverhost = (char*) (*it).c_str();
							 server.sin_port = htons(80);
							 char ip_addr [20];
							 int msg2size = 0;
							 inet_ntop(AF_INET, &(client.sin_addr), ip_addr, 20);
							 string ip  = ip_addr;
							 LRU(cache, serversd, server, serverhost, server_size, msgsize, getname, get, ip, &msg2size, response);
									//vector<CacheLine> &c, int servsd, sockaddr_in serv, char * servhost, socklen_t serv_size, int msize, string subject, char * g, string ip, int * msg2size, char * r
							 send(frsock,response,msg2size, 0);  
							 close(frsock);
		                 // free the message 
							 /*memset(get, 0, SIZE);
 							 memset(response, 0, SIZE);
							 memset(buffer, 0, SIZE);*/
		                 //free(msg);
		           }
				}
			}
		}
	}
return 0;
}
