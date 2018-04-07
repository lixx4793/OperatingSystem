/* login: duxxx336, lixx4793
 * 
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
#include "util.h"
#include <dirent.h>

int main(int argc, char **argv){

	if (argc != 2) {
		printf("Wrong number of input arguments\n");
		return -1;
	}
	
	char* path = argv[1];
	int isleaf = isLeafNode(path);
	if (isleaf == -1) {
		//set by isLeafNode if failed to open directory
		exit(1);
	}
	if (isleaf == 0) {
		printf("Not a leaf node.\n");
		exit(1);
	}
	
	// guaranteed to successfully open the directory at this point
	DIR* dir = opendir(path);
	struct dirent* entry;
	char* name = "votes.txt";
	int found = 0;
	while ((entry = readdir(dir))!= NULL) {
		if (strcmp(entry->d_name, name) == 0) {
			// found the input file
			found = 1;
			break;
		}
	}
	if (found == 0) {
		printf("Input file, votes.txt, was not found");
		exit(1);
	}
	
	char file[strlen(path)+strlen(entry->d_name)+2];
	file[0] = '\0';
	strcat(file, path);
	strcat(file, "/");
	strcat(file, entry->d_name);

	int input = open(file, O_RDONLY);
	char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
	read(input, buf, MAX_IO_BUFFER_SIZE);

	// number of votes
	int nvotes;
	char** votes;
	nvotes = makeargv(buf, "\n", &votes);
	
	for (int i=0; i<nvotes; i++) {
		//printf("Trimming vote ---%s---\n", votes[i]);
		votes[i] = trimwhitespace(votes[i]);
		//printf("Trimmed vote ---%s---\n", votes[i]);
	}
	
	// each file has at most MAX_IO_BUFFER_SIZE characters, thus each leaf node has
	// at most this many candidate (even if each candidate has name of 1 char and 
	// all votes were votes for different candidate)
	struct node* root = (struct node*)malloc(sizeof(struct node)*MAX_IO_BUFFER_SIZE);
	int id = 1;
	
	// parse through votes
	for (int i=0; i<nvotes; i++) {
		//skip empty lines
		if (strcmp(votes[i], " ")==0 || strcmp(votes[i], "\n")==0) continue;
		
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
		
		strcpy(root[id-1].name, votes[i]);
		root[id-1].vote = 1;
		root[id-1].id = id;
		id++;
	}
	
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
	
	strcat(path, "/");
	strcat(path, outfile);
	printf("%s\n", path);

	int output = open(path, O_CREAT|O_WRONLY|O_TRUNC);
	dup2(output, STDOUT_FILENO);
	printf("%s\n", out);

	return 0;
}
