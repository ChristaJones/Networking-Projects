/*--------------------------------------------------------------------*/
/* conference client */

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

#define MAXMSGLEN  1024
#define STDIN 0

extern char * recvtext(int sd);
extern int sendtext(int sd, char *msg);

extern int hooktoserver(char *servhost, ushort servport);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
int main(int argc, char *argv[]) {
    int sock;
    fd_set rfds;
    int retval;
    int save;
    
    /* check usage */
    if (argc != 3) {
        fprintf(stderr, "usage : %s <servhost> <servport>\n", argv[0]);
        exit(1);
    }

    /* get hooked on to the server */
    sock = hooktoserver(argv[1], atoi(argv[2]));
    if (sock == -1) {
        perror("Error: ");
        exit(1);
    }

    FD_ZERO(&rfds);

    FD_SET(0, &rfds);
    FD_SET(sock, &rfds);

    /* keep talking */
    while (1) {

        /*
         FILL HERE
         use select() to watch simultaneously for
         inputs from user and messages from server
         */
            FD_ZERO(&rfds);	
			FD_SET(STDIN, &rfds);
			FD_SET(sock, &rfds);
			save = sock; 
            retval = select(save+1, &rfds, NULL, NULL, NULL);
        if (/* FILL HERE: message from server? */FD_ISSET(save, &rfds)) {
            char *msg;
            msg = recvtext(sock);
            if (!msg) {
                /* server killed, exit */
                fprintf(stderr, "error: server died\n");
                exit(1);
            }

            /* display the message */
            printf(">>> %s", msg);
            /* free the message */
            free(msg);
        }

        if (/* FILL HERE: input from keyboard? */FD_ISSET(STDIN, &rfds)) {
            char msg[MAXMSGLEN];
            if (!fgets(msg, MAXMSGLEN, stdin))
                exit(0);
            sendtext(sock, msg);
        }

        printf(">");
        fflush(stdout);
    }
    return 0;
}
/*--------------------------------------------------------------------*/

