#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include<fcntl.h>





#define RD_VALUE _IOR('a','b',int32_t*)


int main()
{

int fd;
int buffer;
int select;
int value;
int val;
int allign;
fd=open("/dev/adc8",O_RDWR);
printf("ADC\n");
printf("enter the channel for ADC:");
scanf("%d",&select);
printf("\n");
printf("1.right aligned\n");
printf("2.left aligned\n");
printf("enter the alighnment:\n");
scanf("%d",&allign);


switch(select)
{

case 1:
printf("channel 1 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");
break;


case 2:
printf("channel 2 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;



case 3:
printf("channel 3 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;



case 4:
printf("channel 4 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;



case 5:
printf("channel 5 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;




case 6:
printf("channel 6 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;



case 7:
printf("channel 7 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;



case 8:
printf("channel 8 is selected\n");
if(allign==1)
printf("choosed right allignment");
else if(allign==2)
printf("choosed left allignment");

break;

}

printf("\n");
ioctl(fd, RD_VALUE, (int32_t*) &value);
value=value/10;
printf("The ADC output is :%u\n",value);
return 0;

}
		

