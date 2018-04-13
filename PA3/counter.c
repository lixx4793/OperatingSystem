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
#include <pthread.h>
#include "util.h"

struct pathStu* mainNode;
char* Log;
char* input;

//  This function write log sync, remove the name from queue, decrp
void* executeChildThread(void* argu) {

  struct argu* arg = (struct argu *)malloc(sizeof(struct argu));
  arg = (struct argu*)argu;
  pthread_t threadB = pthread_self();
  char* logEnd;
  char* inputName;
  char* outputName;
  char* name;
  char* start;
  char* threadId = malloc(sizeof(char) * 15);

  // Critical Section
    int targetFile = open(Log, O_WRONLY | O_APPEND);
  pthread_mutex_lock(arg->mutex);
  if (!targetFile) {
    perror("Failed to open log.txt\n");
    exit(0);
  }
    name = dequeue(arg->cq);
    inputName = malloc(sizeof(char) * (strlen(name) + strlen(input) + 5));
    inputName[0] = '\0';
    strcat(inputName, input);
    strcat(inputName, "/");
    strcat(inputName, name);
    sprintf(threadId, "%lu",pthread_self());
    start = malloc(sizeof(char) * (strlen(threadId) + strlen(name) + 8));
    start[0] = '\0';
    strcat(start, name);
    strcat(start, ":");
    strcat(start, threadId);
    strcat(start, ":start\n");
    write(targetFile, start, strlen(start));

    // fprintf(targetFile, "%s:%lu:start\n", name, pthread_self());
    pthread_mutex_unlock(arg->mutex);
    close(targetFile);
    // Critical Section
    // Read file
  //
    struct voteNode* rootVote;
    char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
    int r;
    int input = open(inputName, O_RDONLY);
    if (input < 0) {
      printf("Failed to open dragram file\n");
      exit(0);
    }

    while((r = read(input, buf, MAX_IO_BUFFER_SIZE)) > 0) {
      char** content;
      int nlines = makeargv(buf, "\n", &content);
      for (int i = 0; i<nlines; i++) {
    		content[i] = trimwhitespace(content[i]);
          struct voteNode* ballot = (struct voteNode*)malloc(sizeof(struct voteNode));

          char* candidate = malloc(sizeof(char) * (strlen(content[i]) + 10));
          candidate = content[i];
          ballot->vote = 1;
          ballot->name = decrypt(candidate);
          printf("undectped name: %s\n", candidate);
                // TODO: Add Decrpt here
          if(rootVote == NULL) {
            rootVote = (struct voteNode*)malloc(sizeof(struct voteNode));
            rootVote->name = ballot->name;
            rootVote->vote = ballot->vote;
          } else if(atoi(ballot->name) != 0) {
            continue;
          } else {
            appendVote(rootVote, ballot);
          }
      }
    }
    if(r < 0) {
      perror("Reading Error in thread");
      exit(0);
    }
    close(input);
    viewVote(rootVote);
  int outputSize = strlen(findNodeByName(mainNode, name)->name) + strlen(name) + 10;
  outputName = malloc(sizeof(char) * outputSize);
  outputName[0] = '\0';
  strcat(outputName, findNodeByName(mainNode, name)->output);
  strcat(outputName, "/");
  strcat(outputName, name);
  strcat(outputName, ".txt");
  struct voteNode* temp = rootVote;
  FILE* leafTxt = fopen(outputName, "w");
  if (!leafTxt) {
    perror("Failed to open input file\n");
    exit(0);
  }
  // Writing leaf files
  while(temp != NULL) {
    if(atoi(temp->name) !=0 || temp->name[0] == ':') {
      temp = temp->next;
    } else {
    fprintf(leafTxt, "%s:%d\n", temp->name, temp->vote);
    printf("------------Name: %s from %s\n", temp->name, name);
    temp = temp->next;
    }
  }
  fclose(leafTxt);

  char* parentName = findNodeByName(mainNode, name)->parentName;


  pthread_mutex_lock(arg->mutex);
  while(parentName != NULL) {
    struct voteNode* temp2 = rootVote;
    struct pathStu* parentNode = findNodeByName(mainNode, parentName);
  //  Try to open the file
    // Write File
    char* aggName = malloc(sizeof(char) *
      (strlen(parentName) + strlen(parentNode->output) + 10));
    aggName[0] = '\0';
    strcat(aggName, parentNode->output);
    strcat(aggName, "/");
    strcat(aggName, parentName);
    strcat(aggName, ".txt");
    // printf("Agg file is: %s\n", aggName);

    if(access(aggName, F_OK) != -1) {   //  File exist, Read and Write

      int input = open(aggName, O_RDONLY);
      if (input < 0) {
        printf("Failed to open dragram file\n");
        exit(0);
      }
      int r;
      char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
      struct voteNode* readVote = (struct voteNode*) malloc(sizeof(struct voteNode));
      r = read(input, buf, MAX_IO_BUFFER_SIZE);
      printf("adding %s from %s ~~~\n", aggName, name);
        char** content;
        int nlines = makeargv(buf, "\n", &content);
        for(int j = 0; j < nlines; j++){
          char** value;
          int k;
          if( k = makeargv(content[j], ":", &value) != 2) {
            // perror("Error on reading file");
            // exit(0);
          } else {
          struct voteNode* element = (struct voteNode*)malloc(sizeof(struct voteNode));
          element->name = malloc(sizeof(char) * (strlen(value[0])));
          element->name = value[0];
          element->vote = atoi(value[1]);
          if(readVote->name == NULL) {
            readVote = element;
          } else {
            appendVote(readVote, element);
          }
        }
        }
        close(input);
        while(temp2 != NULL) {
          appendVote(readVote, temp2);
          temp2 = temp2->next;
        }
        FILE* agg = fopen(aggName, "w");
        if (!agg) {
          perror("Failed to open input file\n");
          exit(0);
        }
        while(readVote != NULL) {
          fprintf(agg,"%s:%d\n", readVote->name, readVote->vote);
          readVote=readVote->next;
        }
        fclose(agg);

    } else {    // File not exist, Write to file directly
      FILE* agg = fopen(aggName, "w");
      if (!agg) {
        perror("Failed to open input file\n");
        exit(0);
      }
      while(temp2 != NULL) {
        fprintf(agg, "%s:%d\n", temp2->name, temp2->vote);
        temp2 = temp2->next;
      }
      fclose(agg);
    }
    parentName = findNodeByName(mainNode, parentName)->parentName;
    // Critial Section End
  }
  pthread_mutex_unlock(arg->mutex);

  FILE* targetFile2 = fopen(Log, "a");
  pthread_mutex_lock(arg->mutex);
  if(targetFile2 < 0) {
    perror("Unable to open the file");
    exit(0);
  }
  fprintf(targetFile2, "%s:%ld:end\n", name, pthread_self());
  pthread_mutex_unlock(arg->mutex);
  fclose(targetFile2);
}


