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
#include <pthread.h>
#include <linux/input.h>

#define WHITE 0xffffff      //白色
#define BLACK 0x000000		//黑色

int *lcd =NULL;

int q=0;

/*
	满屏刷颜色
*/
void clean_lcd(int color)
{
	int i = 0;
	for(;i<800*480;i++)
		*(lcd+i) = color;
}

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

//控制影片函数
int  video_ctrl()
{
		//打开触摸屏设备 
	int vfd = open("/dev/input/event0",O_RDWR);
	if(vfd < 0)
	{
		perror("");
		return -1;
	}
	int x=0,xx=0,y=0,yy=0;
	//读取触摸屏的事件值 
	struct input_event ts; //定义输入系统模型
	
	 while(1)
	 {
		read(vfd,&ts,sizeof(ts));
		//处理信息 
		if(ts.type == EV_ABS && ts.code == ABS_X)
		{
			//输出X轴的坐标值
			x = ts.value;
		}
		if(ts.type == EV_ABS && ts.code == ABS_Y)
		{
			//输出X轴的坐标值 
			y = ts.value;
		}
			  //手指按下
		if(ts.type == EV_KEY  && ts.code == BTN_TOUCH && ts.value == 1)
		{
			
			xx = x;
			yy = y;
			printf("xx=%d,yy=%d\n",xx,yy);
		}
		
			//手指松开
		if(ts.type == EV_KEY && ts.code == BTN_TOUCH && ts.value == 0)
		{
			//输出X轴的坐标值 
			printf("x=%d,y=%d\n",x,y);
			printf("x-xx=%d,y-yy=%d\n",xx-x,yy-y);
			
			//消除抖动
			//向后遍历链表
			if(xx-x < -100)
			{
				system("echo seek +5 > /pipe");
				//清零坐标
				x  = 0; 
				y  = 0;
				xx = 0; 
				yy = 0;  
				
				continue;	
			}

			//向前遍历链表			
			if(xx - x > 100)
			{
				system("echo seek -5 > /pipe");
				drw(800,30,0,450,0x00ff00);
				//清零坐标
				x  = 0; 
				y  = 0;
				xx = 0; 
				yy = 0;  
				
				continue;		
			}
		
			//上滑动  
			if(yy - y > 100)
			{

				system("echo volume +80 > /pipe");
				//清零坐标
				x  = 0; 
				y  = 0;
				xx = 0; 
				yy = 0;  
				
				continue;
			
			}
				//下滑动
			if(yy - y < -100)
			{
				system("echo volume -80 > /pipe");
				//清零坐标
				x  = 0; 
				y  = 0;
				xx = 0; 
				yy = 0;  
				
				continue;			
			}
			
			if(xx-x > -100&&xx - x < 100&&yy - y < 100&&yy - y > -100)
			{
				system("echo pause > /pipe");
				q++;
				//清零坐标
				x  = 0; 
				y  = 0;
				xx = 0; 
				yy = 0;  
				
				continue;					
			}
			
			
			/*
			
			//判断用户点击的坐标位置		ctrl 最开始的控制流程
			//左边的功能
			if(x > 0 && x < 400 && y > 120 && y <480 && ctrl ==0)
			{

			}
			
			//右边的功能
			if(x > 400 && x < 800 && y > 120 && y <480)
			{

			}
			
			
			//清空数据
			x  = 0; 
			y  = 0;
			xx = 0; 
			yy = 0;  
			printf("--------------------------------------------\n");
			
			*/
		}
	
	
	 }
	 
	 //关闭所有设备
	close(vfd);
}

//线程获取视频进度并画进度条
void *drwProgress(void *arg)
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
		while(q%2)
		{
			
		}
		
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
			pthread_exit(NULL);
			
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
			pthread_t tid=0;			
			pthread_create(&tid,NULL,drwProgress,NULL);		
			video_ctrl();
			return 0;
		}
	
	
}

