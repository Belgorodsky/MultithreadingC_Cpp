#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>                                                              
#include <unistd.h>                                                             
#include <errno.h>                                                              
#include <string.h>                                                             
                                                                                
#define BUF_SIZE 8192
#define RESULT "/home/box/message.txt"                                          
#define P_NAME "/test.mq"                                                                                

int main ()
{
    char buf[BUF_SIZE];
    memset(buf,0 , BUF_SIZE * sizeof(char));
    long msgtype = 0;
    int msgflags = 0; 

    FILE* rf = fopen(RESULT, "w");

    int msgid = mq_open(P_NAME, O_CREAT, 0666, 0);

    printf("msgid %d\n", msgid);
    printf("%s\n", strerror(errno));

    int res = mq_receive(msgid, buf, BUF_SIZE, 0);

    printf("%s\n", strerror(errno));

    if(res) {
        fprintf(rf, "%s", buf); 
    }

    printf("buffer %s\n" , buf); 

    fclose(rf); 

    return 0; 
}
