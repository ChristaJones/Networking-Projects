/*--------------------------------------------------------------------*/
/* functions to connect clients and server */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h>
#include <time.h> 
#include <errno.h>

#include <stdlib.h>

#define MAXNAMELEN 256
/*--------------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/* prepare server to accept requests
 returns file descriptor of socket
 returns -1 on error
 */
int startserver() {
    int sd; /* socket descriptor */

    char * servhost; /* full name of this host */
    ushort servport; /* port assigned to this server */

    /*
     FILL HERE
     create a TCP socket using socket()
     */
     sd = socket(AF_INET, SOCK_STREAM, 0);

    /*
     FILL HERE
     bind the socket to some port using bind()
     let the system choose a port
     */
     struct sockaddr_in my_addr;
     my_addr.sin_family = AF_INET;		
     my_addr.sin_port = htons(0);
     my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
     bind(sd,  (struct sockaddr *) &my_addr, sizeof(my_addr));
 
    /* we are ready to receive connections */
    listen(sd, 5);

    /*
     FILL HERE
     figure out the full host name (servhost)
     use gethostname() and gethostbyname()
     full host name is remote**.cs.binghamton.edu
     */
     servhost = (char*) malloc(128 * sizeof(char));
     gethostname(servhost, 128);
     struct hostent *h = gethostbyname(servhost);
     memcpy(&my_addr.sin_addr.s_addr, h->h_addr, h->h_length);

    /*
     FILL HERE
     figure out the port assigned to this server (servport)
     use getsockname()
     */
     int len = sizeof(struct sockaddr);
     getsockname(sd, (struct sockaddr *) &my_addr, &len);
     servport = ntohs(my_addr.sin_port);

    /* ready to accept requests */
    printf("admin: started server on '%s' at '%hu'\n", servhost, servport);
    free(servhost);
    return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
/*
 establishes connection with the server
 returns file descriptor of socket
 returns -1 on error
 */
int hooktoserver(char *servhost, ushort servport) {
    int sd; /* socket descriptor */
  	 struct sockaddr_in sa;
    ushort clientport; /* port assigned to this client */
	   int thing;
    int buf_size = 20;
    char buf[buf_size];
    /*
     FILL HERE
     create a TCP socket using socket()
     */
     sd = socket(AF_INET, SOCK_STREAM, 0);
     sa.sin_family = AF_INET;
     sa.sin_port = htons(servport);

    /*
     FILL HERE
     connect to the server on 'servhost' at 'servport'
     use gethostbyname() and connect()
     */
     struct hostent *h  = gethostbyname(servhost);
     memcpy(&sa.sin_addr.s_addr, h->h_addr, h->h_length);
     thing = connect(sd, (struct sockaddr *) &sa, sizeof(sa));

    /*
     FILL HERE
     figure out the port assigned to this client
     use getsockname()
     */
		  socklen_t len = sizeof(struct sockaddr);
		  clientport = getsockname(thing, (struct sockaddr *) &sa, &len);
    inet_ntop(AF_INET, &(sa.sin_addr), buf, buf_size);
	   printf("server adrress: %s\n", buf);

    /* succesful. return socket descriptor */
    printf("admin: connected to server on '%s' at '%hu' thru '%hu'\n", servhost,
            servport, clientport);
    printf(">");
    fflush(stdout);
    return (sd);
}
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
int readn(int sd, char *buf, int n) {
    int toberead;
    char * ptr;

    toberead = n;
    ptr = buf;
    while (toberead > 0) {
        int byteread;

        byteread = read(sd, ptr, toberead);
        if (byteread <= 0) {
            if (byteread == -1)
                perror("read");
            return (0);
        }

        toberead -= byteread;
        ptr += byteread;
    }
    return (1);
}

char *recvtext(int sd) {
    char *msg;
    long len;

    /* read the message length */
    if (!readn(sd, (char *) &len, sizeof(len))) {
        return (NULL);
    }
    len = ntohl(len);

    /* allocate space for message text */
    msg = NULL;
    if (len > 0) {
        msg = (char *) malloc(len);
        if (!msg) {
            fprintf(stderr, "error : unable to malloc\n");
            return (NULL);
        }

        /* read the message text */
        if (!readn(sd, msg, len)) {
            free(msg);
            return (NULL);
        }
    }

    /* done reading */
    return (msg);
}

int sendtext(int sd, char *msg) {
    long len;

    /* write lent */
    len = (msg ? strlen(msg) + 1 : 0);
    len = htonl(len);
    write(sd, (char *) &len, sizeof(len));

    /* write message text */
    len = ntohl(len);
    if (len > 0)
        write(sd, msg, len);
    return (1);
}
/*----------------------------------------------------------------*/

