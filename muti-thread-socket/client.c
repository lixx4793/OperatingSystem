/* login: duxxx336, lixx4793
 * date: 04/30/2018
 * name: Feifan Du, Yuhao Li
 * id: 5099129, 5250438
 * Extra credits [No]
 */

#define _BSD_SOURCE
#define NUM_ARGS 3
#define MAX_MSG 256

// note: it does not matter how many bytes are read
// from EACH IO since the reading will be done in a 
// loop until the end of the source is reached
#define BYTES_OF_EACH_IO 1024

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "util.h"

int server_port;
char* server_ip;
char* request_file;
int n_request;
char** requests;
char* relPath;
int diff_dir;

// convert the request in .req to a code of length 2
char* ConvertRequestToCode(char* request) {
	char* code = (char*)malloc(sizeof(char)*2);
	if (strcmp(request, "Return_Winner") == 0) {
		code = "RW";
	} else if (strcmp(request, "Count_Votes") == 0) {
		code = "CV";
	} else if (strcmp(request, "Open_Polls") == 0) {
		code = "OP";
	} else if (strcmp(request, "Add_Votes") == 0) {
		code = "AV";
	} else if (strcmp(request, "Remove_Votes") == 0) {
		code = "RV";
	} else if (strcmp(request, "Close_Polls") == 0) {
		code = "CP";
	} else if (strcmp(request, "Add_Region") == 0) {
		code = "AR";
	}
	return code;
}

// count the votes in file bote_input
char* CountVote(char* vote_input) {
	// open the file
	if (diff_dir) {
		// if the executable is not in the same directory as the input files
		char* vote_input_complete = (char*)malloc(sizeof(char)*(strlen(relPath) + strlen(vote_input) + 1));
		vote_input_complete[0] = '\0';
		strcat(vote_input_complete, relPath);
		strcat(vote_input_complete, vote_input);
		vote_input = vote_input_complete;
		// printf("TO BE OPEN: %s\n", vote_input);
	}
	char* results = (char*)malloc(sizeof(char)*MAX_NUMBER*MAX_NUMBER);
	int input = open(vote_input, O_RDONLY);
	if (input < 0) {
		printf("Failed to open vote-input file: %s\n", vote_input);
		results = "";
		return results;
	}
	
	// initialize a linked list of candidates
	struct candidate2* root = (struct candidate2*)malloc(sizeof(struct candidate2)*MAX_NUMBER);
	int id = 1;
	char* buffer = malloc(sizeof(char)*BYTES_OF_EACH_IO);
	int bytes;
	
	// read and accumulate the votes
	while ((bytes = read(input, buffer, BYTES_OF_EACH_IO)) > 0) {
		// printf("votes read: -------%s-------\n", buffer);
		char** votes;
		int nvotes = makeargv(buffer, "\n", &votes);
		for (int i = 0; i < nvotes; i++) {
			votes[i] = trimwhitespace(votes[i]);
			if (strlen(votes[i]) == 0) continue;
			// printf("one vote: -------%s-------\n", votes[i]);
			
			// If candidate already exist, increment the votes
			int addvote = 0;
			candidate2_t* temp = root;
			while (temp!=NULL && temp->id != 0) {
				if (temp->name != NULL && strcmp(temp->name, votes[i]) == 0) {
					(temp->vote)++;
					addvote = 1;
					// printf("added vote to name %s now vote =  %d\n", temp->name, temp->vote);
					break;
				}
				temp++;
			}

			if (addvote == 1) continue;

			// Candidate does not exist already, initialize a new node
			strcpy(root[id-1].name, votes[i]);
			root[id-1].vote = 1;
			root[id-1].id = id;
			// printf("initialized new node with name %s and vote %d\n", root[id-1].name, root[id-1].vote);
			id++;
		}
		buffer = (char*)malloc(sizeof(char)*BYTES_OF_EACH_IO);
	}
	
	// Results: candidate1:count1,candidate2:count2,...
	results[0] = '\0';
	char tot_vote [MAX_NUMBER];
	for (int i=0; i<id-1; i++) {
		strcat(results, root[i].name);
		strcat(results, ":");
		sprintf(tot_vote, "%d", root[i].vote);
		strcat(results, tot_vote);
		if (i<id-2) {
			strcat(results, ",");
		}
	}
	
	return results;
}

// convert a request in .req file to a request to be sent to the server
char* ConvertCodeToRequest(char* code, char** single_request) {
	char* send = (char*)malloc(sizeof(char)*20);
	if (strcmp(code, "RW") == 0) {
		send[0] = '\0';
		strcat(send, code);
		// pad the field for region names
		strcat(send, ";               ;");
	} else if (strcmp(code, "CV") == 0 || strcmp(code, "OP") == 0 || strcmp(code, "CP") == 0) {
		send[0] = '\0';
		strcat(send, code);
		strcat(send, ";");
		strcat(send, single_request[1]);
		// pad the field for region names
		int padding = 15 - strlen(single_request[1]);
		for (int j = 0; j < padding; j++) {
			strcat(send, " ");
		}
		strcat(send, ";");
	} else if (strcmp(code, "AV") == 0 || strcmp(code, "RV") == 0) {
		send[0] = '\0';
		strcat(send, code);
		strcat(send, ";");
		strcat(send, single_request[1]);
		// pad the field for region names
		int padding = 15 - strlen(single_request[1]);
		for (int j = 0; j < padding; j++) {
			strcat(send, " ");
		}
		strcat(send, ";");
		
		// AV and RV also needs data (voting results)
		char* votes;
		votes = CountVote(single_request[2]);
		char* final_req = (char*)malloc(sizeof(char)*(20+strlen(votes)));
		final_req[0] = '\0';
		strcat(final_req, send);
		strcat(final_req, votes);
		return final_req;
	} else if (strcmp(code, "AR") == 0) {
		send[0] = '\0';
		strcat(send, code);
		strcat(send, ";");
		strcat(send, single_request[1]);
		// pad the field for region names
		int padding = 15 - strlen(single_request[1]);
		for (int j = 0; j < padding; j++) {
			strcat(send, " ");
		}
		strcat(send, ";");
		
		// AR also needs data (new child name)
		char* new_child = single_request[2];
		char* final_req = (char*)malloc(sizeof(char)*(20+strlen(new_child)));
		final_req[0] = '\0';
		strcat(final_req, send);
		strcat(final_req, new_child);
		return final_req;
	}
	return send;
}

