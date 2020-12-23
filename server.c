

#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>


#define MAXBUF 1024
#define MAXPORT 6
#define MAXADDR 16
#define MAXREAD 1024
#define MAXWRITE 1024
#define CONNECTIONS 6


void logginServerReply(char sck_addr[MAXADDR], char reply[], FILE *file);



int main (int argc,char* argv[]) {
    
    char connect_addr[CONNECTIONS][MAXADDR] = {};
    time_t connect_times[CONNECTIONS] = {};

    int agentnum = 0;
    char cPort[MAXPORT];
    int nPort;

    if (argc < 2){
        printf ("Usage: port number required \r\n");
        return(0);
    }

    memset(cPort, 0, MAXPORT);
    sprintf(cPort,"%s",argv[1]);
    nPort = atoi(cPort);

    // file for log
    //open file to send contents
    FILE *file, *read_file;
    file = fopen("log.txt", "w");
    
    // create the server socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define the address server structures
        
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(nPort);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Calling the bind function
    // bind the socket to our specified IP and port
    // now it is bound to IP port and it can listne to connection
    if(bind(server_socket, (struct  sockaddr*) &server_address, sizeof(server_address)) < 0){
        perror("There was an error in the binding stage\n");
    }
    else{
        fprintf(stdout, "Awesome, The Binding Was Successful!\n");
    }

    // creating integer to hold clinet socket to accept connections
    int agent_socket;
    struct sockaddr_in agent_addr;
    int agentlen;
    
    
    // listening stage
    fprintf(stdout, "Server is Listening for Agents...\n");
    listen(server_socket, 15);
    agentlen = sizeof(agent_addr);
    
    // creating an infinite loop to keep the communication between the server, "satellite", and
    // the client, "agent"
    
    while(1){
        
        // accepting a connection fromthe agent
        agent_socket = accept(server_socket, (struct sockaddr *) &agent_addr, &agentlen);
        
        // checking for errors before poceeding
        if(agent_socket < 0){
            perror("ERROR AT ACCEPTING STAGE\n");
        }
        
        // make our charracter arrays for agent socket address and action given
        
        // Socket_add will be the buffer to hold the socket address
        // SIZE MAXADDR = 16
        char socket_add [MAXADDR];
        
        // socket_action will be the buffer where the agent response is stored
        // MAXREAD = 1024
        char socket_action[MAXREAD];
        
        // Note since we are looping after every response we must make sure to cleanr the
        // agent client buffer to prevent any junk from carrying over to the next respsponse
        // the following line will fill the entire socket_action buffer with '0' then fill it
        // with agent message/action
        
        memset(socket_action, 0 ,sizeof(socket_action) );

        inet_ntop(AF_INET, &agent_addr.sin_addr, socket_add, MAXREAD);
        read(agent_socket, socket_action, MAXREAD);
        
        int id;
        bool found = false;
        for(id = 0 ; id< CONNECTIONS; ++id){
            if(!strcmp(connect_addr[id], socket_add)){
                agentnum = id;
                found = true;
            }
        }

        if(!found) {
            agentnum = -1;
        }
        
        //Log the Action
        
        //char buffer to hoold message
        char buffer[MAXBUF];
        //initialize current time
        time_t current_time = time(0);
        
        //set time as string and pint to file
        strftime(buffer, MAXBUF, "%Y-%n-%d %H:%M:%S.000", localtime(&current_time));
        fprintf(file, "\"%s\": Responded to agent \"%s\" with \"%s\"\r\n", buffer, socket_action, socket_add);
        
        
        // *********************** #JOIN ACTION *******************************
        if(!strcmp(socket_action, "#JOIN")) {
            if(agentnum < 0) {
                // boool statement to determine if capacity of satleillite has been met
                bool filled = true;
                int i;
                for(i = 0; i < CONNECTIONS; ++i) {
                    if(connect_addr[i][0] == 0) {
                        sprintf(connect_addr[i], "%s", socket_add);
                        connect_times[i] = time(NULL);
                        filled = false;
                        break;
                    }
                }
                // Responding with $OK after JOIN IS SUCCESSFULL
                write(agent_socket, "$OK", MAXWRITE);
                logginServerReply(socket_add, "$OK", file);
                
            }
            // if already a member
            else {
                write(agent_socket, "$ALREADY MEMBER", 15);
                logginServerReply(socket_add, "$ALREADY MEMBER", file);
            }
        }
        // ************************ #LEAVE ACTION ************************
        else if(!strcmp(socket_action, "#LEAVE")) {
            if(agentnum < 0) {
                write(agent_socket, "$NOT MEMBER", 11);
            }
            else {
                memset(connect_addr[agentnum], 0, MAXADDR);
                write(agent_socket, "$OK", 3);
            }
        }
        // ***************** #LIST ACITON ****************************
        else if(!strcmp(socket_action, "#LIST")) {
            if(agentnum >= 0) {
                //log it
                logginServerReply(socket_add, "List shown", file);
                
                char list[MAXWRITE] = {};
            
                for(int i = 0; i < CONNECTIONS; ++i) {
                    if(connect_addr[i][0] != 0) {
                        sprintf(list, "<%s, %ld> \n", connect_addr[i], time(NULL) - connect_times[i]);
                        write(agent_socket, list, MAXWRITE);
                    }
                }
            }
            else {
                //log it
                logginServerReply(socket_add, "\0", file);
            }
        }
        
        // ***************** LOG ACTION ***********************
        
        else if(!strcmp(socket_action, "#LOG")) {
            if(agentnum >= 0) {
                logginServerReply(socket_add, "LOG WRITTEN AS: log.txt", file);
                fflush(file);
                
                read_file = fopen("log.txt", "r");
                
                char log[MAXWRITE];
                while(fgets(log, MAXWRITE - 1, read_file))
                    write(agent_socket, log, strlen(log));
                
                fclose(read_file);
            }
            else{
                logginServerReply(socket_add, "\0", file);
            }
        }
        else {
            logginServerReply(socket_add, "\0", file);
        }
        fflush(file);
        close(agent_socket);
    }
    return 0;
}


void logginServerReply(char socket_address[MAXADDR], char reply[], FILE *file){
    //char buffer to hold message
    char buffer[MAXBUF];
    //initialize the time
    time_t current_time = time (0);
    
    //put time ito a string and send it to file
    strftime(buffer, MAXBUF, "%Y-%m-%d %H:%M:%S.000", localtime(&current_time));
    if(reply[0] == 0){
        fprintf(file, "\"%s\": No response: \"%s\" (AGENT IS NOT ACTIVE)\r\n", buffer, socket_address);
    }else{
        fprintf(file, "\"%s\": Responded to agent \"%s\" with \"%s\"\r\n", buffer, socket_address, reply);
    }
}
