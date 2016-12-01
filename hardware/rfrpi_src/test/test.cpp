/* ===================================================
C code : test.cpp
* ===================================================
*/

#include <stdlib.h>
#include <string>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

 std::string DeviceR= "/dev/gpiofreq" ;
int main(int argc, char *argv[])
{
    FILE * fp;
    std::string Device ;
    if (argc>=2)
    Device =  argv[1] ;
    else
    Device = DeviceR + "17" ;
    printf("opening %s  \n",Device.c_str());
    
    fp = fopen(Device.c_str(), "r");
    if ( fp == NULL ) {
       printf("[ERROR] %s device not found - kernel driver must be started !!\n",Device.c_str());
       exit(1);
    }

    char buffer[2048];
    char * bstart = buffer;
    int reste = 0;
    while (1) {
       int count = fread(bstart,1,1,fp);
       count += reste;

       if ( count > 0 ) {
         /*
         printf("V : [");
         for ( int k = 0 ; k < count ; k ++  ) printf("%d ",buffer[k]);
         printf("\n");
         */
        buffer[count]=0;
        printf("count %d %s\n",count,buffer);

       }
       usleep(1000000l);
    }
    return 0;

}
