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
typedef struct node2 {
	char* name;
	char* vote;
	struct node2* next;
} node2_t;

void append(struct node2* a, struct node2* b){
	if(b==NULL) return;
	struct node2* current =(struct node2*)malloc(sizeof(struct node2));
	struct node2* newNode = (struct node2*)malloc(sizeof(struct node2));
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


void viewNode(struct node2* a){
	if(a == NULL) {
		printf("-- NULL --\n");
	}
	struct node2* current = (struct node2*)malloc(sizeof(struct node2));
	current = a;
	while(current != NULL) {
		printf("\nNode------:%s, %s+++\n", current->name, current->vote);
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

int isLeafNode(char* path) {
	DIR* dir = opendir(path);
	if (dir == NULL) {
		return 1;
	}

	struct dirent* entry;
	while ((entry = readdir(dir))!= NULL) {
		if (strcmp(entry->d_name, ".")==0 || strcmp(entry->d_name, "..")==0) {
			continue;
		}
		if (entry->d_type == DT_DIR) {
			return 0; // Not a leaf node
		}
	}

	return 1; // Is a leaf node
}
