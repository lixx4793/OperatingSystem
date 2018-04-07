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

void initStructure(char* dagFile, const char* opRoot) {
  int r;
  char* buf = malloc(sizeof(char)*MAX_IO_BUFFER_SIZE);

  // Unknown file size, keep reading input until 0 bytes are readed

  // How should I determine
  //  the max_to_Buffer_SIze?  if may not even reach the size of a node name??????????????????????s
	while( (r = read(input, buf, MAX_IO_BUFFER_SIZE)) > 0){
	if (r < 0) {
		printf("Failed to read votes.txt\n");
		exit(1);
	}

  // Str
	char** content;
	int nLines = makeargv(buf, "\n", &content);
	// Trim starting and ending whitespaces for each line
	for (int i=0; i<nlines; i++) {
		votes[i] = trimwhitespace(votes[i]);
    printf("The content is: --%s", content[i]);
	}
}


}


int main(int argc, char **argv){

	if (argc != 4) {
		perror("Wrong number of input arguments\n");
		exit(0);
	}
  const char* dagFile = argv[1];
  const char* inputRoot = argv[2];
  const char* outputRoot = argv[3];
  // find "DAG.txt"
  struct dirent* dint = initStructure(dagFile, outputRoot);

  }



}
