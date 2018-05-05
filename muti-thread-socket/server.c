/* login: duxxx336, lixx4793
 * date: 04/30/2018
 * name: Feifan Du, Yuhao Li
 * id: 5099129, 5250438
 * Extra credits [No]
 */

#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "util.h"
#define NUM_ARGS 2

#define MAX_CONNECTIONS 100

struct Region** regionStructure;


// Set up the main structue according to dag file
struct Region** initStru(char* path) {
  int fd = open(path, O_RDONLY);
  if(fd < 0)
  {
    perror("Invalid diagram file given");
    exit(0);
  }
  struct Region** structure = (struct Region**)
    malloc(sizeof(struct Region) * MAX_NUMBER);

  // Read file content, and split content by \n
  char* buffer = malloc(sizeof(char) * MAX_IO_BUFFER_SIZE);

//  Read content in the file
  if(read(fd, buffer, MAX_IO_BUFFER_SIZE) < 0)
  {
    printf("Unable to read the file");
    exit(0);
  }
  char** diaContent;
//  Read each line in the file
  int lines = makeargv(buffer, "\n", &diaContent);
  if( lines == 0) return NULL;


  for(int i = 0; i < lines; i++) {
    char** lineContent;
    int words = makeargv(diaContent[i], ":", &lineContent);
  // Handle when the dag only has one region
    if( lines == 1 && words == 1)
    {
      struct Region* region = (struct Region*)malloc(sizeof(struct Region));
      strcpy(region->name, lineContent[0]);
      return structure;
    }
    if(words < 2)
    {
      continue;
    }
  // Set up name of current region
    struct Region* region = (struct Region*)malloc(sizeof(struct Region));
    region->votes = (struct candidate**) malloc(sizeof(struct candidate*) * MAX_NUMBER);
    region->childNames = malloc(sizeof(char*) * MAX_NUMBER);
    strcpy(region->name, lineContent[0]);
//  Set up child name for current region and the parent name for the  child region
    for(int j = 1; j < words; j++)
    {
      struct Region* childRegion = (struct Region*)malloc(sizeof(struct Region));
      childRegion->votes = (struct candidate**) malloc(sizeof(struct candidate*) * MAX_NUMBER);
      childRegion->childNames = malloc(sizeof(char*) * MAX_NUMBER);
      char* childName = malloc(sizeof(char) * MAX_REG_NAME);
      childName = lineContent[j];
      region->childNames[j - 1] = childName;
      strcpy(childRegion->name, lineContent[j]);
      strcpy(childRegion->parentName, lineContent[0]);
// Append child region in the region array
      appendRegion(structure, childRegion);
    }
// append region in the the main array
  appendRegion(structure, region);
  }
  return structure;
}