// This function init the queue
struct queue* initQueue(const char* inputDir) {
  struct queue* mainQueue=(struct queue*) malloc(sizeof(struct queue));
  DIR* dir = opendir(inputDir);
  if(dir == NULL) {
    perror("Unable to find input directory");
    exit(0);
  }

  struct dirent* dint;
    while((dint = readdir(dir)) != NULL){
      if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
      if(dint->d_type == DT_DIR)continue;
      char* name = malloc(sizeof(char) * strlen(dint->d_name)) ;
      name[0] = '\0';
      strcat(name, dint->d_name);
      if(mainQueue->name == NULL) {
        mainQueue->name = name;
      } else {
        if(findNodeByName(mainNode, name) != NULL){
            enqueue(mainQueue, name);
        }
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
    char* name = malloc(sizeof(char) * (strlen(line[0]) + 1));
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
      char* sName = malloc(sizeof(char) * (strlen(line[j]) + 1));
      sName = line[j];
      subNode->name = sName;
      childName[j-1] = line[j];            // fill child name for it's parent
      subNode->parentName = name;        // fill parent name
      // if it's also a parent node, child num and childName will be add from the code above j= 0
        append(mainNode, subNode);
    }
	}
} if (r < 0) {
  printf("Failed to read dagram file\n");
  exit(1);
}
  return mainNode;
}


