#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

int *lcd =NULL;

//画矩形函数
void drw(int x,int y,int px,int py,int color)
{
	int i=0,j=0;
	for(j=0;j<y;j++)
	{
		for(i=0;i<x;i++)
		{
			if(j+py < 480 && i+px < 800)
			{
			*(lcd +(j+py)*800+(i+px)) = color;
			}
		
		}
	}
}
	
int main()
{
	 //对LCD设备进行映射
	int  fd = open("/dev/fb0",O_RDWR);	
	if(fd < 0)
	{
		printf("open lcd fail\n");
	}
	
	//对LCD设备进行映射操作
    lcd = mmap(NULL,800*480*4,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);

	
	
	//创建一个进程播放视屏 
	int pid = fork();
		if(pid == 0)
		{
			int pd = open("/pipe2",O_RDWR);
				if(pd < 0)
				{
					perror("open fail\n");
					exit(0);
				}
			printf("正在播放\n");
			//文件重定向
			dup2(pd,1); 
			
			//播放视频
			
			system("mplayer -slave -quiet -input  file=/pipe -zoom -x 800 -y 410  mv.avi");
			
		}
	
		if(pid > 0)
		{
			char buf[1024]={0};
			int num=0;
			
			drw(800,30,0,450,0x00ff00);
			int pd = open("/pipe2",O_RDWR);
			if(pd < 0)
			{
				perror("open fail\n");
				exit(0);
			}


			
			//不断写入获取百分比
			while(1)
			{
				
				//写入信息到管道中
				system("echo get_percent_pos > /pipe");
				sleep(1);

				//读取管道中的数据到buf中 
				
				read(pd,buf,1024);		
				//根据百分比绘制进度条
				
				/*
				strtok(buf,"=");
				char a[100]={0};
				strcpy(a,strtok(NULL,"="));
				int date = atoi(a); //把切割后的字符串转换成整形
				*/				
				sscanf(buf,"ANS_PERCENT_POSITION=%d",&num);
				drw(num*8,30,0,450,0xff0000); //绘制进度条
				if(num*8==800)
				{
					sleep(2);
					printf("播放结束\n");
					break;
					
				}					
			}
		}
	
	
	
	
	
}