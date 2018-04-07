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

// 1. Set up a file structure and relation according to input dir
// 2. build output directory and file according to  the Structure
// 3. Enqueue  and Dequeue to caculate the total counts
struct pathStu* initStructure(char* dagFile, const char* opRoot, struct pathStu* mainNode) {
  int input = open(dagFile, O_RDONLY);
  if (input < 0) {
    printf("Failed to open dragram file\n");
    exit(0);
  }
  int r;
  char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
  // Unknown file size, keep reading input until 0 bytes are readed
              // How should I determine
              //  the max_to_Buffer_SIze?  if may not even reach the size of a node name??????????????????????s
	while(( r = read(input, buf, MAX_IO_BUFFER_SIZE)) > 0) {
	if (r < 0) {
		printf("Failed to read dagram file\n");
		exit(1);
	}
  // Str
	char** content;
	int nlines = makeargv(buf, "\n", &content);
	// Trim starting and ending whitespaces for each line
	for (int i=0; i<nlines; i++) {
		content[i] = trimwhitespace(content[i]);

    char** line;

    int nNodes = makeargv(content[i], ":", &line);
    printf("content is: %s, size is: %ld\n",content[i], strlen(content[i]));
    // The root node
    struct pathStu* lineNode = (struct pathStu*)malloc(sizeof(struct pathStu));
    char* name = malloc(sizeof(char) * strlen(line[0]));
    char** childName = malloc(sizeof(char*) * nNodes);
    name = line[0];
    lineNode->name = name;
    lineNode->numChild = nNodes - 1;
    lineNode->childName = childName;
    if(mainNode == NULL) {
      mainNode = lineNode;
    } else {
  // !!! when the node already in mainNOde, only reset existing node child name, and child num
  // in util h. and not append again
        append(mainNode, lineNode);
    }
    for(int j = 1; j < nNodes; j++) {
      struct pathStu* subNode = (struct pathStu*)malloc(sizeof(struct pathStu));
      char* sName = malloc(sizeof(char) * strlen(line[j]));
      sName = line[j];
      subNode->name = sName;
      childName[j-1] = line[j];            // fill child name for it's parent
      subNode->parentName = name;        // fill parent name
      // if it's also a parent node, child num and childName will be add from the code above j= 0
        append(mainNode, subNode);
    }
	}
}
  return mainNode;
}


int main(int argc, char **argv){
	if (argc != 4) {
		perror("Wrong number of input arguments\n");
		exit(0);
	}
  char* dagFile = argv[1];
  const char* inputRoot = argv[2];
  const char* outputRoot = argv[3];
  // find "DAG.txt"
  struct pathStu* mainNode;
  mainNode = initStructure(dagFile, outputRoot, mainNode);
  


  }
