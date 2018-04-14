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
	char* output;
	struct voteNode* votes;
	struct pathStu* next;
} pathStu_t;


typedef struct voteNode {
	char* name;
	int vote;
	struct voteNode* next;
} voteNode_t;


typedef struct queue {
	char* name;
	struct queue* next;
} queue_t;


typedef struct argu {
	struct queue* cq;
	pthread_mutex_t* mutex;
} argu_t;



struct pathStu* findRoot(struct pathStu* a) {
	struct pathStu* p = (struct pathStu*)malloc(sizeof(struct pathStu));
	p = a;
	while(p != NULL) {
		if( p->parentName ==NULL) {
			return p;
		}
		p = p->next;
	}
	return NULL;
}






// This function add an element into queue q
void enqueue(struct queue* q, char* name) {
	struct queue* current = (struct queue*)malloc(sizeof(struct queue));
	struct queue* newnode = (struct queue*)malloc(sizeof(struct queue));
	current = q;
	while(current->next) {
		current = current->next;
	}
	newnode->name = name;
	current->next = newnode;
}


// This function remove a element and return the name
char* dequeue(struct queue* q) {
	char* name = malloc(sizeof(char) * strlen(q->name));
	strcpy(name, q->name);
	if(!(q->next)) {
		q = NULL;
	} else {
		q->name = q->next->name;
		q->next = q->next->next;
	}
	// printf("--%s is dequeued from mainQueue\n", name);
	return name;
}

void viewQueue(struct queue* q) {
	struct queue* temp  = (struct queue*)malloc(sizeof(struct queue));
	temp = q;
	while(temp != NULL) {
		printf("Queue-----: %s\n", temp->name);
		temp = temp->next;
	}
	free(temp);
}


