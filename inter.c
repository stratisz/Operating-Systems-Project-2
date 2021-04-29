#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
# include <signal.h>

# define MSG_BUF 256
# include <poll.h>

extern int errno;
#define PERMS   0666

struct table{
    int writefd;
    int readfd;
    pid_t childpid;
};

struct Node { 
    int prime;
    float time;
    struct Node* next; 
};

void sortedInsert(struct Node** head_ref, struct Node* new_node){ 
    struct Node* current;
    if (*head_ref == NULL || (*head_ref)->prime >= new_node->prime){ 
        new_node->next = *head_ref; 
        *head_ref = new_node; 
    } 
    else {
        current = *head_ref; 
        while (current->next != NULL 
               && current->next->prime < new_node->prime) { 
            current = current->next; 
        } 
        new_node->next = current->next; 
        current->next = new_node; 
    } 
} 
  
struct Node* newNode(int new_prime,float new_time){ 
    struct Node* new_node  = (struct Node*)malloc(sizeof(struct Node)); 
    new_node->prime = new_prime;
    new_node->time = new_time; 
    new_node->next = NULL; 
  
    return new_node; 
}

void deleteList(struct Node* Head){
    struct Node* temp = Head;
    while(Head->next != NULL){
        Head=Head->next;
        free(temp);
        temp = Head;
    }
    free(Head);
}

void f( int signum ){
	signal (SIGUSR1 , f); 
}

