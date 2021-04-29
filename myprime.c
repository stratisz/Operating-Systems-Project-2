#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <poll.h>
# include <signal.h>

extern int errno;
#define PERMS   0666

int flag = 0;

struct table{
    int writefd;
    int readfd;
    pid_t childpid;
};

void f( int signum ){ /* no explicit call to function f in main()*/
	signal (SIGUSR1 , f); 
    flag++;
}

void main(int argc, char *argv[]){
    signal (SIGUSR1 , f);
    int lb, ub, NumOfChildren;
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
    int length = (ub-lb)/NumOfChildren;
    pid_t parentpid = (long)getpid();
	pid_t childpid;
    struct table* Table = malloc(NumOfChildren*sizeof(struct table));
    for(int i=0;i<NumOfChildren;i++){ 
        if((childpid = fork()) <= 0){
            char buff[10];
            sprintf(buff,"%d",NumOfChildren);
            execlp("./inter","./inter", "-w", buff, (char *) NULL );
        }
        int readfd, writefd;
		char fifo1[20];
		sprintf(fifo1,"./%ldto%d",(long)getpid(),childpid);
		char fifo2[20];
		sprintf(fifo2,"./%dto%ld",childpid,(long)getpid());
		if((mkfifo(fifo1, PERMS) < 0) && (errno != EEXIST) ){
			printf("%s\n",fifo1);
			perror("can't create fifo");
		}
		if ((mkfifo(fifo2, PERMS) < 0) && (errno != EEXIST)) {
			unlink(fifo1);
			perror("can't create fifo");
		}
		if ( (readfd = open(fifo2, O_RDONLY)) < 0) {
			perror("server: can't open read fifo");
		}
		if ( (writefd = open(fifo1, O_WRONLY)) < 0) {
			perror("server: can't open write fifo");
		}
		Table[i].readfd = readfd;
		Table[i].writefd = writefd;
		Table[i].childpid = childpid;
        char buffer[256];
        if( i == 0 )
            sprintf(buffer,"%d-%d",lb,lb+length);
        else if( i == NumOfChildren-1 )
            sprintf(buffer,"%d-%d",lb+(i*length+1),ub);
        else
            sprintf(buffer,"%d-%d",lb+(i*length)+1,lb+(i*length)+length);
        write(writefd, buffer, strlen(buffer));
    }
    float*totals = (float*) malloc((NumOfChildren*NumOfChildren) * sizeof(float));
    int bufferSize = 1024;
    char buffer[bufferSize];
    char* token;
    char delim[] = "- \n";
    printf("Primes in [%d,%d] are: ",lb,ub); 
    for(int i = 0 ; i < NumOfChildren ; i++){
        memset(buffer,'\0',sizeof(buffer));
        read(Table[i].readfd, buffer, bufferSize);
        token = strtok(buffer,delim);
        while(strcmp(token,"t")!=0){
            printf("%s ",token);
            token = strtok(NULL,delim);
        }
        if(strcmp(token,"t") == 0){
            for(int t = (i*NumOfChildren) ; t < ((i*NumOfChildren)+NumOfChildren) ; t++){
                token = strtok(NULL,delim);
                totals[t] = atof(token);
            }
        }
    }
    float min = totals[0];
    float max = totals[0];
    for(int t = 0 ; t < NumOfChildren*NumOfChildren ; t++){
        if(min < totals[t]) min = totals[t];
        if(max > totals[t]) max = totals[t]; 
    }
    printf("\nMin Time for Workers : %lf msecs\n",min);
    printf("Min Time for Workers : %lf msecs\n",max);
    printf("Num of USR1 Received : %d/%d\n",flag,NumOfChildren*NumOfChildren);
    for(int t = 0 ; t < NumOfChildren*NumOfChildren ; t++){
        printf("Time for W%d: %lf msecs\n",t+1,totals[t]);
    }
    free(Table);
    free(totals);
}