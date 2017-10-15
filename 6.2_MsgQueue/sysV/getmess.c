#include <sys/ipc.h>                                                            
#include <sys/msg.h>                                                             
#include <sys/types.h>                                                          
#include <stdio.h>                                                              
#include <unistd.h>                                                             
#include <fcntl.h>                                                              
#include <errno.h>                                                              
#include <string.h>                                                             
                                                                                
#define SYNCH_F "/tmp/msg.temp"                                                 
#define PROJ_ID 1                                                               
#define BUF_SIZE 80                                                             
#define RESULT "/home/box/message.txt"                                          
                                                                                
struct message {
                                                                    
        long mtype;                                                                 
            char mtext[BUF_SIZE];                                                       
};  

int main ()
{
    struct message buf;                                                     
    memset(buf.mtext, 0 , BUF_SIZE * sizeof(char));                         
    long msgtype = 0;                                                       
    int msgflags = 0;                                                       

    FILE* rf = fopen(RESULT, "w");                                          

    key_t key = ftok("/tmp/msg.temp", 1);                                   
    printf("key %d\n", key);                                                
    int msgid = msgget(key, 0666 |IPC_CREAT);                               

    printf("msgid %d\n", msgid);                                            
    printf("%s\n", strerror(errno));                                        

    int res = msgrcv(msgid, (void*)&buf, BUF_SIZE, msgtype, msgflags);      

    printf("%s\n", strerror(errno));                                        

    if(res) {
        fprintf(rf, "%s", buf.mtext);                           
    }                                                               

    printf("buffer %s\n" , buf.mtext);                                      

    fclose(rf);                                                             

    msgctl(msgid, IPC_RMID, NULL);                                          

    return 0; 
}
