/**
 * \file
 * \brief Empty user application template
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to board_init()
 * -# "Insert application code here" comment
 *-pipe -fno-strict-aliasing -Wall -Wstrict-prototypes -Wmissing-prototypes -Werror-implicit-function-declaration -Wpointer-arith -std=gnu99 -ffunction-sections -fdata-sections -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int -Wmain -Wparentheses -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused -Wuninitialized -Wunknown-pragmas -Wfloat-equal -Wundef -Wshadow -Wbad-function-cast -Wwrite-strings -Wsign-compare -Waggregate-return  -Wmissing-declarations -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations -Wpacked -Wredundant-decls -Wnested-externs -Wlong-long -Wunreachable-code -Wcast-align --param max-inline-insns-single=500
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
//#include <hsf.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdio.h>
//#include <httpc/httpc.h>
//#include <base64.h>
// #include <hfthread.h>
#include "../example.h"
#include <my_xmpp.h>

/**
	Name:	   pet.c
	Time:	   2016-4-23 22:55:23
	Function:  测试定时功能的字符串问题是否由于全局变量定义太多导致出错
	Result:	   原来程序中定义全局变量太多导致的错误，pet.c没有问题
	
*/

#if (EXAMPLE_USE_DEMO==USER_SOCKET_DEMO)

void recvTest(void *arg);
void pingTest(void *arg);
void udpRecvTest(void *arg);

void fdthread(void *arg);
void wtthread(void *arg);
void time_fdthread(void *arg);


const int hf_gpio_fid_to_pid_map_table[HFM_MAX_FUNC_CODE]=
{
	HF_M_PIN(2),	//HFGPIO_F_JTAG_TCK
	HF_M_PIN(3),	//HFGPIO_F_JTAG_TDO
	HF_M_PIN(4),	//HFGPIO_F_JTAG_TDI
	HF_M_PIN(5),	//HFGPIO_F_JTAG_TMS
	HFM_NOPIN,		//HFGPIO_F_USBDP
	HFM_NOPIN,		//HFGPIO_F_USBDM
	HF_M_PIN(39),	//HFGPIO_F_UART0_TX
	HF_M_PIN(40),	//HFGPIO_F_UART0_RTS
	HF_M_PIN(41),	//HFGPIO_F_UART0_RX
	HF_M_PIN(42),	//HFGPIO_F_UART0_CTS
	
	HF_M_PIN(27),	//HFGPIO_F_SPI_MISO
	HF_M_PIN(28),	//HFGPIO_F_SPI_CLK
	HF_M_PIN(29),	//HFGPIO_F_SPI_CS
	HF_M_PIN(30),	//HFGPIO_F_SPI_MOSI
	
	HFM_NOPIN,	//HFGPIO_F_UART1_TX,
	HFM_NOPIN,	//HFGPIO_F_UART1_RTS,
	HFM_NOPIN,	//HFGPIO_F_UART1_RX,
	HFM_NOPIN,	//HFGPIO_F_UART1_CTS,
	
	HF_M_PIN(43),	//HFGPIO_F_NLINK
	HF_M_PIN(44),	//HFGPIO_F_NREADY
	HF_M_PIN(45),	//HFGPIO_F_NRELOAD
	HF_M_PIN(7),	//HFGPIO_F_SLEEP_RQ
	HF_M_PIN(8),	//HFGPIO_F_SLEEP_ON

	HFM_NOPIN,		//HFGPIO_F_RESERVE0
	HFM_NOPIN,		//HFGPIO_F_RESERVE1
	HFM_NOPIN,		//HFGPIO_F_RESERVE2
	HFM_NOPIN,		//HFGPIO_F_RESERVE3
	HFM_NOPIN,		//HFGPIO_F_RESERVE4
	HFM_NOPIN,		//HFGPIO_F_RESERVE5
	
	HFM_NOPIN,	//HFGPIO_F_USER_DEFINE
};

const hfat_cmd_t user_define_at_cmds_table[]=
{
	{NULL,NULL,NULL,NULL} //the last item must be null
};

int udp_fd;
struct sockaddr_in address;

hfuart_handle_t huart;
int i, j, k, n;
char recvbuf[50];
char *p, *q;
char *cmd[2];
char *actcmd[3];

