/* login: duxxx336, lixx4793
 * date: 04/30/2018
 * name: Feifan Du, Yuhao Li
 * id: 5099129, 5250438
 */
#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
// max number of candidates
// max number of regions
// max length of candidate name
#define MAX_NUMBER 100

// max length of region name
#define MAX_REG_NAME 15

// max length of the message being sent through socket
#define MAX_IO_BUFFER_SIZE 256

// Structure for candidate in leaf node (used by the server)
typedef struct candidate {
	char* name;
	int vote;
	int id;
} candidate_t;

// Structure for candidate in leaf node (used by the client)
typedef struct candidate2 {
	char name[MAX_NUMBER];
	int vote;
	int id;
} candidate2_t;

// structure for each voting region
typedef struct Region {
  char name[MAX_REG_NAME];
  struct candidate** votes;
  int status;				// 0 init, 1 open, -1 close
  int areaId;
	char parentName[MAX_REG_NAME];
  char** childNames;
} Region_t;

// structure of the argument for the function executed by each thread
typedef struct threadArg {
	int clientfd;
	pthread_mutex_t* mutex;
	char* message;
	char* port;
	char* add;
} threadArg_t;


// find the root node of the structure passed in
struct Region* findRoot(struct Region** stru)
{
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(strcmp(stru[i]->parentName, "") == 0)
		{
		 return stru[i];
	 	}
	}
}

// find a region node by its name
struct Region* findRegion(struct Region** stru, char* name)
{
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(stru[i] == NULL)
		{
			return NULL;
		}
		if(strcmp(stru[i]->name, name) == 0 )
		{
			return stru[i];
		}
	}
}

// Taken from Unix Systems Programming, Robbins & Robbins, p37
// modified to be thread-safe
// i.e. use of strtok_r instead of strtok
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;
   char *saveptr1, *saveptr2;

   if ((s == NULL) || (delimiters == NULL) || (argvp == NULL)) {
      errno = EINVAL;
      return -1;
   }
   *argvp = NULL;
   snew = s + strspn(s, delimiters);
   if ((t = malloc(strlen(snew) + 1)) == NULL)
      return -1;
   strcpy(t,snew);
   numtokens = 0;
   if (strtok_r(t, delimiters, &saveptr1) != NULL)
      for (numtokens = 1; strtok_r(NULL, delimiters, &saveptr1) != NULL; numtokens++) ;

   if ((*argvp = malloc((numtokens + 1)*sizeof(char *))) == NULL) {
      error = errno;
      free(t);
      errno = error;
      return -1;
   }
   if (numtokens == 0)
      free(t);
   else {
      strcpy(t,snew);
      **argvp = strtok_r(t,delimiters, &saveptr2);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok_r(NULL,delimiters, &saveptr2);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}

// recursively open poll from a region
void recOpen(struct Region* region, struct Region** main)
{
	region->status = 1;
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(region->childNames[i] == NULL) break;
		struct Region* temp = (struct Region*)malloc(sizeof(struct Region));
		temp = findRegion(main, region->childNames[i]);
		if(temp == NULL)
		{

		}
		else
		{
			if(temp->childNames[0] != NULL)
			{
				recOpen(temp, main);
			}
			else
			{
				temp->status = 1;
			}
		}
	}
}

// recursively close poll from a region
void recClose(struct Region* region, struct Region** main)
{
	region->status = -1;
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(region->childNames[i] == NULL) break;
		struct Region* temp = (struct Region*)malloc(sizeof(struct Region));
		temp = findRegion(main, region->childNames[i]);
		if(temp == NULL)
		{
		}
		else
		{
			if(temp->childNames[0] != NULL)
			{
				recClose(temp, main);
			}
			else
			{
				temp->status = -1;
			}
		}
	}
}

// append vote to a candidate
struct candidate** appendVotes(struct candidate** res, int vote, char* name)
{
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(res[i] == NULL)
		{
			struct candidate* cd = (struct candidate*)malloc(sizeof(struct candidate));
			cd->vote = vote;
			cd->name = malloc(sizeof(char) * strlen(name));
			strcpy(cd->name, name);
			res[i] = cd;
			break;
		}
		else if(strcmp(res[i]->name, name) == 0)
		{
			res[i]->vote += vote;
			break;
		}
	}

}