int main(int argc, char **argv){
	if (argc != 4) {
		perror("Wrong number of input arguments\n");
		exit(0);
	}
  char* dagFile = argv[1];
  input = argv[2];
  char* outputRoot = argv[3];
  recDelete(outputRoot);
  mainNode = initStructure(dagFile);
  // Create output directory
  DIR* dir = opendir(outputRoot);
  if(dir){
      // The directory already exist.
  } else {
  int d = mkdir(outputRoot, 0777);
  if( d < 0) {
    perror("Unable to Create Directory");
    exit(0);
  }
}
// Create log.txt file
  char* logFile = malloc(sizeof(char) * (strlen(outputRoot) + 20));
  logFile[0] = '\0';
  strcat(logFile, outputRoot);
  strcat(logFile, "/log.txt");
  Log = logFile;
  int fd = open(logFile, O_CREAT );
  chmod(logFile, 0777);
  if(fd < 0) {
    perror("Unable to open or create log file");
    exit(0);
  }
  close(fd);
  // If the log file already exited, clear the file content to empty
  FILE* targetFile = fopen(logFile, "w");
  if (!targetFile) {
    perror("Failed to open log.txt\n");
    exit(0);
  }
  fclose(targetFile);
  // Delete output file if existed

  // Setup directory structure
  addOutputDir(mainNode, outputRoot);
  // viewNode(mainNode);
  // 2. Determine the size of queue
  int count = dirSize(input, mainNode);
  struct queue* processQueue = initQueue(input);
  // viewQueue(processQueue);

  // Set up muti thread variables
  struct argu* arg = (struct argu*)malloc(sizeof(struct argu));
  arg->cq = processQueue;
  arg->mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(arg->mutex, NULL);
  pthread_t ids[count];
  for(int i = 0; i < count; i++) {
    pthread_create(&ids[i], NULL, executeChildThread, (void*)arg);
  }
  for(int j = 0; j< count; j++) {
    pthread_join(ids[j], NULL);
  }

  struct pathStu* pN = findRoot(mainNode);
  char* file = malloc(sizeof(char) * (strlen(pN->output) + strlen(pN->name) + 10));
  file[0] = '\0';
  strcat(file, pN->output);
  strcat(file, "/");
  strcat(file, pN->name);
  strcat(file, ".txt");
  char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
  int r;
  int input2 = open(file, O_RDONLY);
  if (input2 < 0) {
    printf("Failed to open file\n");
    exit(0);
  }
  char* winner;
  char* votes;
  while((r = read(input2, buf, MAX_IO_BUFFER_SIZE)) > 0) {
    char** content;
    int nlines = makeargv(buf, "\n", &content);
    int max = 0;
    for(int k = 0; k < nlines; k++) {
      char** value;
      int sep = makeargv(content[k], ":", &value);
      if(sep == 2){
      if(atoi(value[1]) > max) {
        max = atoi(value[1]);
        votes = value[1];
        winner = value[0];
      }
    }
    }
  }
  close(input2);

  int input3 = open(file, O_WRONLY | O_APPEND);
  char* result = malloc(sizeof(char) * (strlen(winner) + 8));
  result[0] = '\0';
  strcat(result, "WINNER:");
  strcat(result, winner);
  strcat(result, "\n");
  write(input3, result, strlen(result));
  close(input3);
}
