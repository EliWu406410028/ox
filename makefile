all:
	gcc -o server server.c -lpthread
	gcc -o client client.c -lpthread
clean:
	rm server
	rm client
