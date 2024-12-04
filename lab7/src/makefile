all: tcp udp

tcp:
	gcc -o tcpclient tcpclient.c
	gcc -o tcpserver tcpserver.c  

udp:
	gcc -o udpserver udpserver.c
	gcc -o udpclient udpclient.c

clean:
	rm udpclient udpserver tcpclient tcpserver

# ./tcpserver --bufsize 100 --port 5051
# ./tcpclient --bufsize 100 --ip 127.0.0.0 --port 5051