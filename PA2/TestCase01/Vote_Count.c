#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include "util.h"

int main(int argc, char** argv) {
  char* pathName = malloc(sizeof(char) *  MAX_FILE_NAME_SIZE);

  // Check input error
	if (argc > 2) {
		printf("Wrong number of args, expected 1, given %d", argc - 1);
		exit(1);
	}

  // Determine current directory name if no input
  if(argc == 1) {
    char** pathSegment = malloc(sizeof(char*) * MAX_FILE_NAME_SIZE);
    getcwd(pathName,  MAX_FILE_NAME_SIZE);
    int sep = makeargv(pathName, "/", &pathSegment);
    pathName = pathSegment[ sep - 1];
  } else {
    pathName = argv[1];
  }
  printf("targetPath is: %s\n", pathName);

  // Starting generating file and determine winnner
  pid_t id = fork();
  if( id == 0) {
  execl("Aggregate_Votes", "Aggregate_Votes",pathName,NULL);
  perror("Error on Executing Aggreate_Votes\n");
} else if( id > 0){
  wait(NULL);

  //Get name of targetFile
  char targetFile [MAX_FILE_NAME_SIZE];
  targetFile[0] = '\0';
  strcat(targetFile, pathName);
  strcat(targetFile, "/");
  char** pathSegment = malloc(sizeof(char*) *  MAX_FILE_NAME_SIZE);
  int sep = makeargv(argv[1], "/", &pathSegment);
  if( sep == 0) {
  strcat(targetFile, pathName);
} else {
  strcat(targetFile, pathSegment[sep - 1]);
}
  strcat(targetFile, ".txt");

  // Open The File and READ
  int readFD = open(targetFile, O_RDWR);
  char* buff = malloc(sizeof(char) * MAX_IO_BUFFER_SIZE);
  char* result = malloc(sizeof(char) * MAX_FILE_NAME_SIZE);
  int i = read(readFD, buff, 1024);
  if( i < 0 ) {
    perror("unable to read file\n");
  }


  //Write to buff and write to file
  char * Winner = malloc(sizeof(char) * MAX_FILE_NAME_SIZE);
  char** strHolder = malloc(sizeof(char*) * MAX_IO_BUFFER_SIZE);
  int num = makeargv(buff, ",", &strHolder);
  int max = 0;
  for(int i = 0; i < num; i++) {
    char** strHolder2 = malloc(sizeof(char*) * MAX_IO_BUFFER_SIZE);
    makeargv(strHolder[i], ":", &strHolder2);
    if(atoi(strHolder2[1]) > max) {
      max = atoi(strHolder2[1]);
      Winner = strHolder2[0];
    }
  }
  result[0] = '\0';
  strcat(result, "\n");
  strcat(result, "Winner:");
  strcat(result, Winner);
  int j = write(readFD, result, strlen(result));
  if(j < 0) {
    perror("Unable to write file\n");
  }
  close(readFD);
} else {
  perror(" Error on fork\n");
  exit(1);
}
}
