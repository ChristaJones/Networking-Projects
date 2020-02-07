/*--------------------------------------------------------------------*/
/* conference server */

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

#include <stdlib.h>

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int startserver();
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int fd_isset(int fd, fd_set *fsp) {
    return FD_ISSET(fd, fsp);
}
/* main routine */
int main(int argc, char *argv[]) {
    int servsock; /* server socket descriptor */

    fd_set livesdset, copy; /* set of live client sockets */
    int livesdmax; /* largest live client socket descriptor */

    struct sockaddr_in client;
    client.sin_family = AF_INET; 
    socklen_t client_size = sizeof(struct sockaddr_in);

    char *msg;
    /* check usage */
    if (argc != 1) {
        fprintf(stderr, "usage : %s\n", argv[0]);
        exit(1);
    }

    /* get ready to receive requests */
    servsock = startserver();
    if (servsock == -1) {
        perror("Error on starting server: ");
        exit(1);
    }

    /*
     FILL HERE:
     init the set of live clients
     */
	  FD_ZERO(&livesdset);
	  FD_ZERO(&copy);
	  FD_SET(servsock, &livesdset);
	  livesdmax= servsock;
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

        /* look for messages from live clients */
        for (frsock = 3; frsock <= livesdmax; frsock++) {
            /* skip the listen socket */
            /* this case is covered separately */
            if (frsock == servsock)
                continue;

            if ( /* FILL HERE: message from client 'frsock'? */ FD_ISSET(frsock, &copy)) {
                char * clienthost = (char*) malloc(64); /* host name of the client */
                ushort clientport; /* port number of the client */

                /*
                 FILL HERE:
                 figure out client's host name and port
                 using getpeername() and gethostbyaddr()
                 */
						getpeername(frsock, (struct sockaddr *) &client, &client_size);
					   struct hostent *h = gethostbyaddr(&client.sin_addr.s_addr, client_size ,AF_INET);
						memcpy(clienthost, h->h_name, 64);
						clientport = ntohs(client.sin_port);

                /* read the message */
                msg = recvtext(frsock);
                if (!msg) {
                    /* disconnect from client */
                    printf("admin: disconnect from '%s(%hu)'\n", clienthost,
                            clientport);

                    /*
                     FILL HERE:
                     remove this guy from the set of live clients
                     */

                     FD_CLR(frsock, &livesdset);

                    /* close the socket */
                    close(frsock);
                } else {
                    /*
                     FILL HERE
                     send the message to all live clients
                     except the one that sent the message
                     */
							for(int i = 3; i < livesdmax+1; i++)
							{
								if(i != servsock && i != frsock)
								{
										sendtext(i, msg);
								}
							}
                    /* display the message */
                    printf("%s(%hu): %s", clienthost, clientport, msg);

                    /* free the message */
                    free(msg);
                }
            }
        }

        /* look for connect requests */
        if ( /* FILL HERE: connect request from a new client? */FD_ISSET(servsock, &copy) ) {
		int csd;
            /*
             FILL HERE:
             accept a new connection request
             */
				csd = accept(servsock, (struct sockaddr*)&client, &client_size);

            /* if accept is fine? */
            if (csd != -1) {
                char * clienthost; /* host name of the client */
                ushort clientport; /* port number of the client */

                /*
                 FILL HERE:
                 figure out client's host name and port
                 using gethostbyaddr() and without using getpeername().
                 */
						struct hostent *h = gethostbyaddr(&client.sin_addr, sizeof(client.sin_addr), AF_INET);
						clienthost = h->h_name;
						clientport = ntohs(client.sin_port);
                  printf("admin: connect from '%s' at '%hu'\n", clienthost,
                        clientport);

                /*
                 FILL HERE:
                 add this guy to set of live clients
                 */
						FD_SET(csd, &livesdset); 
						if(csd > livesdmax)
		    			{
							livesdmax = csd;
						}

            } else {
                perror("accept");
                exit(0);
            }
        }
    }
    return 0;
}
/*--------------------------------------------------------------------*/

