# login: duxxx336, lixx4793
# 04/30/2018
# name: Feifan Du, Yuhao Li
# id: 5099129, 5250438

CC = gcc
CFLAGS = -std = c99
DBFLAGS = -g
ALL = client.c server.c
Output = client server
make: $(ALL)
		$(CC) $(CFFLAGS) -o client client.c
		$(CC) -pthread $(CFFLAGS) -o server server.c

client:
		$(CC) $(CFFLAGS) -o client client.c

server:
		$(CC) -pthread $(CFFLAGS) -o server server.c

debug: $(ALL)
	$(CC) $(CFFLAGS) $(DBFLAGS) -o client client.c
	$(CC) -pthread $(CFFLAGS) $(DBFLAGS) -o server server.c

clean:
		rm $(Output)
