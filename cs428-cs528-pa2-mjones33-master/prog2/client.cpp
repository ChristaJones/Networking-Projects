#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <vector>

#define PORT     8080 
#define MAXLINE 1024 

void error(const char *msg)
{
    perror(msg);
    exit(0);
} 

int main(int argc, char *argv[])
{

    int sockfd; 
    char buffer[MAXLINE]; 
    char *hello = (char*) "4Hello from client"; 
    struct sockaddr_in     servaddr; 
  	 sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    // Creating socket file descriptor 
    if ( sockfd < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    //memset(&servaddr, 0, sizeof(servaddr)); 

    std::vector<struct sockaddr_in> node(3);
    memset(&node[0], 0, sizeof(node[0]));
    memset(&node[1], 0, sizeof(node[1]));
    // Filling server information 

    node[0].sin_family = AF_INET; 
    node[0].sin_port = htons(5000); 
    node[0].sin_addr.s_addr = inet_addr("128.226.114.200");

    node[1].sin_family= AF_INET;
    node[1].sin_port = htons(5002);
    node[1].sin_addr.s_addr = inet_addr("128.226.114.203");
 
    node[2].sin_family= AF_INET;
    node[2].sin_port = htons(5001);
    node[2].sin_addr.s_addr = inet_addr("128.226.114.206");
    socklen_t l;
  	 /*for(int i = 0; i<node.size(); i++)
	 {
		sendto(sockfd, (const char*)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&node[i], sizeof(node[i]));
		printf("sent\n");
		recvfrom(sockfd,(char*) buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&node[i], &l);
		printf("recieved\n");
	 }*/ 
	 printf("Nodes are awake\n");
    int n;
    int index = 0;
	 socklen_t len; 
	 while(1){
		 std::cout<<"give command\n";
		 std::string command;
		 getline(std::cin, command);
		 std::vector<std::string> tok;
		 char * mesg_node1 = (char *) malloc(32);
		 char * mesg_node2 = (char *) malloc(32);
		 std::stringstream ss(command);
	std::string temp;
	while(getline(ss,temp,' '))
	{
		tok.push_back(temp);
	}
	std::string temp2, temp3;
	std::cout<<tok[0]<<std::endl;
	int n1,n2;
	n1 = stoi(tok[1]);
	n2 = stoi(tok[2]);
	//std::size_t found = command.find("create");
	if(tok[0].compare("routetrace")==0)
	{
		temp2 = "1"+tok[2];
		//temp3 = "
		mesg_node1 = (char*) temp2.c_str();
		
    	sendto(sockfd, (const char *)mesg_node1, 32, 0, (const struct sockaddr *) &node[n1-1], sizeof(node[n1-1])); 
    	printf("message sent to node %d.\n",n1); 
          
    	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &node[n1-1], &len); 
    	buffer[n] = '\0'; 
    	printf("Node %s: %s\n",tok[1], buffer); 
	}
	else if(tok[0].compare("create-link")==0)
	{
		char name[128];
		gethostname(name, sizeof(name));
		printf("%s\n", name);
		//struct hostent *h = gethostbyname(
		temp2 = "2,"+tok[2]; //+","+node[n1-1].port+",";
		temp3 = "2"+tok[1];
		mesg_node1 =(char*) temp2.c_str();
		mesg_node2 =(char*) temp3.c_str();

    	sendto(sockfd, (const char *)mesg_node1, 32, 0, (const struct sockaddr *) &node[n1-1], sizeof(node[n1-1])); 
    	printf("message sent to node %d.\n",n1); 
          
    	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &node[n1-1], &len); 
    	buffer[n] = '\0'; 
    	printf("Node %d: %s\n",n1, buffer); 

	sendto(sockfd, (const char*) mesg_node2, 32, 0, (const struct sockaddr *)&node[n2-1],sizeof(node[n2-1]));
	printf("message sent to node %d.\n", n2);

	n = recvfrom(sockfd,(char*)buffer,MAXLINE, 0, (struct sockaddr*) &node[n2-1],&len);
	buffer[n] = '\0';
	printf("Node %d: %s.\n", n2, buffer);
	}

       else if(tok[0].compare("remove-link")==0)
       {
	       temp2 = "3"+ tok[2];
	       temp3 = "3" +tok[1];
	       mesg_node1 = (char*) temp2.c_str();
	       mesg_node2 = (char*) temp3.c_str();
    	sendto(sockfd, (const char *)mesg_node1, 32, 0, (const struct sockaddr *) &node[n1-1], sizeof(node[n1-1])); 
    	printf("message sent to node %d.\n",n1); 
          
    	n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *) &node[n1-1], &len); 
    	buffer[n] = '\0'; 
    	printf("Node %d: %s\n",n1, buffer); 

	sendto(sockfd, (const char*) mesg_node2, 32, 0, (const struct sockaddr *)&node[n2-1],sizeof(node[n1-1]));
	printf("message sent to node %d.\n", n2);

	n = recvfrom(sockfd,(char*)buffer,MAXLINE, 0, (struct sockaddr*) &node[n2-1],&len);
	buffer[n] = '\0';
	printf("Node %d: %s.\n", n2, buffer);
       }
    
	}

    /*n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, (struct sockaddr *) &servaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Node : %s\n", buffer);*/ 

    close(sockfd); 
   return 0;
}


