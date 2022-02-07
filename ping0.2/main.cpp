#include"ping.h"


#define STR_LEN 50//
#define IP_LEN 20//

//从键盘输入字符串，ping ip
//字符串输入处理函数
char* s_fputs(char *st ,int n)
{
	char* ret;
	char* find;

	ret = fgets(st, n, stdin);

	if (ret)//当ret!=NULL，
	{
		find = strchr(st, '\n');//查找换行符
		if (find)//查找到换行符将其换成字符串结束标志
		{
			*find = '\0';
		}
		else//没有找到换行符说明输入的字符串超过限定的长度，将多余的字符串回收
		{
			while (getchar() != '\n')
				continue;
		}
	}
	return ret;
}

//处理输入的字符串，将其转化为IP地址
//参数1：输入的字符串
//参数2：用于存储处理后的IP
bool entryToIPaddr(char* str,char* destIP)
{
	struct hostent *host;
	char* temIP;
	
	//输入的字符串长度小于5
	if (strlen(str)<5) 
	{
		puts("输入有误");
		return false;
	}

	//将ping后面的字符串复制给destip,这个字符串可能是ip也可能是域名,
	//若这个字符串的第一个字符是空格或者是结束符说明输入错误
	memmove(destIP, str+5, strlen(str) - 5);
	destIP[strlen(str) - 5] = '\0';
	if (destIP[0] == ' ' || destIP[0] == '\0')
	{
		puts("输入有误");
		return false;
	}

	//destIP的第一个字符为字母说明这个字符串为域名，将域名转化为IP
	if (isalpha(destIP[0]))
	{
		host = gethostbyname(destIP);
		if (!host)
		{
			puts("域名转化IP失败");
			return false;
		}
		temIP = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);//将网络字节序转化为主机字节序
		memmove(destIP, temIP, strlen(temIP));//将转化来的IP存入destip
		destIP[strlen(temIP)] = '\0';
	}
	
	return true;
}
int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsadata;
	char str[STR_LEN];
	char destIP[IP_LEN];

	//初始化WSA
	if (WSAStartup(sockVersion, &wsadata) != 0)
	{
		puts("初始化失败");
		return -1;
	}

	//从键盘输入字符串，ping ip
	s_fputs(str, STR_LEN);

	puts(str);
	//处理输入的字符串为IP
	if (!entryToIPaddr(str, destIP))
	{
		puts("转化IP出现错误");
		return -1;
	}
	puts(destIP);

	ping(destIP);
	WSACleanup();
	return 0;
}