// Call back function for mutli thread.
void* operation(void* args) {
  int bytes;
  char buffer[MAX_IO_BUFFER_SIZE];
  struct threadArg* argu = (struct threadArg*)args;
  while ((bytes = read(argu->clientfd, &buffer, sizeof(buffer))) > 0)
  {
// Read all information from the argument variable
  char** commandMessage;
  char* add = argu->add;
  char* port = argu->port;
// Read information from socket
  int sep = makeargv(buffer, ";", &commandMessage);
  if(commandMessage[2] != NULL)
  {
    printf("Request received from client at %s:%s, %s %s %s\n", add, port, commandMessage[0],
    commandMessage[1], commandMessage[2]);
  }
  else{
    printf("Request received from client at %s:%s, %s %s\n", add, port, commandMessage[0],
    commandMessage[1]);
  }
  char* code = trimwhitespace(commandMessage[0]);
  char* response = malloc(sizeof(char) * (MAX_IO_BUFFER_SIZE));
  struct Region* region;
  // Set up region for all request except for RW (RW has no region name given)
  if(strcmp(code, "RW") != 0)
  {
    region = findRegion(regionStructure, trimwhitespace(commandMessage[1]));
  }
  response[0] = '\0';
// Handle RW request
      if(strcmp(code, "RW") == 0)
      {
// Find the root node that determines winner
      struct Region* root = findRoot(regionStructure);
// Makesure the pool is closed
      if(root->status == -1)
      {
        int max = -1;
        char* winner;
//  Find the winner that has the max vote number
        for(int i =0; i < MAX_NUMBER; i++)
        {
          if(root == NULL || root->votes[i] == NULL)
          {
            break;
          }
          if(root->votes[i]->vote > max)
          {
            winner = root->votes[i]->name;
            max = root->votes[i]->vote;
          }
        }
        if(winner != NULL )   //  Winner is dertermined
        {
          strcat(response, "SC;Winner:");
          strcat(response, winner);
          strcat(response,"\0");
        }
        else              // Winner has no votes
        {
          strcat(response, "SC;");
          strcat(response,"No votes;");
        }
      }
      else          //  The status of current node is not allowed get winner
      {
        // The region is not be initialized
        if(root->status == 0)
        {
          strcat(response,"RC;");
          strcat(response, root->name);
          strcat(response,"\0");
        }
        else  // The Region is still open
        {

          strcat(response,"RO;");
          strcat(response, root->name);
          strcat(response,"\0");
        }
      }
    }
    else if(strcmp(code, "CV") == 0)    // Handle VC request
    {
      if(region->name == NULL)        //  If the region is not fount in structure
      {
        strcat(response,"NR;");
        strcat(response, trimwhitespace(commandMessage[1]));
        strcat(response,"\0");
      }
      else                            //  The region satisfy pre-condition
      {
    //    Iterate all vote in structure and add them to buffer.
        char res[MAX_IO_BUFFER_SIZE];
        res[0] = '\0';
    //  Add all vote from the region to response
        for(int i = 0; i < MAX_NUMBER; i++)
        {
          if(region->votes[i] == NULL) break;
          char* voteS = malloc(sizeof(char) * 15);
          sprintf(voteS, "%d", region->votes[i]->vote);
          strcat(res, region->votes[i]->name);
          strcat(res, ":");
          strcat(res, voteS);
          if(region->votes[i + 1] != NULL)
          {
            strcat(res, ",");
          }
        }
    //  If noting in the res after the iteration, there is no votes recorded in this region
        if(strcmp(res, "") == 0)    //  There is no vote in that region
        {
          strcat(response, "SC;");
          strcat(response, "no votes recorded.");
          strcat(response,"\0");
        }
        else          //    The request is handled successfully
        {
        strcat(response, "SC;");
        strcat(response, res);
        strcat(response,"\0");
        }
      }
    }
    else if(strcmp(code, "OP") == 0)    // Handle OP request
    {
    // check the open status of current region
      if(region->name == NULL)
      {
        strcat(response,"NR;");
        strcat(response, trimwhitespace(commandMessage[1]));
        strcat(response,";");
      }
      else if(region->status == 1)    // The region is already opened
      {
        strcat(response, "PF;");
        strcat(response, region->name);
        strcat(response, " open");
        strcat(response,"\0");
      }
      else if(region->status == -1)   // the region is already closed
      {
        strcat(response, "RR;");
        strcat(response, region->name);
        strcat(response,"\0");
      }
      else
      {
        //  recOpen will open all region that is recorded in the childNames of current region
        recOpen(region, regionStructure);
        strcat(response, "SC;");
        strcat(response,"\0");
      }
    }
    else if(strcmp(code, "AV") == 0)    //   handle AV request
    {

    // Check that the region is opened to add
      if(region->name == NULL)      //  Invalid region that region not exited
      {
          strcat(response, "NR;");
          strcat(response, trimwhitespace(commandMessage[1]));
          strcat(response, "\0");
      }
      else if(region->childNames[0] != NULL)    // The region is not the leaf node
      {
        strcat(response, "NL;");
        char* data =malloc(sizeof(char) * strlen(region->name));
        strcat(response, region->name);
        strcat(response,"\0");
      }
      else if(region->status != 1)      //  The region is not open
      {
        strcat(response, "RC;");
        strcat(response, region->name);
        strcat(response,"\0");
      }
      else
      {
    //  Check that the vote is adding to leaf region
    // Add vote to the region
          int i = addVote(region->votes, trimwhitespace(commandMessage[2]));
          struct Region* parent = findRegion(regionStructure, region->parentName);

    // Add vote to all parent node of current node
          while(parent != NULL)
          {
            if( i < 0)
            {
              perror("Add vote error");
              exit(0);
            }
            i = addVote(parent->votes, trimwhitespace(commandMessage[2]));
            if(parent)
            parent = findRegion(regionStructure, parent->parentName);
          }
          strcat(response, "SC;");
          strcat(response,"\0");

      }     //  End of checing open status
    }
    else if(strcmp(code, "RV") == 0)    //  Handle RV request
    {
      if(region->name == NULL)
      {
          strcat(response, "NR;");
          strcat(response, trimwhitespace(commandMessage[1]));
          strcat(response, "\0");
      }
      else if(region->childNames[0] != NULL)        // The reagion is not Leaf Region
        {
          strcat(response, "NL;");
          strcat(response, region->name);
          strcat(response,"\0");
        }
      else if(region->status != 1)          // The current region is not opened
      {
        strcat(response, "RC;");
        strcat(response, region->name);
        strcat(response,"\0");
      }
      else
      {
          // delete vote to the region
          char** flag2;
          flag2 = deleteVote(region->votes, trimwhitespace(commandMessage[2]));
          // If nothing returned from the deleteVote function, there is no error
          // for the request and delete process
          if( flag2[0] == NULL)
          {
            struct Region* parent = findRegion(regionStructure, region->parentName);
            // delete vote to all parent node of current node
            while(parent != NULL)
            {
              if(flag2 >= 0)
              {
                break;
              }
              deleteVote(parent->votes, trimwhitespace(commandMessage[2]));
              parent = findRegion(regionStructure, parent->parentName);
            }
            strcat(response, "SC;");
            strcat(response,"\0");
          }
          else              //   Error appears in process
          {
            strcat(response, "IS;");
            // Add all candidate name from error array to the response
            for(int i = 0; i < MAX_NUMBER; i++)
            {
              if(flag2[i] == NULL)
              {
                break;
              }
              strcat(response, flag2[i]);
              if(flag2[i+1] != NULL)
              {
                strcat(response, ",");
              }
            }
            strcat(response, "\0");
          }
        }   //  End of checking leaf region
    }
    else if(strcmp(code, "CP") == 0)    // Handle CP request
    {
    // Check open status
      if(region->name == NULL)    //  The region is not existed
      {
        strcat(response, "NR;");
        strcat(response, trimwhitespace(commandMessage[1]));
        strcat(response, "\0");
      }
      else if(region->status != 1)      //  The region is not opened for close
      {
        strcat(response, "PF;");
        strcat(response, region->name);
        strcat(response, " close\0");
      }
      else
      {
    //  Close  all child poll
        recClose(region, regionStructure);
        strcat(response, "SC;");
        strcat(response,"\0");
      }
    }
    else if(strcmp(code, "AR") == 0)    // Extra credits for Add Region
    {
      // Make sure the r
      if(region->name == NULL)    //  The parent region is not existed
      {
        strcat(response, "NR;");
        strcat(response, trimwhitespace(commandMessage[1]));
        strcat(response, "\0");
      }
      else
      {
        // Create a new Region and give the newRegion name as well as parent Name
        struct Region* newRegion = (struct Region*)malloc(sizeof(struct Region));
        strcpy(newRegion->name, trimwhitespace(commandMessage[2]));
        strcpy(newRegion->parentName, trimwhitespace(commandMessage[1]));
        newRegion->status = region->status;
        newRegion->votes =(struct candidate**)malloc(sizeof(struct  candidate*) * MAX_NUMBER);
        newRegion->childNames = malloc(sizeof(char*) * MAX_NUMBER);
        // Create relationship between parent region and new region
        printf("???????\n");
        for(int i = 0; i < MAX_NUMBER; i++)
        {
          printf("!!!!!!\n");
          // Find the last position to store child name
          if(region->childNames[i] == NULL)
          {
            char* name = malloc(sizeof(char) * MAX_REG_NAME);
            name = trimwhitespace(commandMessage[2]);
            region->childNames[i] = name;
            break;
          }
        }
        printf("--------\n");

        // Add the new Region to regionStructure
        for( int j = 0; j < MAX_NUMBER; j++)
        {
          if(regionStructure[j] == NULL)
          {
            regionStructure[j] = newRegion;
            break;
          }
        }

        // Write response
        strcat(response, "SC;");
        strcat(response, trimwhitespace(commandMessage[1]));
        strcat(response, "\0");
      }
    }
    else      //  Unexpected request
    {
      strcat(response, "UC;");
      strcat(response,"\0");
    }

    //  Write response into socket
    printf("\n");
      printf("\n");
  int byte = write(argu->clientfd, response, 30);
  char** content;
  makeargv(response, ";", &content);
  // Print out response
  printf("Sending response to client at %s:%s, %s %s\n", add,
    port, content[0], content[1]);
  }
  		close(argu->clientfd);
}





