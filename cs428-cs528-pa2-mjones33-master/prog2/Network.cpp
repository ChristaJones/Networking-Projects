
#include <iostream>
#include <fstream>
#include <sstream>
#include <string> 
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>
#include <vector>
#include <stdlib.h>
#include <thread>
#include "Node.hpp"
using namespace std;

void data(Node n)
{
		struct sockaddr_in data_addr;
		data_addr.sin_family = AF_INET;		
		data_addr.sin_port = htons(n.get_dataport());
		struct hostent *h2  = gethostbyname(n.get_hostname());
		memcpy(&data_addr.sin_addr.s_addr, h2->h_addr, h2->h_length); 
		if (bind(n.get_data(),  (struct sockaddr *) &data_addr, sizeof(data_addr))< 0) {
                perror("bind failed");
        }

}


void control(Node n, fd_set livesdset)
{
		/*Create Address*/
		int buf_size = 20;
    	char buf[buf_size];
		struct sockaddr_in control_addr;
      control_addr.sin_family = AF_INET;		
      control_addr.sin_port = htons(n.get_controlport());
      struct hostent *h  = gethostbyname(n.get_hostname());
		memcpy(&control_addr.sin_addr.s_addr, h->h_addr, h->h_length);
		printf("%s\n",n.get_hostname());
		inet_ntop(AF_INET, &(control_addr.sin_addr), buf, buf_size);
	   printf("server adrress: %s\n", buf);
		/*Bind socket to address*/      
		if (bind(n.get_control(),  (struct sockaddr *) &control_addr, sizeof(control_addr)) < 0) {
                perror("bind failed");
					exit(0);
        }
		printf("i'm bound\n");
	  	/*FD_ZERO(&livesdset);
	  	//FD_ZERO(&copy);
	  	FD_SET(n.get_control(), &livesdset);
	  	int livesdmax= n.get_control();

		//Process message*/
		/*select(livesdmax+1, &livesdset, NULL, NULL, NULL);
		if(FD_ISSET(n.get_control(), &livesdset))
		{*/
		while(1){		
			n.receive();
			//FD_CLR();
		//}
	}
}
int main(int argc, char *argv[]) {

	if(argc != 3)
	{
		printf("usage: filename and node missing\n");
	}
	else 
	{
		//Read from file
		ifstream file("testconfig.txt");
    	string line;
		getline(file, line);
		vector<Node> network;
		vector<tuple<int, ushort, string> > beta;
		int maxfd, ready; 
		fd_set rset, copy;
		int controlsd = socket(AF_INET, SOCK_DGRAM, 0);
    	
		Node host;

 	   //FD_ZERO(&rset); 
		//maxfd = controlsd + 1;
		while (getline(file, line))
    	{
		//getline(file,line);
        stringstream linestream(line);
        string item;
		  int id, control, data;
		  vector<tuple<int, ushort, string> > friends;
		  string name;
		  getline(linestream, item, '\t');
		  id = stoi(item);
		  
		 	getline(linestream, item, '\t');
		  	name = item;
		  	getline(linestream, item, '\t');  
		  	control = stoi(item);
		  	getline(linestream, item, '\t'); 
		  	data = stoi(item);
		  	beta.push_back(make_tuple(id, control, name)); 

		     	while (getline(linestream, item, '\t')) 
		     	{
					friends.push_back(make_tuple(stoi(item), 0, "-"));         
					//std::cout <<  item << endl;
		     	}

				Node n(id, name, control, data, friends);
				network.push_back(n);
    	}    
		for(int x = 0; x<network.size(); x++)
		{

				//vector<tuple<int, ushort> > n = network[x].getneighbors;
				for(int y = 0; y<beta.size(); y++)
				{
					network[x].setneighbor(get<0>(beta[y]), get<1>(beta[y]), get<2>(beta[y]));
				}
				//FD_SET(network[x].getsocket(), &rset);
				
				if(atoi(&argv[2][4]) == network[x].id())
				{
					host = network[x];
				}
		}

			
			printf("Network has been created now send distance vectors to neighbors %d\n",controlsd);
			
			thread controlthread1(control,host, rset);
			
			if(controlthread1.joinable())
			{
				controlthread1.join();

			}
	/*thread controlthread2(control,network[1], rset);
			if(controlthread2.joinable())
			{
				controlthread2.join();

			}*/
			//close(controlsd);
	}
	return 0;
}
