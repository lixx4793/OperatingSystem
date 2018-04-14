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

struct pathStu* mainNode;
//  This function write log sync, remove the name from queue, decrp
void executeChildThread(struct queue* pq, struct pathStu* mainNode) {


}


// This function init the queue
struct queue* initQueue(const char* inputDir) {
  struct queue* mainQueue=(struct queue*) malloc(sizeof(struct queue));
  DIR* dir = opendir(inputDir);
  struct dirent* dint;
    while((dint = readdir(dir)) != NULL){
      if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
      if(dint->d_type == DT_DIR)continue;
      char* name = malloc(sizeof(char) * strlen(dint->d_name));
      name = dint->d_name;
      if(mainQueue->name == NULL) {
        mainQueue->name = dint->d_name;
      } else {
      enqueue(mainQueue, name);
    }
  }
  return mainQueue;
}




struct pathStu* initStructure(char* dagFile) {
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

  struct pathStu* mainNode = (struct pathStu*)malloc(sizeof(struct pathStu));
	while(( r = read(input, buf, MAX_IO_BUFFER_SIZE)) > 0) {
	if (r < 0) {
		printf("Failed to read dagram file\n");
		exit(1);
	}
  // Str
	char** content;
	int nlines = makeargv(buf, "\n", &content);
	// Trim starting and ending whitespaces for each line
	for (int i = 0; i<nlines; i++) {
		content[i] = trimwhitespace(content[i]);

    char** line;

    int nNodes = makeargv(content[i], ":", &line);

    // The root node
    struct pathStu* lineNode = (struct pathStu*)malloc(sizeof(struct pathStu));
    char* name = malloc(sizeof(char) * strlen(line[0]));
    char** childName = malloc(sizeof(char*) * nNodes);
    name = line[0];
    lineNode->name = name;
    lineNode->numChild = nNodes - 1;
    lineNode->childName = childName;
    if(i == 0) {
      mainNode = lineNode;
    } else {
  // when the node already in mainNOde, only reset existing node child name, and child num
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
  char* outputRoot = argv[3];

  mainNode = initStructure(dagFile);
  addOutputDir(mainNode, outputRoot);
  viewNode(mainNode);

  // 2. Determine the size of queue
  int count = dirSize(inputRoot);
  struct queue* processQueue = initQueue(inputRoot);
  viewQueue(processQueue);
  pthread_t ids[count];
  for(int i = 0; i < count; i++) {
    // pthread_create(&ids[j], NULL, executeChildThread, (void*)&processQueue);
  }
  for(int j = 0; j< count; j++) {
    // pthread_join(ids[j2], NULL);
  }
  }
