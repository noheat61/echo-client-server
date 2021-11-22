all: echo-server echo-client

echo-server:
	g++ -o echo-server echo-server.cpp -lpthread
	
echo-client:
	g++ -o echo-client echo-client.cpp -lpthread

clean:
	rm -f echo-server
	rm -f echo-client
	rm -f *.o