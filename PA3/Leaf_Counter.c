/* login: duxxx336, lixx4793
 * date: 03/07/2018
 * name: Feifan Du, Yuhao Li
 * id: 5099129, 5250438
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "util.h"


int main(int argc, char **argv){

	if (argc != 2) {
		printf("Wrong number of input arguments\n");
		return -1;
	}

	char* path = argv[1];
	int isleaf = isLeafNode(path);
	if (isleaf == -1) {
		// Set by isLeafNode if failed to open directory
		exit(1);
	}
	if (isleaf == 0) {
		printf("Not a leaf node.\n");
		exit(1);
	}

	// Guaranteed to successfully open the directory at this point
	DIR* dir = opendir(path);
	struct dirent* entry;
	char* name = "votes.txt";
	int found = 0;
	while ((entry = readdir(dir))!= NULL) {
		if (strcmp(entry->d_name, name) == 0) {
			// Found the input file, votes.txt
			found = 1;
			break;
		}
	}
	if (found == 0) {
		printf("No more subdirectories (i.e. this should be a leaf node), but the input file, votes.txt, was not found\n");
		exit(1);
	}

	// Open votes.txt
	char file[strlen(path)+strlen(entry->d_name)+2];
	file[0] = '\0';
	strcat(file, path);
	strcat(file, "/");
	strcat(file, entry->d_name);
	int input = open(file, O_RDONLY);
	if (input < 0) {
		printf("Failed to open votes.txt\n");
		exit(1);
	}
	char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
	int r = read(input, buf, MAX_IO_BUFFER_SIZE);
	if (r < 0) {
		printf("Failed to read votes.txt\n");
		exit(1);
	}

	// Number of votes (this number may be larger than the number of
	// valid votes (such as whitespaces or empty lines)
	int nvotes;
	char** votes;
	nvotes = makeargv(buf, "\n", &votes);

	// Trim starting and ending whitespaces for each vote
	for (int i=0; i<nvotes; i++) {
		votes[i] = trimwhitespace(votes[i]);
	}

	// Each file has at most MAX_IO_BUFFER_SIZE characters, thus each leaf node has
	// at most this many candidate (even if each candidate has name of 1 char and
	// all votes were votes for different candidate)
	struct node* root = (struct node*)malloc(sizeof(struct node)*MAX_IO_BUFFER_SIZE);
	int id = 1;

	// Parse through votes
	for (int i=0; i<nvotes; i++) {
		// Skip invalid votes (original whitespaces or empty lines)
		if (strcmp(votes[i], "")==0 || strcmp(votes[i], "\n")==0) continue;

		// If candidate already exist, increment the votes
		int addvote = 0;
		node_t* temp = root;
		while (temp!=NULL && temp->id != 0) {
			if (temp->name != NULL && strcmp(temp->name, votes[i]) == 0) {
				(temp->vote)++;
				addvote = 1;
				break;
			}
			temp++;
		}

		if (addvote == 1) continue;

		// Candidate does not exist already, initialize a new node
		strcpy(root[id-1].name, votes[i]);
		root[id-1].vote = 1;
		root[id-1].id = id;
		id++;
	}

	// Results: candidate1:count1, candidate2:count2,...
	char out [MAX_IO_BUFFER_SIZE*4];
	out[0] = '\0';
	char tot_vote [MAX_IO_BUFFER_SIZE];
	for (int i=0; i<id-1; i++) {
		strcat(out, root[i].name);
		strcat(out, ":");
		sprintf(tot_vote, "%d", root[i].vote);
		strcat(out, tot_vote);
		if (i<id-2) {
			strcat(out, ",");
		}
	}

	char** dirs;
	int ndir = makeargv(path, "/", &dirs);

	char outfile[strlen(dirs[ndir-1])+4];
	outfile[0] = '\0';
	strcat(outfile, dirs[ndir-1]);
	strcat(outfile, ".txt");

	// Write to the standard output the path to the output file
	strcat(path, "/");
	strcat(path, outfile);
	printf("%s\n", path);

	// Redirect and write the results to the output file
	int output = open(path, O_CREAT|O_WRONLY|O_TRUNC);
	if (output < 0) {
		printf("Failed to open the output file\n");
		exit(1);
	}

	int w = write(output, out, strlen(out));
	if (w < 0) {
		printf("Failed to write to the output file\n");
		exit(1);
	}

	return 0;
}