int main(int argc, char** argv) {

  if(argc != NUM_ARGS + 1)
  {
    printf("Unexpected number of argument, expeted: %d, given %d",
      NUM_ARGS, argc - 1);
    exit(0);
  }
  const int SERVER_PORT = atoi(argv[2]);
  const char* path = argv[1];
  //  Initialize data structure
  regionStructure = initStru(argv[1]);


	// Create a TCP socket.
	int sock = socket(AF_INET , SOCK_STREAM , 0);

	// Bind it to a local address.
	struct sockaddr_in servAddress;
	servAddress.sin_family = AF_INET;
	servAddress.sin_port = htons(SERVER_PORT);
	servAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sock, (struct sockaddr *) &servAddress, sizeof(servAddress));

	// We must now listen on this port.
	listen(sock, MAX_CONNECTIONS);
  printf("Server listening on port %d\n", SERVER_PORT);

  char* add = inet_ntoa(servAddress.sin_addr);


  char* port = malloc(sizeof(char) * 10);
   sprintf(port, "%d", servAddress.sin_port);



  pthread_t thread[MAX_CONNECTIONS];
  int count = 0;



	// A server typically runs infinitely, with some boolean flag to terminate.
	while (1) {
		// Now accept the incoming connections.
		struct sockaddr_in clientAddress;
		socklen_t size = sizeof(struct sockaddr_in);
		int clientfd = accept(sock, (struct sockaddr*) &clientAddress, &size);
    printf("Connection initated from client at %s:%s\n", add, port);
    struct threadArg* argu = (struct threadArg* )malloc(sizeof(struct threadArg));
    argu->mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(argu->mutex, NULL);
    argu->clientfd = clientfd;
    argu->add = add;
    argu->port = port;
		// Buffer for data.
		// Read from the socket and print the contents/

    if(pthread_create(&thread[count], NULL, operation, (void*)argu) < 0)
    {
      perror("Unable to create thread\n");
      exit(0);
    }
    count = (count + 1) % MAX_CONNECTIONS;
    //  Keep reading until all thread finish
	}

	// Close the socket.
	close(sock);
}
