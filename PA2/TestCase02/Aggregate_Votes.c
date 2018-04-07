/* login: duxxx336, lixx4793
 * date: 03/07/2018
 * name: Feifan Du, Yuhao Li
 * id: 5099129, 5250438
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "util.h"
// Call leaf_counter for each file


  void getVoteResult(char* path, char* clearName) {
    // The current file has children
    int count = 0;
    DIR* dir = opendir(path);
    struct dirent* dint;
    struct node2* result = (struct node2*)malloc(sizeof(struct node2));
    char output[strlen(path) + strlen(clearName) + 7];
    output[0] = '\0';
    strcat(output, path);
    strcat(output, "/");
    strcat(output, clearName);
    strcat(output, ".txt");
    // Get all vote from it's child directory
    while( (dint = readdir(dir)) != NULL) {
      if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
      // ignore other txt file in current directory
      if(dint->d_type == DT_DIR){
      // e.g region1 -> sub   home/region1 + / +sub + / + sub + .txt + NULL
      char targetFile[strlen(path) + strlen(dint->d_name) * 3 + 4 + 3];
      targetFile[0] = '\0';
      strcat(targetFile, path);
      strcat(targetFile, "/");
      strcat(targetFile, dint->d_name);
      strcat(targetFile, "/");
      strcat(targetFile, dint->d_name);
      strcat(targetFile, ".txt");
      // Uncomment to see the structure
      // printf("Child: %s\nParent: %s, clear: %s\n\n", targetFile, path, clearName);
      chmod(targetFile, 0777);
      int input = open(targetFile, O_RDONLY);
      if(input < 0) {
        perror("Error on open File\n");
      }
      chmod(targetFile, 0777);
      char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);
      ssize_t r = read(input, buf, MAX_IO_BUFFER_SIZE);
      if (r < 0) {
		printf("Failed to read input file\n");
		exit(1);
	  }

      char** candiList;
      char** candiInfo;
      // Get a list of Candidant
      int numCandidate = makeargv(buf, ",", &candiList);
      // Read each candidate
      for(int i = 0; i < numCandidate; i++) {
        struct node2* currentNode = (struct node2*)malloc(sizeof(struct node2));
        // Get candidate information -> Name:vote
        int numInfo = makeargv(candiList[i], ":", &candiInfo);
        if( numInfo == 2) {
         char* candiName = malloc(sizeof(char) * MAX_FILE_NAME_SIZE);
         char* vote = malloc(sizeof(char) * MAX_FILE_NAME_SIZE);
         strcpy(candiName, candiInfo[0]);
         strcpy(vote, candiInfo[1]);
         currentNode->name = candiName;
         currentNode->vote = vote;
         // Append the candidate into result lined list
         if(result->name == NULL){
             struct node2* temp = (struct node2*)malloc(sizeof(struct node2));
             temp->vote = currentNode->vote;
             temp->name = currentNode->name;
             // char* m = malloc(sizeof(char) * MAX_FILE_NAME_SIZE);
            //  printf("%s", temp->vote);
             result = temp;
         } else {
           append(result, currentNode);
         }
        }
        }
        close(input);
      }   //  end of if type = DIR
  }       // End of while loop.

  // viewNode(result);
  // Writing information in node to txt file
    struct node2* cN = (struct node2*)malloc(sizeof(struct node2));
    cN = result;
    chmod(output, 0777);
    int fd = open(output, O_CREAT | O_RDWR | O_TRUNC);
    if( fd < 0) {
      perror("Error on open File\n");
    }
    chmod(output, 0777);
    char* str = malloc(sizeof(char) * MAX_IO_BUFFER_SIZE);
    str[0] = '\0';
    while(cN != NULL){
      if(count != 0){
        strcat(str, ",");
      } else {
        count = 1;
      }
      strcat(str, cN->name);
      strcat(str, ":");
      strcat(str, cN->vote);
      cN = cN->next;
    }
    write(fd, str, strlen(str));
    close(fd);
    return;

}




void getAggCount(char* path){
  // If current directory is leaf node
  if(isLeafNode(path) > 0) {

    // Change stander output to pipe
    int fd[2];
    if(pipe(fd) == -1) {
      perror("Pipe Fail!\n");
    }
    dup2(fd[1],STDOUT_FILENO);
    execl("Leaf_Counter", "Leaf_Counter", path, NULL);
    perror("Fail on Executing\n");
  } else {
  // current directory is not leaf-node
  DIR* dir = opendir(path);
  struct dirent* dint;
  // int count = 0;
  pid_t id;
    while((dint = readdir(dir)) != NULL){
      // Add child
      if(!strcmp(".", dint->d_name) || !strcmp("..",dint->d_name)) continue;
      if(dint->d_type != DT_DIR)continue;
      char nextPath[strlen(path) + strlen(dint->d_name) + 2];
      nextPath[0] = '\0';
      id = fork();
      if(id > 0) {        // if It is parent
        wait(NULL);
        strcat(nextPath, path);
        strcat(nextPath, "/");
        strcat(nextPath, dint->d_name);
        if(isLeafNode(nextPath) == 0 ) {
        getVoteResult(nextPath, dint->d_name);

        }

      } else if( id == 0) {
        strcat(nextPath, path);
        strcat(nextPath, "/");
        strcat(nextPath, dint->d_name);
        // Recursively call getAggCOunt
        getAggCount(nextPath);
        exit(0);
      } else {
        perror("Error on fork");
        exit(1);
      }
    }

  }
}



int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Wrong number of args, expected 1, given %d", argc - 1);
		exit(1);
	}

  if (isLeafNode(argv[1]) == -1) {
	  // failed to open the path
	  exit(1);
  }

  // When the program is called on a leaf node
  if(isLeafNode(argv[1])) {
    char** pathSegment = malloc(sizeof(char*) *  MAX_FILE_NAME_SIZE);
    int sep = makeargv(argv[1], "/", &pathSegment);
    printf("%s", argv[1]);
    printf("/%s.txt\n", pathSegment[sep - 1]);
  }

  getAggCount(argv[1]);
  char** pathSegment = malloc(sizeof(char*) *  MAX_FILE_NAME_SIZE);
  int sep = makeargv(argv[1], "/", &pathSegment);
  if(sep == 1) {
   getVoteResult(argv[1], argv[1]);
   printf("%s", argv[1]);
   printf("/%s.txt\n", argv[1]);
 } else {
   getVoteResult(argv[1], pathSegment[sep - 1]);
   printf("%s", argv[1]);
   printf("/%s.txt\n", pathSegment[sep - 1]);
 }
}