char control_water[4] = {0xFF,0x02,0x01,0x00};
char control_feed[4] = {0xFF,0x03,0x01,0x00};

///全局变量用于返回操作状态
int fdstate = 0;                           									//喂食状态,状态0表示可用
int wtstate = 0;

///定时喂食
char read_flashbuf[80], store_flashbuf[80];                    		//读取及存储用户flash的缓存数组
char tmp_flash_strcmp[11], tmp_cmd_strcmp[11];
char *flash_set_str[4];
char *p_str;
char *vertLine = "|";
int n_str, k_str, c_str;

///取时钟模块时间值
char query_time_cmd[4] = {0xFF,0x01,0x03,0x00};
int hour, min;
char time_str[6];
char recv_timebuf[2];

///定时线程函数中的变量
char read_timed_flashbuf[80], store_timed_flashbuf[80], timed_cmpbuf[11];
char *timed_flash_set_str[4], *everyTimeCmd[4];
char *p_timed_str, *p_cmd;
int n_timed, k_timed, k_cmd, c_cmd;

int BCDToInt(char bcd)               ///bcd码到十进制int, 例如：char 0x14转换为int 14
{
    return (0xff & (bcd>>4))*10 +(0xf & bcd);
}

char *reverse(char *s)  
{  
    char temp;  
    char *p = s;    //p指向s的头部  
    char *q = s;    //q指向s的尾部  
    while(*q)  
        ++q;  
    q--;  
  
    //交换移动指针，直到p和q交叉  
    while(q > p)  
    {  
        temp = *p;  
        *p++ = *q;  
        *q-- = temp;  
    }  
    return s;  
}

char *my_itoa(int n)  
{  
    int i = 0,isNegative = 0;  
    static char s[100];      //必须为static变量，或者是全局变量  
    if((isNegative = n) < 0) //如果是负数，先转为正数  
    {  
        n = -n;  
    }  
    do      //从各位开始变为字符，直到最高位，最后应该反转  
    {  
        s[i++] = n%10 + '0';  
        n = n/10;  
    }while(n > 0);  
  
    if(isNegative < 0)   //如果是负数，补上负号  
    {  
        s[i++] = '-';  
    }  
    s[i] = '\0';    //最后加上字符串结束符  
    return reverse(s);    
}