// convert a request to be sent to the server
// to a string to be printed out by the client
char* ConvertRequestToPrint(char* request) {
	char** fields;
	int n_null = 0;
	int print_len = 0;
	int n_field = makeargv(request, ";", &fields);
	for (int i = 0; i < n_field; i++) {
		fields[i] = trimwhitespace(fields[i]);
		if (strlen(fields[i]) == 0) n_null++;
		print_len = print_len + strlen(fields[i]);
	}
	char* print_msg = (char*)malloc(sizeof(char)*(print_len + 14));
	print_msg[0] = '\0';
	// print out "(null)" if a field is empty
	for (int i = 0; i < n_field; i++) {
		if (strlen(fields[i]) == 0) {
			strcat(print_msg, "(null)");
		} else {
			strcat(print_msg, fields[i]);
		}
		if (i < (n_field-1)) {
			strcat(print_msg, " ");
		}
	}
	if (n_field < 3) {
		strcat(print_msg, " ");
		strcat(print_msg, "(null)");
	}
	return print_msg;
}

int main(int argc, char** argv) {
	if (argc != NUM_ARGS + 1) {
		printf("Usage: ./client <REQ FILE> <SERVER IP> <SERVER PORT>\n");
		exit(1);
	}

	request_file = argv[1];
	server_port = atoi(argv[3]);
	server_ip = argv[2];
	
	// test to see if the executable is in the same directory as the input files
	// if so, store the relative path
	char** pathParts;
	int n_slash = makeargv(request_file, "/", &pathParts);
	if (n_slash == 1) {
		// printf("same directory\n");
		diff_dir = 0;
	} else {
		// printf("NOT THE SAME DIREC\n");
		diff_dir = 1;
		char* partialPath = (char*)malloc(sizeof(char)*strlen(request_file));
		partialPath[0] = '\0';
		for (int i = 0; i < n_slash -1; i++) {
			strcat(partialPath, pathParts[i]);
			strcat(partialPath, "/");
		}
		relPath = partialPath;
	}

	// Create a TCP socket
	int sock = socket(AF_INET , SOCK_STREAM , 0);

	// Specify an address to connect to
	// (we use the local host or 'loop-back' address).
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(server_port);
	address.sin_addr.s_addr = inet_addr(server_ip);
	
	int check_connect = connect(sock, (struct sockaddr *) &address, sizeof(address));

	// Connect the socket
	if (check_connect != 0) {
		printf("Connection failed! Server with ip: %s" 
			" and port: %d is unreachable or does not exist.\n",
			server_ip, server_port);
		exit(1);
	}

	printf("Initiated connection with server at %s:" 
		"%d\n", server_ip, server_port);

	// Buffer for data from req file
	char* buffer = (char*)malloc(sizeof(char)*(BYTES_OF_EACH_IO));
	// buffer for data read from socket
	char buffer_soc[MAX_MSG];

	// Open the file
	int fd = open(request_file, O_RDONLY);
	if (fd < 0) {
		printf("Failed to open the file containing the requests.\n");
		exit(1);
	}

	int bytes;
	char** single_request;
	char* code;
	char* request;
	char* msg;
	char* final_req;
	char** responses;
	char* response;
	
	// read requests from .req file
	while ((bytes = read(fd, buffer, BYTES_OF_EACH_IO)) > 0) {
		// printf("content read: ----%s----\n", buffer);
		n_request = makeargv(buffer, "\n", &requests);
		for (int i = 0; i < n_request; i++) {
			requests[i] = trimwhitespace(requests[i]);
			if (strlen(requests[i]) == 0) continue;
			// printf("one command: ----%s----\n", requests[i]);
			makeargv(requests[i], " ", &single_request);
			code = ConvertRequestToCode(single_request[0]);
			request = ConvertCodeToRequest(code, single_request);
			// printf("request: ----%s----\n", request);
			
			// Write request to socket
			int length = strlen(request) + 1;
			if (length <= 1) continue;
			final_req = (char*)malloc(sizeof(char)*length);
			final_req[0] = '\0';
			strcat(final_req, request);
			// printf("full request: ----%s----\n", final_req);
			write(sock, final_req, length);
			msg = ConvertRequestToPrint(request);
			printf("Sending request to server: %s\n", msg);
			
			// receive response from the server
			read(sock, &buffer_soc, MAX_MSG);
			int n = makeargv(buffer_soc, ";", &responses);
			if (n > 1) {
				response = (char*)malloc(sizeof(char)*(strlen(responses[0])+strlen(responses[1])+2));
			} else {
				response = (char*)malloc(sizeof(char)*(strlen(responses[0])+2));
			}
			response[0] = '\0';
			strcat(response, responses[0]);
			strcat(response, " ");
			if (n > 1) { strcat(response, responses[1]); }
			else { strcat(response, "(null)"); }
			printf("Received response from server: %s\n", response);
		}
	}

	// Close the file
	close(fd);
	// Close the socket
	close(sock);
	
	printf("Closed connection with server at %s:" 
		"%d\n", server_ip, server_port);
}
