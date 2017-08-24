#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>


char*buffer;
int length=4096;
void setBuf(char c){
  memset(buffer,c,length);
  buffer[length-1]='\0';
}
void setAlphaBuf(void){
  int i=0;
  for(;i<length;i++){
  	buffer[i]=i%26+65;
  }
  buffer[length-1]='\0';
}
int main(int argc, char*argv[]){

	buffer=malloc(length);
	int fd = open("/dev/asgn1",O_RDWR);
	int rc=0;// = read(fd,buffer,1);

	setBuf('1');
   
	printf("buffer value: %s\n",buffer);
	
	rc=write(fd,buffer,length);
	setBuf('2');
	rc=write(fd,buffer,length);
	setBuf('3');
	rc=write(fd,buffer,length);
	setBuf('4');
	rc=write(fd,buffer,length);

	printf("file:%d wirte:%d\n",fd,rc);

	setBuf('5');  
	
	printf("buffer now: %s\n",buffer);
	rc=write(fd,buffer,length);
        setBuf('6');
	rc=write(fd,buffer,length);

	printf("file:%d wirte:%d\n",fd,rc);
       
       	setBuf('*');
 	
	printf("now: %s\n",buffer);
	
	lseek(fd,0,SEEK_SET);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);
	
	//rc=read(fd,buffer,length);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);
	rc=read(fd,buffer,length);
	printf("buffer after read: %s\n",buffer);


	close(fd);

	return 0;
}