// add multiple votes
int addVote(struct candidate** votes, char* buffer)
{
	char** content;
	if(buffer == NULL) return 0;
	int total = makeargv(buffer, "," , &content);
	for(int i =0; i < total; i++)
	{
		char** NV;
		int flag = makeargv(content[i], ":", &NV);
		if(flag != 2)
		{
			return -1;
		}
		char* name = malloc(sizeof(char) * strlen(NV[0]));
		strcpy(name, NV[0]);
		int vote = atoi(NV[1]);
			appendVotes(votes, vote, NV[0]);
	}
	return 0;
}

// delete vote
void deleteEle(struct candidate** ori, int vote, char* name)
{
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(strcmp(ori[i]->name, name) == 0)
		{
			 ori[i]->vote -= vote;
			 break;
		}
	}
}

// check if removing vote(s) is legal, if is illegal then return the name
char* checkValid(struct candidate** ori, int vote, char* name)
{
	for(int i = 0; i < MAX_NUMBER; i++)
	{
		if(ori[i] == NULL)			// Deleting name is not existed
		{
			return name;
		}
		else if(strcmp(ori[i]->name, name) == 0)
		{
			 if((ori[i]->vote - vote)< 0)
			 {
			  	return name;		// illegal substraction
			 }
			 else
			 {
				 	return NULL;
			 }
		}
	}
}

// delete multiple votes, it add all name of candidate, who is invalid, to the array and
// return it. If the array is empty then no error appear
char** deleteVote(struct candidate** votes, char* buffer)
{

	int count = 0;
	char** content;
	char* flag;
	char** error = malloc(sizeof(char*) * MAX_NUMBER);
	int total = makeargv(buffer, "," , &content);
	for(int i =0; i < total; i++)
	{
		char** NV;
		makeargv(content[i], ":", &NV);
	// Check if the request for the candidate is valid, if valid a NULL return  
		flag = checkValid(votes, atoi(NV[1]), NV[0]);
		if(flag != NULL)
		{
			error[count] = NV[0];
			count++;
		}
	}
	if(error[0] == NULL)
	{  			//	No error detected then delete the vote from region
		for(int j = 0; j < total; j++)
		{
			char** NV2;
			makeargv(content[j], ":", &NV2);
			deleteEle(votes, atoi(NV2[1]), NV2[0]);
		}
	}

return error;
}

//  This function apend a region to a existiong region array
//  The new region will be added into array if it' not existed
void appendRegion(struct Region** stru, struct Region* region) {
	for(int i = 0; i < MAX_NUMBER; i++) {
			if(stru[i]->name == NULL) {
				stru[i] = region;
				break;
			}
			if(strcmp(stru[i]->name, region->name) == 0) {
				if(stru[i]->parentName != NULL) {
					stru[i]->childNames = region->childNames;
				}
				else
				{
					stru[i]->parentName == region->parentName;
				}
				break;
			}
	}
}

//	Print out current structure relationship of a region array
void viewStructure(struct Region** root) {
	int count = 0;
	while(root[count]->name != NULL) {
		printf("\n");
		printf("The region name is: %s, parent is: %s, status is: %d\n",
		 	root[count]->name, root[count]->parentName, root[count]->status);

		for(int i = 0; i < 100; i++) {
			if(root[count]->childNames[i] == NULL) break;
			else {
				printf("			child name is: %s\n",root[count]->childNames[i]);
			}
		}
		for(int j =0; j < 100; j++)
		{
			if(root[count]->votes[j] == NULL) break;
			else
			{
				printf("  	--candidate: %s, vote: %d.\n", root[count]->votes[j]->name, root[count]->votes[j]->vote);
			}
		}
		count++;
	}
}

/**********************************
*
* Taken from Unix Systems Programming, Robbins & Robbins, p38
*
*********************************/
void freemakeargv(char **argv) {
   if (argv == NULL)
      return;
   if (*argv != NULL)
      free(*argv);
   free(argv);
}

//  This function trim the space of a line
char *trimwhitespace(char *str) {
  char *end;
  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;

  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}