int main(int argc, char *argv[]){
    signal (SIGUSR1 , f);
    pid_t childpid;
    int lb,ub,length;
    int NumOfChildren = 3;
    for(int i = 1 ; i < argc ; i++){
        if(strcmp(argv[i],"-l")==0){
            lb = atoi(argv[i+1]);
        }
        if(strcmp(argv[i],"-u")==0){
            ub = atoi(argv[i+1]);
        }
        if(strcmp(argv[i],"-w")==0){
            NumOfChildren = atoi(argv[i+1]);
        }
    }
    int readfd, writefd;
	char fifo1[20];
	sprintf(fifo1,"./%ldto%ld",(long)getppid(),(long)getpid());
	char fifo2[20];
	sprintf(fifo2,"./%ldto%ld",(long)getpid(),(long)getppid());
	while ( (writefd = open(fifo2, O_WRONLY)) < 0){
	}
	if ( (readfd = open(fifo1, O_RDONLY)) < 0){
		perror("client: can't open read fifo \n");
	}
    struct table* Table = malloc(NumOfChildren*sizeof(struct table));
    for(int j=0;j<NumOfChildren;j++){
        if((childpid = fork()) == 0){
            execlp("./worker","./worker", (char *) NULL );
        }
        int readfdc, writefdc;
		char fifo3[20];
		sprintf(fifo3,"./%ldto%d",(long)getpid(),childpid);
		char fifo4[20];
		sprintf(fifo4,"./%dto%ld",childpid,(long)getpid());
		if((mkfifo(fifo3, PERMS) < 0) && (errno != EEXIST) ){
		    printf("%s\n",fifo3);
		    perror("can't create fifo");
		}
		if ((mkfifo(fifo4, PERMS) < 0) && (errno != EEXIST)) {
		    unlink(fifo3);
			perror("can't create fifo");
		}
		if ( (readfdc = open(fifo4, O_RDONLY)) < 0) {
		    perror("server: can't open read fifo");
		}
	    if ( (writefdc = open(fifo3, O_WRONLY)) < 0) {
		    perror("server: can't open write fifo");
	    }
		Table[j].readfd = readfdc;
		Table[j].writefd = writefdc;
		Table[j].childpid = childpid;

    }
    char buffer[1024];
    memset(buffer,'\0',sizeof(buffer));
    int bufferSize = 1024;
    read(readfd, buffer, bufferSize);
    char* token;
    char delim[] = "-";
    token = strtok(buffer, delim);
    lb = atoi(token);
    token = strtok(NULL,delim);
    ub = atoi(token);
    length = (ub-lb)/NumOfChildren;
    pid_t rootpid = getppid();
    for(int j = 0 ; j < NumOfChildren ; j++){
        if( j == 0 )
            sprintf(buffer,"%d-%d 1 %d",lb,lb+length,rootpid);
        else if( j == NumOfChildren-1 ){
            if( (j-1) % 3 == 0)
                sprintf(buffer,"%d-%d 2 %d",lb+(j*length+1),ub,rootpid);
            else if( (j-2) % 3 == 0)
                sprintf(buffer,"%d-%d 3 %d",lb+(j*length+1),ub,rootpid);
            else
                sprintf(buffer,"%d-%d 1 %d",lb+(j*length+1),ub,rootpid);    
        }
        else{
            if( (j-1) % 3 == 0)
                sprintf(buffer,"%d-%d 2 %d",lb+(j*length)+1,lb+(j*length)+length,rootpid);
            else if( (j-2) % 3 == 0)
                sprintf(buffer,"%d-%d 3 %d",lb+(j*length)+1,lb+(j*length)+length,rootpid);
            else
                sprintf(buffer,"%d-%d 1 %d",lb+(j*length)+1,lb+(j*length)+length,rootpid);  
        }
        
        write(Table[j].writefd, buffer, strlen(buffer));
    }
    struct pollfd fds[NumOfChildren];
    int timeout_msecs = 500;
    int ret;
    int i = 0;
    int end = 0;
    struct Node* head = NULL; 
    struct Node* new_node = NULL;
    float*totals = (float*) malloc(NumOfChildren * sizeof(float));

    for(int j = 0 ; j < NumOfChildren ; j++){
        fds[j].fd = Table[j].readfd;
        fds[j].events = POLLIN || POLLHUP;
    }
    while(1){
        memset(buffer,'\0',sizeof(buffer));
        ret = poll(fds, NumOfChildren, timeout_msecs);

        if (ret > 0) {
            for (i=0; i<NumOfChildren; i++) {
                if (fds[i].revents & POLLIN) {
                    read(fds[i].fd, buffer, bufferSize);
                    char* token;
                    char delim[] = "- \n";
                    token = strtok(buffer, delim);
                    int prime;
                    float time;
                    if(strcmp(token,"total") == 0){
                        end++;
                        token = strtok(NULL,delim);
                        totals[i] = atof(token);
                        break;
                    }   
                    prime = atoi(token);
                    token = strtok(NULL,delim);
                    time = atof(token);
                    new_node = newNode(prime,time); 
                    sortedInsert(&head, new_node); 
                    while((token = strtok(NULL,delim)) != NULL){
                        if(strcmp(token,"total") == 0){
                            end++;
                            token = strtok(NULL,delim);
                            totals[i] = atof(token);
                            break;
                        }
                        prime = atoi(token);
                        token = strtok(NULL,delim);
                        time = atof(token);
                        new_node = newNode(prime,time); 
                        sortedInsert(&head, new_node); 
                    }
                    break;
                }
                if (fds[i].revents & POLLHUP) {
                    end++;
                    if (end == NumOfChildren) break;
                }

            }
            i = 0;
            if (end == NumOfChildren) break;
        }
    }
    memset(buffer,'\0',sizeof(buffer));
    char t[64];
    struct Node* temp = head; 
    while (temp != NULL) {
        sprintf(t,"%d %lf ", temp->prime,temp->time);
        strcat(buffer,t);
        temp = temp->next; 
    }
    strcat(buffer,"t ");
    for(int s = 0 ; s < NumOfChildren ; s++){
        sprintf(t,"%lf ",totals[s]);
        strcat(buffer,t);
    }
    write(writefd, buffer, strlen(buffer));
    for(int u = 0 ; u < NumOfChildren ; u++){
        close(Table[u].readfd);
        close(Table[u].writefd);
        char fifo3[64];
        memset(fifo3,'\0',sizeof(fifo3));
        sprintf(fifo3,"./%ldto%d",(long)getpid(),Table[u].childpid);
        if ( unlink(fifo3) < 0) {
	        perror("client: can't unlink \n");
        }
        memset(fifo3,'\0',sizeof(fifo3));
        sprintf(fifo3,"./%dto%ld",Table[u].childpid,(long)getpid());
        if ( unlink(fifo3) < 0) {
	        perror("client: can't unlink \n");
        }
    }
    deleteList(head);
    free(Table);
    free(totals);
    close(readfd);
	close(writefd);
	if ( unlink(fifo1) < 0) {
		perror("client: can't unlink \n");
	}
	if ( unlink(fifo2) < 0) {
	    perror("client: can't unlink \n");
	}
    exit(0); 
}