struct pathStu* findNodeByName(struct pathStu* p, char* name) {
	if( p == NULL) return NULL;
	struct pathStu* temp  = (struct pathStu*)malloc(sizeof(struct pathStu));
	temp = p;
	while( temp != NULL) {
		if(!strcmp(temp->name, name)) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}



// Recursively Initalize for child output path for all nodes that have children
void recAdd(struct pathStu* path) {
	struct pathStu* temp  = (struct pathStu*)malloc(sizeof(struct pathStu));
	temp = path;
	for(int i = 0; i < temp->numChild; i++) {
		struct pathStu* childTemp = findNodeByName(path, temp->childName[i]);
		if(childTemp == NULL) return;
		char* append = malloc(sizeof(char) * (strlen(childTemp->name) + 5 ) );
		append = childTemp->name;
		char* output = malloc(sizeof(char) * ( strlen(temp->output)
		 + strlen(childTemp->name) + 50 ));
		 strcat(output, temp->output);
		 strcat(output, "/");
		 strcat(output, append);
		 childTemp->output = output;


		 DIR* dir = opendir(output);
		 if(dir){

		 } else {
		 int d = mkdir(output, 0777);
		 if( d < 0) {
			 perror("Unable to Create Directory");
			 exit(0);
		 }
	 }


		 if(childTemp->numChild > 0) {
			 struct pathStu* temp2  = (struct pathStu*)malloc(sizeof(struct pathStu));
			 temp2 = childTemp;
			 recAdd(temp2);
		 }
	}
}




//  Add output directory to the root file
void addOutputDir(struct pathStu* p, char* output) {
	if(p == NULL) return;
	struct pathStu* temp  = (struct pathStu*)malloc(sizeof(struct pathStu));
	temp = p;
	while(temp != NULL) {
		if(temp->parentName == NULL) {
			//  Initialize the output file for the root node
			char* op = malloc(sizeof(char) * (strlen(output) + strlen(temp->name) + 50));
			strcat(op, output);
			strcat(op, "/");
			strcat(op, temp->name);
			temp->output =op;


			DIR* dir2 = opendir(temp->output);
			if(dir2) {}
			else {
			int d = mkdir(temp->output, 0777);
			if( d < 0) {
				perror("Unable to Create Directory");
				exit(0);
			}
		}
			// Initialize the output file for the children directory
			for(int i = 0; i < temp->numChild; i++) {
				struct pathStu* childPath = (struct pathStu*)malloc(sizeof(struct pathStu));
				childPath = findNodeByName(p, temp->childName[i]);
				char* outputPath = malloc(sizeof(char) *
				(strlen(temp->output) + strlen(childPath->name) + 50));
				strcat(outputPath, temp->output);
				strcat(outputPath, "/");
				strcat(outputPath, childPath->name);
				childPath->output = outputPath;
				DIR* dir = opendir(outputPath);
				if(dir){

				} else {
				int d2 = mkdir(outputPath, 0777);
				if( d2 < 0) {
					perror("Unable to Create Directory");
					exit(0);
				}
			}
				if(childPath->numChild != 0) {
					recAdd(childPath);
				}
			}
			return;
		}
		temp = temp->next;
	}
}

// This function append voteNode
void appendVote(struct voteNode* a, struct voteNode* b){
	struct voteNode* current = (struct voteNode*)malloc(sizeof(struct voteNode));
	struct voteNode* newNode = (struct voteNode*)malloc(sizeof(struct voteNode));
	newNode = b;
	current = a;
	while(current->next != NULL) {
		if(!strcmp(b->name, current->name)) {
			current->vote += b->vote;
			return;
		}
			current = current->next;
		}

		// For the last Node!!!
		if(!strcmp(b->name, current->name)) {
			// printf("---------%s: %d + %d: \n", current->name, current->vote, b->vote);
			current->vote += b->vote;
			return;
		}
		// // else set a new node
		current->next = b;
}

// This funciton append for pathStu
void append(struct pathStu* a, struct pathStu* b){

	if(b==NULL) return;
	struct pathStu* current =(struct pathStu*)malloc(sizeof(struct pathStu));
	current = a;

	while(current->next != NULL) {

		//  merge the vale of childName and number of child to original node
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

// The finction print out the information stored in a node structure
void viewNode(struct pathStu* a) {
	if(a == NULL) {
		printf("-- NULL --\n");
	}
	struct pathStu* current = (struct pathStu*)malloc(sizeof(struct pathStu));
	current = a;
	while(current != NULL) {
		printf("\nNode------: %s, numChild: %d, parent: %s, output: %s\n", current->name, current->numChild, current->parentName, current->output);
		if(current->numChild != 0) {
			for(int i =0; i < current->numChild; i++){
			printf("			Child: %s\n", current->childName[i]);
		}
		}
		current = current->next;
	}
}

void viewVote(struct voteNode* a) {
	struct voteNode* current = a;
	while(current!= NULL) {
		printf("The Name is: %s, the votes is: %d\n", current->name, current->vote);
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


char* decrypt(char* input) {
	char* copy = input;
	int len = strlen(copy);
	char* output = malloc(sizeof(char)*len);
	for (int i = 0; i < len; i++) {
		int code = (int)(copy[i]);
		if ((code >= 65 && code <= 90) || (code >= 97 && code <= 122)) {
			// only decrypt english alphabets
			int temp = code + 2;
			if (temp == 91 || temp == 92 || temp == 123 || temp == 124) {
				// YZ->AB, yz->ab
				temp = temp - 26;
			}
			output[i] = (char)temp;
		} else { output[i] = copy[i]; }
	}
	return output;
}


// This function return the number of element in a directory
int dirSize(const char* path, struct pathStu* node){
	DIR* dir = opendir(path);
	if(dir == NULL) {
		perror("Unable to open the directory");
		exit(0);
	}
	struct dirent* dint;
	int count = 0 ;
		while((dint = readdir(dir)) != NULL){
			if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
			if(findNodeByName(node, dint->d_name)){ count++;}
		}
		return count;
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


void recDelete(char* root) {
	struct stat sb;
	if (stat(root, &sb) == 0 && S_ISDIR(sb.st_mode))
	{
		DIR *d = opendir(root);
		struct dirent* dint;
	    while((dint = readdir(d)) != NULL){
				char* newName = malloc(sizeof(char) * (strlen(root) + strlen(dint->d_name) + 5));
				strcat(newName, root);
				strcat(newName, "/");
				strcat(newName, dint->d_name);
	      if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
				if(dint->d_type == DT_DIR) {recDelete(newName);
				}
	      if(dint->d_type != DT_DIR && strcmp(dint->d_name, "log.txt")) {
					if(remove(newName) == 0) {
					} else {
						printf("Unable to delete file ~~~~~~~ %s\n", dint->d_name);
					}
				}
		}
	}
}
