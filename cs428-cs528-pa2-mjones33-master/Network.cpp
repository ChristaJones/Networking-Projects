
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
#include "Node.hpp"
using namespace std;

int main(int argc, char *argv[]) {

	if(argc != 2)
	{
		printf("usage: filename missing\n");
	}
	else
	{
		//Read from file
		 ifstream file("configuration.txt");
    	 string line;
		getline(file, line);
		vector<Node> network;
    	while (getline(file, line))
    	{
        stringstream linestream(line);
        string item;
		  int id, control, data;
		  vector<int> friends;
		  string name;
		  getline(linestream, item, '\t');
		  id = stoi(item);
		  getline(linestream, item, '\t');
		  name = item;
		  getline(linestream, item, '\t');
		  control = stoi(item);
		  getline(linestream, item, '\t');
		  data = stoi(item);
		  
        while (getline(linestream, item, '\t'))
        {
			   friends.push_back(stoi(item));         
				//std::cout <<  item << endl;
        }

			Node n(id, name, control, data, friends);
			network.push_back(n);
    	}    
			//cout<<network[2].getname()<<endl;
			printf("Network has been created now send distance vectors to neighbors\n");

		
	}
	return 0;
}
