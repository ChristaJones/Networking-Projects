
#ifndef NODE_HPP
#define NODE_HPP

#include <iostream>
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
#include <algorithm> // for copy() and assign() 
#include <iterator> // for back_inserter 
#include <tuple> 
#include <stdlib.h>

using namespace std;

class Node 
	{
		private:
					 struct Packet{
						short source;
						short destination;
						short packet;
						short TTL;
					 };

					 //Packet routetrace;
					 int nodeid;
					 //int max;
					 int controlsd, datasd;
  					 char message[2048];
                string hostname; /* host name of the node */
                ushort control_port; /* control port number of the node */
					 ushort data_port; /* data port number of the node */
					 vector<tuple<int, ushort, string> >neighbors; /* vector of all directly connected neighbors (nodeid, port, hostname) */
 					 vector<tuple<int, int, int> > distance_vector; /* distance vector of node (Destination, Next hop, Distance);*/
					 vector<tuple<int, int, int> > routing_table; /* distance vector of node (Destination, Next hop, Distance);*/

		/*Code for sending messages
			0-distance vector 
				->next is size 
				->followed by vector
			1-routetrace
			2-create link
			3-remove link
			4-wake up
		*/
		
		public:
				Node()
				{

				}
				/*Constructor*/
				Node(int nid, string name, ushort cp, ushort dp, vector<tuple<int, ushort, string> > friends)
				{
					//std::cout<<"Constructor called\n";
					
					//Set parameters					
					nodeid = nid;
					hostname = name;
					control_port = cp;
					data_port = dp;
					copy(friends.begin(), friends.end(), back_inserter(neighbors));
					
					/*Create Control socket*/
					controlsd = socket(AF_INET, SOCK_DGRAM, 0);
					if (controlsd < 0 )
					{
						printf("socket failed");
						exit(0);
					}
					/*Create and bind data socket*/
					datasd = socket(AF_INET, SOCK_DGRAM, 0);
					//dataaddr.sin_family = AF_INET;		
					//dataaddr.sin_port = htons(data_port);
					//struct hostent *h2  = gethostbyname(hostname.c_str());
					//memcpy(&controladdr.sin_addr.s_addr, h2->h_addr, h2->h_length);
					//bind(datasd,  (struct sockaddr *) &dataaddr, sizeof(dataaddr));

					//create distance vector			
					for(int i = 0; i< neighbors.size(); i++)
					{
						 distance_vector.push_back(make_tuple(get<0>(neighbors[i]), get<0>(neighbors[i]),1)); 
					}

					//copy distance vector into routing table
					copy(distance_vector.begin(), distance_vector.end(), back_inserter(routing_table)); 

					cout<<"NodeID "<<nodeid<<"\tHostname "<<hostname<<"\tControl Port "<<control_port<<"\tData Port "<<data_port<<endl;
				};

				Node &operator=(Node &diff)
				{
					using std::swap;
					swap(nodeid, diff.nodeid);
					swap(controlsd,diff.controlsd);
					swap(datasd,diff.datasd);
					swap(hostname, diff.hostname);
					swap(control_port, diff.control_port);
					swap(data_port, diff.data_port);
					swap(distance_vector, diff.distance_vector);
					swap(neighbors, diff.neighbors);
					swap(routing_table, diff.routing_table);
					swap(message, diff.message);
					return *this;
				};
				/*Destructor*/				
				~Node()
				{
					//std::cout<<"Destructor called on "<<nodeid<<endl;		
				};

				vector<tuple<int, ushort,string> > getneighbors()
				{
					return neighbors;
				}
				int id()
				{
					return nodeid;
				}	
				int get_control()
				{
					return controlsd;
				}

				int get_data()
				{
					return datasd;
				}

				ushort get_controlport()
				{
					return control_port;
				}

				ushort get_dataport()
				{
					return data_port;
				}
				char * get_hostname()
				{
					return (char *)hostname.c_str();
				}
				void setneighbor(int id, ushort port, string name)
				{
					  auto it = std::find_if(neighbors.begin(), neighbors.end(), [id](const std::tuple<int,ushort, string>& e) {return std::get<0>(e) == id;});
					  if (it != neighbors.end()) {
						  //std::cout << "Found" << std::endl;
						  get<1>(*it) = port;
						  get<2>(*it) = name;
						  //std::cout<<get<0>(*it)<<" "<<get<1>(*it)<<" "<<get<2>(*it)<<std::endl;
						}
				}
				/*Sends distance vector to all directly connected neighbors*/
				void send(string message, struct sockaddr_in dest, socklen_t addrlen)
				{
						char * buff = (char *)message.c_str();					
						if (sendto(controlsd, buff, strlen(buff), 0,(struct sockaddr*) &dest, addrlen)< 0) {
							perror("sendto failed");
						}
				}

				/*Sends distance vector to all directly connected neighbors and Recieves distance vector from directly connected neighbors*/
				void receive()
				{

						struct sockaddr_in src; 
						socklen_t addrsize = sizeof(src);
					//while(1){
							 //bind(controlsd,  (struct sockaddr *) &controladdr, sizeof(controladdr));
							 printf("Recieving a message for node %d on port %d\n", nodeid, control_port);
							 int n; 
							 //socklen_t len;
							 n = recvfrom(controlsd, (char*) message, sizeof(message), 0, (struct sockaddr *) &src, &addrsize);
							 printf("received %d bytes\n", n);
                		 			if (n > 0) {
                        					message[n] = 0;
                        					printf("received message: \"%s\"\n", message);
                					}  
						   	int code = message[0]-48;
							switch(code)
							{
								case 0: {
											printf("Recieving a distance vector\n");
											//Recieve distance vector*/			
											//message+=2;					
											vector<tuple<int, int, int> > v = makevector(&message[2]);
											/*Update routing table*/											
											//update(v);
										}
										break;
								case 1: {
											printf("Routetrace\n");
											int tracenode = message[1]-48;
											trace_route(tracenode);		  
										break;
											}
								case 2: {
											printf("Creating link\n");
											int createnode = message[1]-48;		
											/*Create link between nodes*/
											createlink(createnode);
								      			string cm = "created"; 			
											/*Send message back to client*/											
											send(cm, src, addrsize);

											/*Send Distance vector to neighbor(s)*/
											struct sockaddr_in dest; 
											socklen_t addrlen = sizeof(dest);
											dest.sin_family = AF_INET;		
											dest.sin_port = htons(get<1>(neighbors[0]));
											struct hostent *h  = gethostbyname(get<2>(neighbors[0]).c_str());
											memcpy(&dest.sin_addr.s_addr, h->h_addr, h->h_length);
											send(makemessage(),dest,addrlen);
										  }
										break;

								case 3: {
											printf("Removing link\n");
											int removenode = message[1]-48;
											/*Remove link between nodes*/
											removelink(removenode);	
											}	  
										break;
								case 4: {
											printf("I'm awake\n");
											string mesg = "I'm awake";
											/*Send message back to client*/											
											send(mesg, src, addrsize);

											/*Send Distance vector to neighbor(s)*/
											struct sockaddr_in dest; 
											socklen_t addrlen = sizeof(dest);
											dest.sin_family = AF_INET;		
											dest.sin_port = htons(get<1>(neighbors[0]));
											struct hostent *h  = gethostbyname(get<2>(neighbors[0]).c_str());
											memcpy(&dest.sin_addr.s_addr, h->h_addr, h->h_length);
											send(makemessage(),dest,addrlen);
										}
									
										break;
						
								default: printf("ERROR WRONG SELECTION\n");

							}
					//}

				}
 

				/*Converts message to a vector*/
				vector<tuple<int, int, int>>makevector(char * m)
				{
					    vector<tuple<int, int, int> > v; 
						 istringstream sst(m);
						 string token;
						 getline(sst, token, ',');
						 std::cout<<token<<endl;
						 /*int size = stoi(token);
						 
						 for(int x = 0; x< size; x++){
							  int i,j,k;
							  getline(sst, token, ','); 
							  i = stoi(token);
							  getline(sst, token, ','); 
							  j = stoi(token);
							  getline(sst, token, ','); 
							  k = stoi(token);
							  v.push_back(make_tuple(i,j,k)); 
							  //cout << get<0>(v[x]) <<' '<< get<1>(v[x]) <<' '<<get<2>(v[x]) <<'\n';
						 }*/
					return v;
				}

				/*Converts message to a vector*/
				string makemessage()
				{
						char * mesg = (char*) malloc(2048);
					    stringstream a;
						 a<<'0'<<','<<distance_vector.size();
						 for(int y = 0; y< distance_vector.size(); y++)
						 {
							  a<<','<< get<0>(distance_vector[y]) <<','<< get<1>(distance_vector[y]) <<','<<get<2>(distance_vector[y]); 
						 }
						 //cout<< a.str()<<endl;
						 string s = a.str();
						 //mesg = (char*) s.c_str();
						
					return s;
				}

				/*Updates the routing table*/
				/*void updatetable(vector<tuple<int, int, int>>dv, int q)
				{
					std::cout<<"Updating routing table\n";
					std::vector<tuple<int, int, int>>::iterator it;
					for(int x = 0; x< dv.size(); x++)
					{

					   it = find (routing_table.begin(),routing_table.end(), dv[x]);
						if(it != myvector.end()) //Case 2: route is already in routing table
						{
							
							if(get<1>(dv[x]) !=q)
							{
							//p'!= q
								if(get<2>(dv[x]) > get<2>(*it)+1)
								{
									//c'>c+1
									get<1>(*it) =  q;
									get<2>(*it) += 1 ;
								}
							}
							else //Case 3: route is in table
							{
								//p'==q exists in the routing table
								get<2>(*it) += 1 ;
							}
						}
						else //Case 1: route is not already in routing table
						{
							//route not in routing table 
							//must add new node to routing table
							routing_table.push_back(make_tuple(get<0>(dv[x],q get<2>(*it)+1)));
						}

					}
					printf("Node %d routing table\n", nodeid);
					for(int y = 0; y< routing_table.size(); y++)
					{
						cout<<"["<< get<0>(routing_table[y]) <<","<< get<1>(routing_table[y]) <<","<<get<2>(dv[y])<<"],"; 
					}
					cout<<endl;
				}*/

				/* Establishes a connection between two nodes*/
				void createlink(int node)
				{
					std::cout<<"Create link called between nodes "<<nodeid<<" and "<< node<<endl;
					//check if n is in distance vector and routing table
					int x = 0;
					int y = 0;
					bool found = false;									
					bool found2 = false;				
					while(!found && x != routing_table.size())
					{
						if(get<0>(routing_table[x]) == node)
						{
							get<1>(routing_table[x]) = node;
							get<2>(routing_table[x]) = 1;
							found = true;

							while(!found2 && y != distance_vector.size())
							{
								if(get<0>(distance_vector[y]) == node)
								{
									get<1>(distance_vector[y]) = node;
									get<2>(distance_vector[y]) = 1;
									found2 = true;
								}
								y++;
							}
							if(!found2)
							{
								distance_vector.push_back(make_tuple(node, node ,1));
							}
						}
						x++;
					}

					if(!found)					
					{
						distance_vector.push_back(make_tuple(node, node ,1));
	 					routing_table.push_back(make_tuple(node, node ,1));
					}
				}
	
				/* Removes the connection between two nodes*/
				void removelink(int node)
				{
					std::cout<<"Remove link called between nodes "<<nodeid<<" and "<< node<<endl;
					bool removed = false;
					int x = 0;
					while(!removed && x!=distance_vector.size())
					{
						if(get<0>(distance_vector[x]) == node)
						{
							distance_vector.erase(distance_vector.begin()+x);
							removed = true;
						}
						x++;
					}
					removed = false;
					x = 0;
					while(!removed && x!=routing_table.size())
					{
						if(get<0>(routing_table[x]) == node)
						{
							routing_table.erase(routing_table.begin()+x);
							removed = true;
						}
						x++;
					}
				}

				/* Displays the route trace between two nodes*/
				void trace_route(int node){
					std::cout<<"Routetrace called between nodes "<<nodeid<<" and "<< node<<endl;			
				}


};
#endif
