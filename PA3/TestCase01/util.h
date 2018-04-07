/* login: duxxx336, lixx4793
 *
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

/* The maximum amount of bytes for a file name */
#define MAX_FILE_NAME_SIZE 255

/* The maximum amount of bytes for each I/O operation */
#define MAX_IO_BUFFER_SIZE 1024

// Structure for every node in linked list
typedef struct pathStu {
	char* name;
	int numChild;
	char** childName;
	char* parentName;
	struct voteNode* votes;
	struct pathStu* next;
} pathStu_t;


typedef struct voteNode {
	char* name;
	char* vote;
	struct voteNode* next;
} voteNode_t;

void appendVote(struct voteNode* a, struct voteNode* b){
	if(b==NULL) return;
	struct voteNode* current =(struct voteNode*)malloc(sizeof(struct voteNode));
	struct voteNode* newNode = (struct voteNode*)malloc(sizeof(struct voteNode));
	current = a;
	newNode = b;
	while(current->next != NULL){
		if(!strcmp(newNode->name, current->name)) {
			int vote = atoi(newNode->vote) + atoi(current->vote);
			char total[10];
			sprintf(total, "%d", vote);
			strcpy(current->vote, total);
			return;
		}
			current = current->next;
		}
		// For the last Node!!!
		if(!strcmp(newNode->name, current->name)) {
			int vote = atoi(newNode->vote) + atoi(current->vote);
			char total[10];
			sprintf(total, "%d", vote);
			strcpy(current->vote, total);
			return;
		}
		// else set a new node
		current->next = b;
}

void append(struct pathStu* a, struct pathStu* b){
	if(b==NULL) return;
	struct pathStu* current =(struct pathStu*)malloc(sizeof(struct pathStu));
	current = a;
	while(current->next != NULL) {
		//  Do nothing if the node is already existed.
		if(!strcmp(b->name, current->name)) {
			current->childName = b->childName;
			current->numChild = b->numChild;
			return;
		}
			current = current->next;
		}
		if(!strcmp(b->name, current->name)) {
			current->childName = b->childName;
			current->numChild = b->numChild;
			return;
		}
		current->next = b;
}


void viewNode(struct pathStu* a) {
	if(a == NULL) {
		printf("-- NULL --\n");
	}
	struct pathStu* current = (struct pathStu*)malloc(sizeof(struct pathStu));
	current = a;
	while(current != NULL) {
		printf("\nNode------: %s, numChild: %d, parent%s \n", current->name, current->numChild, current->parentName);
		if(current->numChild != 0) {
			for(int i =0; i < current->numChild; i++){
			printf("			Child: %s\n", current->childName[i]);
		}
		}
		current = current->next;
	}
}



/**********************************
*
* Taken from Unix Systems Programming, Robbins & Robbins, p37
*
*********************************/
int makeargv(const char *s, const char *delimiters, char ***argvp) {
   int error;
   int i;
   int numtokens;
   const char *snew;
   char *t;

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
   if (strtok(t, delimiters) != NULL)
      for (numtokens = 1; strtok(NULL, delimiters) != NULL; numtokens++) ;

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
      **argvp = strtok(t,delimiters);
      for (i=1; i<numtokens; i++)
         *((*argvp) +i) = strtok(NULL,delimiters);
   }
   *((*argvp) + numtokens) = NULL;
   return numtokens;
}




// This function will make sure one candidate is only appeard once in a node

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
