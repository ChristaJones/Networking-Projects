.c.o:
	g++ -g -c $?

# compile client 
all: network client

# compile client only
network: Network.o
	g++ -g -pthread -o Network Network.o 

# compile client only
client: client.o
	g++ -g -o client client.o 
# compile server program  
clean:
	rm -f *.o network client

