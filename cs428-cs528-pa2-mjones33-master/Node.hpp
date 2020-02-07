
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

					 Packet routetrace;
					 int nodeid;
					 int max;
					 int controlsd, datasd;
					 struct sockaddr_in controladdr, dataaddr;
					 fd_set rset; 
                string hostname; /* host name of the node */
                ushort control_port; /* control port number of the node */
					 ushort data_port; /* data port number of the node */
					 vector<int> neighbors; /* vector of all directly connected neighbors */
 					 vector<tuple<int, int, int> > distance_vector; /* distance vector of node (Destination, Next hop, Distance);*/
					 vector<tuple<int, int, int> > routing_table; /* distance vector of node (Destination, Next hop, Distance);*/

		/*Code for sending messages
			0-distance vector 
				->next is size 
				->followed by vector
			1-routetrace
			2-create link
			3-remove link
		*/
		
		public:
				/*Constructor*/
				Node(int nid, string name, ushort cp, ushort dp, vector<int> friends)
				{
					//std::cout<<"Constructor called\n";
					
					//Set parameters					
					nodeid = nid;
					hostname = name;
					control_port = cp;
					data_port = dp;
					neighbors = friends;

					/*Create and bind Control socket*/
					controlsd = socket(AF_INET, SOCK_DGRAM, 0);
					controladdr.sin_family = AF_INET;		
					controladdr.sin_port = htons(control_port);
					struct hostent *h1  = gethostbyname(hostname.c_str());
					memcpy(&controladdr.sin_addr.s_addr, h1->h_addr, h1->h_length);
					bind(controlsd,  (struct sockaddr *) &controladdr, sizeof(controladdr));

					printf("Control Socket %d ", controlsd);

					/*Create and bind data socket*/
					datasd = socket(AF_INET, SOCK_DGRAM, 0);
					dataaddr.sin_family = AF_INET;		
					dataaddr.sin_port = htons(data_port);
					struct hostent *h2  = gethostbyname(hostname.c_str());
					memcpy(&controladdr.sin_addr.s_addr, h2->h_addr, h2->h_length);
					bind(datasd,  (struct sockaddr *) &dataaddr, sizeof(dataaddr));

					//create distance vector			
					for(int i = 0; i< neighbors.size(); i++)
					{
						 distance_vector.push_back(make_tuple(neighbors[i], neighbors[i],1)); 
					}

					//copy distance vector into routing table
					copy(distance_vector.begin(), distance_vector.end(), back_inserter(routing_table)); 

					cout<<"NodeID "<<nodeid<<"\tHostname "<<hostname<<"\tControl Port "<<control_port<<"\tData Port "<<data_port<<endl;


					//send distance vector to neighbors
				};

				/*Destructor*/				
				~Node()
				{
					//std::cout<<"Destructor called on "<<nodeid<<endl;		
				};

				string getname()
				{
					return hostname;
				}
				
				/*Sends distance vector to all directly connected neighbors*/
				/*void send(struct sockaddr remaddr)
				{
					//
				}*/

				/*Recieves distance vector from directly connected neighbors*/
				void recieve()
				{
					while(1){

						printf("Recieving a message for node %d on port %d\n", nodeid, );
							int code = message[0]-48;
							switch(code)
							{
								case 0: printf("Recieving a distance vector\n");
											//message+=2;									
											//makevector(message);
											//update(v);	
										break;

								case 1: printf("Routetrace\n");
											trace_route(node);		  
										break;

								case 2: printf("Creating link\n");
											createlink(node);		  
										break;

								case 3: printf("Removing link\n");
											removelink(node);		  
										break;
						
								default: printf("ERROR WRONG SELECTION\n");

							}
					}

				}

				/*Converts message to a vector*/
				/*vector<tuple<int, int, int>>makevector(char * m)
				{
					    vector<tuple<int, int, int> > v; 
						 istringstream sst(m);
						 string token;
						 getline(sst, token, ',');
						 int size = stoi(token);
						 
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
						 }
					return v;
				}

				/*Converts message to a vector*/
				/*char const* makemessage(vector<tuple<int, int, int>>dv)
				{
					    stringstream a;
						 a<<'0'<<','<<dv.size();
						 for(int y = 0; y< dv.size(); y++)
						 {
							  a<<','<< get<0>(dv[y]) <<','<< get<1>(dv[y]) <<','<<get<2>(dv[y]); 
						 }
						 //cout<< a.str();
						 string s = a.str();
						 char const* mesg = s.c_str();
					
					return mesg;
				}*/

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
				/*void createlink(int node)
				{
					std::cout<<"Create link called between nodes "<<nodeid<<" and "<< node<<endl;
					//check if n is in distance vector and routing table
					int x = 0;
					int y = 0;
					bool found = false;									
					bool foun2 = false;				
					while(!found && x != routing_table.size())
					{
						if(get<0>(routing_table[x]) == node)
						{
							get<1>(routing_table[x]) = node;
							get<2>(routing_table[x]) = 1;
							found = true;

							while(!found2 && y!= distance_vector.size())
							{
								if(get<0>(distance[y]) == node)
								{
									get<1>(distance_vector[y]) = node;
									get<2>(distance_vector[y]) = 1;
									found2 = true;
								}
								y++;
							}
							if(!found2)
							{
								distance_vector.push_back(make_tuple(n, n ,1));
							}
						}
						x++;
					}

					if(!found)					
					{
						distance_vector.push_back(make_tuple(n, n ,1));
	 					routing_table.push_back(make_tuple(n, n ,1));
					}
				}*/
	
				/* Removes the connection between two nodes*/
				/*void removelink(int node)
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
				}*/

				/* Displays the route trace between two nodes*/
				/*void trace_route(int node){
					std::cout<<"Routetrace called between nodes "<<nodeid<<" and "<< node<<endl;			
				}*/


};
#endif
