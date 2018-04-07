CC = gcc
CFLAGS = -std = c99
DBFLAGS = -g
ALL = Vote_Counter.c Leaf_Counter.c Aggregate_Votes.c
Output = Vote_Counter Leaf_Counter Aggregate_Votes
make: $(ALL)
		$(CC) $(CFFLAGS) -o Leaf_Counter Leaf_Counter.c
		$(CC) $(CFFLAGS) -o Aggregate_Votes Aggregate_Votes.c
		$(CC) $(CFFLAGS) -o Vote_Counter Vote_Counter.c

debug: $(ALL)
	$(CC) $(CFFLAGS) $(DBFLAGS) -o Leaf_Counter Leaf_Counter.c
	$(CC) $(CFFLAGS) $(DBFLAGS) -o Aggregate_Votes Aggregate_Votes.c
	$(CC) $(CFFLAGS) $(DBFLAGS) -o Vote_Counter Vote_Counter.c

clean:
		rm $(Output)
