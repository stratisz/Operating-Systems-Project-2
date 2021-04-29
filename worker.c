#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/errno.h>
#include <sys/times.h>
# include <signal.h>
#include <poll.h>

extern int errno;
#define PERMS   0666

#define YES 1
#define NO 0

struct table{
    int writefd;
    int readfd;
    pid_t childpid;
};

int prime1 (int n){
    int i;
    if (n ==1) return (NO);
    for (i=2 ; i<n ; i++)
        if ( n % i == 0) return (NO);
    return ( YES );
}

int prime2 (int n){
    int i=0 , limitup =0;
    limitup = ( int )( sqrt (( float )n));
    if (n ==1) return (NO);
    for (i=2 ; i <= limitup ; i ++)
        if ( n % i == 0) return (NO);
    return ( YES );
}

int prime3(int n){
    int i=0 , limitup =0;
    if (n % 2 == 0) return (n == 2);
    limitup = ( int )( sqrt (( float )n));
    if (n ==1) return (NO);
    for (i=3 ; i <= limitup ; i+=2)
        if ( n % i == 0) return (NO);
    return ( YES );
}

int main(int argc, char *argv[]){
    pid_t rootpid;
    int readfd, writefd;
    char fifo3[20];
    sprintf(fifo3,"./%ldto%ld",(long)getppid(),(long)getpid());
    char fifo4[20];
    sprintf(fifo4,"./%ldto%ld",(long)getpid(),(long)getppid());
    while ( (writefd = open(fifo4, O_WRONLY)) < 0){
    }
    if ( (readfd = open(fifo3, O_RDONLY)) < 0){
	    perror("client: can't open read fifo \n");
    }

    char buffer[256];
    memset(buffer,'\0',sizeof(buffer));
    int bufferSize = 256;
    read(readfd, buffer, bufferSize);
    char* token;
    char delim[] = "- ";
    token = strtok(buffer, delim);
    int lb,ub,length;    
    lb = atoi(token);
    token = strtok(NULL,delim);
    ub = atoi(token);
    token = strtok(NULL,delim);
    int method = atoi(token);
    token = strtok(NULL,delim);
    rootpid = atoi(token);
    length = (ub-lb)/3;

    double t12 , t22 ;
    struct tms tb12 , tb22 ;
    double ticspersec2 ;
    int sum2 = 0;
    ticspersec2 = ( double ) sysconf ( _SC_CLK_TCK );
    t12 = ( double ) times (& tb12) ;

    int  i=0;
    if ( ( lb <1 ) || ( lb > ub ) ) {
        printf (" usage : prime1 lb ub\n") ;
        exit (1) ; }
    if(method == 1){
        struct pollfd fds[1];
        int timeout_msecs = 10000;
        int ret;
        int i = 0;
        fds[0].fd = writefd;
        fds[0].events = POLLOUT;
        for (i=lb ; i <= ub ; i ++){
            double t1 , t2 , cpu_time ;
            struct tms tb1 , tb2 ;
            double ticspersec ;
            int sum = 0;
            ticspersec = ( double ) sysconf ( _SC_CLK_TCK );
            t1 = ( double ) times (& tb1) ;
            if ( prime1 (i) == YES ){
                t2 = ( double ) times (& tb2) ;
                cpu_time = (double) ((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime + tb1.tms_stime));
                memset(buffer,'\0',sizeof(buffer));
                sprintf (buffer,"%d %lf\n",i,(t2-t1)/ticspersec);
                ret = poll(fds, 1, timeout_msecs);

                if (ret > 0) {
                    if (fds[0].revents & POLLOUT) {
                        write(writefd, buffer, strlen(buffer));
                    }
                }
            }
        }
    }
    else if(method == 2){
        struct pollfd fds[1];
        int timeout_msecs = 10000;
        int ret;
        int i = 0;
        fds[0].fd = writefd;
        fds[0].events = POLLOUT;
        for (i=lb ; i <= ub ; i ++){
            double t1 , t2 , cpu_time ;
            struct tms tb1 , tb2 ;
            double ticspersec ;
            int sum = 0;
            ticspersec = ( double ) sysconf ( _SC_CLK_TCK );
            t1 = ( double ) times (& tb1) ;
            if ( prime2 (i) == YES ){
                t2 = ( double ) times (& tb2) ;
                cpu_time = (double) ((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime + tb1.tms_stime));
                memset(buffer,'\0',sizeof(buffer));
                sprintf (buffer,"%d %lf\n",i,(t2-t1)/ticspersec);
                ret = poll(fds, 1, timeout_msecs);

                if (ret > 0) {
                    if (fds[0].revents & POLLOUT) {
                        write(writefd, buffer, strlen(buffer));
                    }
                }
            }
        }
    }
    else{
        struct pollfd fds[1];
        int timeout_msecs = 10000;
        int ret;
        int i = 0;
        fds[0].fd = writefd;
        fds[0].events = POLLOUT;
        for (i=lb ; i <= ub ; i ++){
            double t1 , t2 , cpu_time ;
            struct tms tb1 , tb2 ;
            double ticspersec ;
            int sum = 0;
            ticspersec = ( double ) sysconf ( _SC_CLK_TCK );
            t1 = ( double ) times (& tb1) ;

            if ( prime3 (i) == YES ){
                t2 = ( double ) times (& tb2) ;
                cpu_time = (double) ((tb2.tms_utime+tb2.tms_stime)-(tb1.tms_utime + tb1.tms_stime));
                memset(buffer,'\0',sizeof(buffer));
                sprintf (buffer,"%d %lf\n",i,(t2-t1)/ticspersec);
                ret = poll(fds, 1, timeout_msecs);

                if (ret > 0) {
                    if (fds[0].revents & POLLOUT) {
                        write(writefd, buffer, strlen(buffer));
                    }
                }
            }
        }
    }

    t22 = ( double ) times (& tb22) ;
    memset(buffer,'\0',sizeof(buffer));
    sprintf (buffer,"total %lf", (t22 - t12) / ticspersec2);
    write(writefd, buffer, strlen(buffer));
    struct pollfd fds[1];
    int timeout_msecs = 10000;
    int ret;
    fds[0].fd = writefd;
    fds[0].events = POLLHUP;
    ret = poll(fds, 1, timeout_msecs);

    if (ret > 0) {
        if (fds[0].revents & POLLHUP){}
    }
    kill(rootpid,SIGUSR1);
    exit(0);

}