//主函数
int app_main(void) 
{
	time_t now = time(NULL);
	hfdbg_set_level(3);
	HF_Debug(DEBUG_LEVEL,"[FILE DEMO]sdk version(%s),the app_main start time is %d %s\n",hfsys_get_sdk_version(), now, ctime(&now));
	if (hfgpio_fmap_check(HFM_TYPE_LPB100) != 0) 
	{
		while (1) 
		{
			HF_Debug(DEBUG_ERROR, "gpio map file error\n");
			msleep(1000);
		}
		//return 0;
	}

	while (!hfnet_wifi_is_active()) 
	{
		msleep(50);
	}

	if (hfnet_start_assis(ASSIS_PORT) != HF_SUCCESS) 
	{
		HF_Debug(DEBUG_WARN, "start httpd fail\n");
	}

	if (hfnet_start_httpd(HFTHREAD_PRIORITIES_MID) != HF_SUCCESS) 
	{
		HF_Debug(DEBUG_WARN, "start httpd fail\n");
	}
#ifndef TEST_UART_SELECT		
	// if (hfnet_start_uart(HFTHREAD_PRIORITIES_LOW, NULL) != HF_SUCCESS) 
	// {
		// HF_Debug(DEBUG_WARN, "start uart fail!\n");
	// }
#endif	
	sleep(2);
	
	
	//创建一个udp方便调试程序
	if((udp_fd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
	{
		u_printf("create socket error!\n");
		exit(1);
	}
	
	bzero(&address,sizeof(address));
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=inet_addr("115.28.179.114");
	address.sin_port=htons(8123);
	
	//在主函数中打开串口
	if ((huart = hfuart_open(0))<0)
	{
		u_printf("open_port error!");
		exit(1);
	}
	
	/*内网模式：初始化相关数据，开启udp服务，开启接收线程*/
	 /*
	//初始化udp相关数据
	initUdpAddrData();
	//开启udp服务
	startUdpServer();
	
	//创建一个线程接收udp数据 
	if (hfthread_create(udpRecvTest, "RECV_THREAD", 256, NULL,HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS) {
		u_printf("create udpRecv thread fail\n");
		return 0;
	}
	
*/
	
	
	
	/*外网模式：初始化相关数据，并连接登陆服务器，开启相关线程*/
 
	//初始化数据
	initData();
  //连接服务器
	connectServer();

	//登陆服务器 
	loginServer();

	//创建一个线程接收数据 
	if (hfthread_create(recvTest, "RECV_THREAD", 256, NULL,HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS) 
	{
		u_printf("create recvMessage thread fail\n");
		return 0;
	}
	
  //创建一个线程监测主板的在线状态
	if (hfthread_create(pingTest, "RECON_THREAD", 256, NULL,HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS) 
	{
		u_printf("create pingServer thread fail\n");
		return 0;
	}
	
	

  
	return 1;

}


//外网模式接收测试
USER_FUNC void recvTest(void *arg) 
{
	if (hfthread_create(fdthread, "feed", 256, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS)         		 //创建喂食线程
	{
		u_printf("create feed thread fail\n");
	}
	
	if (hfthread_create(wtthread, "water", 256, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS)         		 //创建喂水线程
	{
		u_printf("create water thread fail\n");
	}
	
	if (hfthread_create(time_fdthread, "timed_fd", 256, NULL, HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS)        //创建定时线程
	{
		u_printf("create timed_fd thread fail\n");
	}
	
	while (getMessageThreadFlag())
	{
		sleep(1);								///因为设备在连接服务器过程中此线程一直在循环，实际使用时需要删除这个时延
		
		u_printf("Enter while!\n");
		int msgType = recvMessage();
		
		if (msgType == MESSAGE_FLAG)
		{
			//如果收到的是控制指令
			// u_printf("recv-> %s",getCurrentMsg());
			// sendMessage(getCurrentUser(),getCurrentMsg());
			/**
			 *第一步：调用getCurrentMsg()函数，获取收到的指令字符串，该函数返回字符数组（char*类型）
			 *第二步：根据需要解析字符串得到有效指令码，并发到底层进行控制
			 *第三步：控制结束后，反馈的信息通过sendMessage(user,msg)函数发出，该函数需要传入两个参数
			 *        user参数通过调用getCurrentUser()获得，msg参数即为底层要发给客户端的反馈信息（字符串类型）
			 *
			 *示例：char* codeStr = getCurrentMsg();
			 *      char* code = 指令解析函数（）;
			 *      进行底层控制,并得到反馈字符串;
			 *      sendMessage(getCurrentUser(),"反馈字符串");
			 *
			 */ 
			u_printf("recv control cmd!\n");

			memset(recvbuf, 0, sizeof(recvbuf));
			char* str = getCurrentMsg();
			u_printf("The client has received a message:    %s\n", str);
			strcpy(recvbuf, str);
			// sendto(udp_fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)&address, sizeof(address));

			p = strtok(recvbuf, "#");
			n = 0;
			while(p)
			{
				cmd[n] = p;
				n++;
				p = strtok(NULL, "#");
			}
			u_printf("cmd[1] is :%s\n", cmd[1]);
			if (strncmp(cmd[0],"action",sizeof("action"))==0)                         //控制喂水、喂食，指令格式：action#1,10
			{
				q = strtok(cmd[1], ",");
				j = 0;
				while(q)
				{
					actcmd[j] = q;
					j++;
					q = strtok(NULL, ",");
				}
				u_printf("actcmd[0] is :%s\n", actcmd[0]);
				
				if (strncmp(actcmd[0],"1",sizeof("1"))==0)
				{
					if (fdstate == 0)
					{
						fdstate = 1;                                                 //将喂食状态改为1，触发喂食线程
					}
					else
					{
						sendMessage(getCurrentUser(), "fdbusy");
						u_printf("Warning: fdbusy!\n");
					}
				}
				else if (strncmp(actcmd[0],"0",sizeof("0"))==0)
				{
					if (wtstate == 0)
					{
						wtstate = 1;                                                 //将喂水状态改为1，触发喂水线程
					}
					else
					{
						sendMessage(getCurrentUser(), "wtbusy");
						u_printf("Warning: wtbusy!\n");
					}
				}
			}
			else if(strncmp(cmd[0],"set",sizeof("set"))==0)							  //定时设置(添加和修改)
			{
				memset(read_flashbuf, 0, sizeof(read_flashbuf));
				hfuflash_read(0, read_flashbuf, sizeof(read_flashbuf));
				
				if (read_flashbuf[1] == ',') 								   									///用户flash中有定时设置
				{
					///第一步：竖线分割flash中时间值，存于指针数组flash_set_str中
					p_str = strtok(read_flashbuf, "|");
					n_str = 0;
					while(p_str)
					{
						flash_set_str[n_str] = p_str;
						n_str++;
						p_str = strtok(NULL, "|");
					}
					
					///第二步：取flash_set_str值中标识与cmd[1]进行比较
					
					k_str = n_str - 1;
					while(k_str >= 0)
					{
						memset(tmp_flash_strcmp, 0, sizeof(tmp_flash_strcmp));
						strcpy(tmp_flash_strcmp, flash_set_str[k_str]);               ///strcpy慎用
						u_printf("The tmp_flash_strcmp is: %s\n", tmp_flash_strcmp);
						
						memset(tmp_cmd_strcmp, 0, sizeof(tmp_cmd_strcmp));
						strcpy(tmp_cmd_strcmp, cmd[1]);
						u_printf("The tmp_cmd_strcmp is: %s\n", tmp_cmd_strcmp);
						
						if (strncmp(strtok(tmp_cmd_strcmp, ","), strtok(tmp_flash_strcmp, ","), 1) == 0)        ///比较时间的标识
						{
							// strcpy(flash_set_str[k_str], cmd[1]);
							flash_set_str[k_str] = cmd[1];
							
							break;
						}
						
						k_str--;
					}
					
					///第三步：连接字符串后放到store_flashbuf中
					memset(store_flashbuf, 0, sizeof(store_flashbuf));
					if (k_str < 0)								//flash中没有该标识，即表示添加
					{
						c_str = 0;
						while(c_str < n_str)
						{
							strcat(store_flashbuf, flash_set_str[c_str]);
							strcat(store_flashbuf, vertLine);
							c_str++;
						}
						strcat(store_flashbuf, cmd[1]);
					}
					else										//修改某个标识的设置
					{
						c_str = 0;
						while(c_str < n_str)
						{
							strcat(store_flashbuf, flash_set_str[c_str]);
							if (c_str < (n_str - 1))
								strcat(store_flashbuf, vertLine);
							c_str++;
						}
					}
				}
				else		///flash中为空
				{
					memset(store_flashbuf, 0, sizeof(store_flashbuf));
					strcat(store_flashbuf, cmd[1]);
				}
				
				///第四步：存储到用户flash中
				hfuflash_erase_page(0, 1);
				hfuflash_write(0, store_flashbuf, sizeof(store_flashbuf));
				u_printf("We have stored the store_flashbuf: %s\n", store_flashbuf);
				sendto(udp_fd, store_flashbuf, sizeof(store_flashbuf), 0, (struct sockaddr *)&address, sizeof(address));
				sendMessage(getCurrentUser(),"control success");
			}
			else if(strncmp(cmd[0], "cancel", sizeof("cancel")) == 0)				  //取消定时设置
			{
				memset(read_flashbuf, 0, sizeof(read_flashbuf));
				hfuflash_read(0, read_flashbuf, sizeof(read_flashbuf));
				
				if (read_flashbuf[1] == ',') 								   									///用户flash中有定时设置
				{
					///第一步：竖线分割flash中时间值，存于指针数组flash_set_str中
					p_str = strtok(read_flashbuf, "|");
					n_str = 0;
					while(p_str)
					{
						flash_set_str[n_str] = p_str;
						n_str++;
						p_str = strtok(NULL, "|");
					}
					
					///第二步：取flash_set_str值中标识与cmd[1]进行比较
					k_str = 0;
					while(k_str < n_str)
					{
				        memset(tmp_flash_strcmp, 0, sizeof(tmp_flash_strcmp));
				        strcpy(tmp_flash_strcmp, flash_set_str[k_str]);
						
						if (strncmp(strtok(tmp_flash_strcmp, ","), cmd[1], 1) == 0)
						{
							break;
						}
						
						k_str++;
					}
					
					///第三步：flash_set_str[k_str]即为要删除的定时字符串，连接其余的放到store_flashbuf中
					c_str = 0;
					memset(store_flashbuf, 0, sizeof(store_flashbuf));
					
					while(c_str < n_str) 
					{
						if(c_str == k_str)
						{
							c_str++;
							continue;
						}
						if (k_str < (n_str -1))
						{
							strcat(store_flashbuf, flash_set_str[c_str]);
							if (c_str < (n_str - 1))
								strcat(store_flashbuf, vertLine);
							c_str++;
						}
						else
						{
							strcat(store_flashbuf, flash_set_str[c_str]);
							if (c_str < (n_str - 2))
								strcat(store_flashbuf, vertLine);
							c_str++;
						}
					}
					
					///第四步：存储到用户flash中
					hfuflash_erase_page(0,1);
					hfuflash_write(0, store_flashbuf, sizeof(store_flashbuf));
					u_printf("We have stored the store_flashbuf: %s\n", store_flashbuf);
					sendto(udp_fd, store_flashbuf, sizeof(store_flashbuf), 0, (struct sockaddr *)&address, sizeof(address));
					sendMessage(getCurrentUser(),"control success");
				}
			}
			else if (strncmp(cmd[0],"flashMemory",sizeof("flashMemory"))==0)		  //读取内存中设定的时间值
			{
				memset(read_flashbuf, 0, sizeof(read_flashbuf));
				hfuflash_read(0, read_flashbuf, sizeof(read_flashbuf));
				u_printf("The time setting is: %s\n", read_flashbuf);
				sendto(udp_fd, read_flashbuf, sizeof(read_flashbuf), 0, (struct sockaddr *)&address, sizeof(address));
				
				sendMessage(getCurrentUser(), read_flashbuf);
			}
			// else if (strncmp(cmd[0],"clearAll",sizeof("clearAll"))==0)                //清空内存中所有设定的时间值
			// {
				// hfuflash_erase_page(0,1);
				// sendMessage(getCurrentUser(),"control success");
			// }
		}
	}
}

//外网模式下，服务器在线测试
USER_FUNC void pingTest(void *arg) 
{
	while (getPingServerThreadFlag()) 
	{
		sleep(5);
		pingServer();
		sleep(120);
		u_printf("check ping result");
		if (getPingResultFlag()) 
		{
			//if ping result is correct
			setPingResultFlag(0);
		}
		else 
		{
			//若ping服务器无反应，则释放资源并让主板重新上线
			resetPingData();
			//关闭socket连接
			
			closeSocket(getSocketFd());

			sleep(10);
      //10s后主板开始重新上线
			initPingData();

			connectServer();

			loginServer();

			if (hfthread_create(recvTest, "RECV_THREAD", 256, NULL,HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS) 
			{
				u_printf("create recvMessage thread fail\n");

			}

			if (hfthread_create(pingTest, "PING_THREAD", 256, NULL,HFTHREAD_PRIORITIES_LOW, NULL, NULL) != HF_SUCCESS) 
			{
				u_printf("create pingServer thread fail\n");

			}

		}
	}
}

///喂食线程
USER_FUNC void fdthread(void *arg)
{
	while(1)
	{
		if (fdstate == 1)
		{
			u_printf("Start to feed %d secs!\n", atoi(actcmd[1]));
			
			control_feed[3]=(char)(atoi(actcmd[1]));
			if (hfuart_send(huart, control_feed, sizeof(control_feed), 0) > 0)
			{
				sendMessage(getCurrentUser(),"control success");
			}
			
			sleep((int)(atoi(actcmd[1]) / 0.7343));
			fdstate = 0;
			u_printf("Feeding has finished!\n");
		}
		
		msleep(100);
	}
}

///喂水线程
USER_FUNC void wtthread(void *arg)
{
	while(1)
	{
		if (wtstate == 1)
		{
			u_printf("Start to water %d secs!\n", atoi(actcmd[1]));
			
			control_water[3]=(char)(atoi(actcmd[1]));
			if (hfuart_send(huart, control_water, sizeof(control_water), 0) > 0)
			{
				sendMessage(getCurrentUser(),"control success");
			}

			sleep((int)(atoi(actcmd[1]) / 0.7343));
			wtstate = 0;
			u_printf("Watering has finished!\n");
		}
		
		msleep(100);
	}
}

///定时线程，需要连接跳线
USER_FUNC void time_fdthread(void *arg)
{
	while(1)
	{
		///第一步：读时钟模块上的时间值
		hfuart_send(huart, query_time_cmd, sizeof(query_time_cmd), 0);
		msleep(500);
		if (hfuart_recv(huart, recv_timebuf, sizeof(recv_timebuf), 0) > 0)
		{
			memset(time_str, 0, sizeof(time_str));
			hour = BCDToInt(recv_timebuf[0]);			 ///将BCD码转成十进制的时
			min  = BCDToInt(recv_timebuf[1]);
			time_str[0] = (hour / 10) + 48;              ///数字到字符
			time_str[1] = (hour % 10) + 48;
			time_str[2] = ':';
			time_str[3] = (min / 10) + 48;
			time_str[4] = (min % 10) + 48;
			
			// sendto(udp_fd, time_str, sizeof(time_str), 0, (struct sockaddr *)&address, sizeof(address));
		}
		
		
		///第二步：取内存中设定的时间值与当前获取的时间值进行匹配
		///	 备注：每次都轮询完flash中所有的时间设置，标识1~4为喂食，标识5~8为喂水，这样set和cancel不用作修改
		memset(read_timed_flashbuf, 0, sizeof(read_timed_flashbuf));
		hfuflash_read(0, read_timed_flashbuf, sizeof(read_timed_flashbuf));
		
		if (read_timed_flashbuf[1] == ',')							///如果不空
		{
			///2.1 分割整个竖线连接的字符串read_timed_flashbuf数组
			p_timed_str = strtok(read_timed_flashbuf, "|");
			n_timed = 0;
			while(p_timed_str)
			{
				timed_flash_set_str[n_timed] = p_timed_str;
				n_timed++;
				p_timed_str = strtok(NULL, "|");
			}
			
			///2.2 匹配时间并喂食，将需要连接的字符串标识存入存储数组中
			k_timed = -1;
			memset(store_timed_flashbuf, 0, sizeof(store_timed_flashbuf));
			while(k_timed < (n_timed - 1))
			{
				k_timed++;
				memset(timed_cmpbuf, 0, sizeof(timed_cmpbuf));
				strcpy(timed_cmpbuf, timed_flash_set_str[k_timed]);				///timed_flash_set_str[0 -  (n_timed-1)]的格式为：1,08:30,1,3
				// u_printf("timed_cmpbuf is: %s\n", timed_cmpbuf);
				
				///以","分割timed_cmpbuf
				k_cmd = 0;
				p_cmd = strtok(timed_cmpbuf, ",");
				while(p_cmd)
				{
					everyTimeCmd[k_cmd] = p_cmd;
					k_cmd++;
					p_cmd = strtok(NULL, ",");
				}
				
				///取everyTimeCmd[1]中时间值与时钟模块time_str比较
				if (strcmp(time_str, everyTimeCmd[1]) == 0)
				{
					sendto(udp_fd, "Got it", sizeof("Got it"), 0, (struct sockaddr *)&address, sizeof(address));
					//注：此处添加定时喂食、喂水的判断
					
					control_feed[3]=(char)(atoi(everyTimeCmd[3]) * 5);
					hfuart_send(huart, control_feed, sizeof(control_feed), 0);
					if (strcmp(everyTimeCmd[2], "1") == 0)
					{
						continue;
					}
				}
				
				///连接字符串
				strcat(store_timed_flashbuf, timed_flash_set_str[k_timed]);
				strcat(store_timed_flashbuf, vertLine);
				
			}
			
			///写进flash中
			hfuflash_erase_page(0,1);
			hfuflash_write(0, store_timed_flashbuf, sizeof(store_timed_flashbuf));
			sendto(udp_fd, "Only pet(A496): ", sizeof("Only pet(A496): "), 0, (struct sockaddr *)&address, sizeof(address));
			sendto(udp_fd, store_timed_flashbuf, sizeof(store_timed_flashbuf), 0, (struct sockaddr *)&address, sizeof(address));
		}
		
		sleep(75);							///保证每分钟轮询一次
	}
}





                                      
#endif
