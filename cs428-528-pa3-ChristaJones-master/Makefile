.c.o:
	g++ -g -c $?

# proxy 
all: proxy

# compile proxy
proxy: proxy.o
	g++ -g -o proxy proxy.o 

# clean 
clean:
	rm -f *.o proxy
