#include "ping.h"

#define IPLEN 30

//st:要写入的字符串，n:要写入的长度
//当输入的长度到达n，或者遇到回车符时结束输入
char* s_fgets(char* st, int n)
{
	char* ret_val;
	char* find;

	ret_val = fgets(st, n, stdin);

	if (ret_val)
	{
		find = strchr(st, '\n');

		if (find)
		{
			*find = '\0';
		}
		else
		{
			//收掉命令行中多余的字符
			while (getchar() != '\n')
			{
				continue;
			}
		}

	}
	return ret_val;
}

//从输入的字符串中提取ip地址。处理非法输入，将域名转化为IP
bool entryToIp(char* str, char* ipStr)
{

	struct hostent* host;
	s_fgets(str, IPLEN);
	//puts(str);
	if (strlen(str) < 5)
	{
		puts("输入错误");
		return false;
	}
	memmove(ipStr, str + 5, strlen(str) - 5);
	ipStr[strlen(str) - 5] = '\0';
	if (ipStr[0] == ' ' || ipStr[0] == '\0')
	{
		puts("输入错误，可能是ip前面多了一个空格");
		return false;
	}

	//如果字符串是字符，说明ping的是域名。这里将域名转化为IP
	if (isalpha(ipStr[0]))
	{
		if ((host = gethostbyname(ipStr)) == NULL)
		{
			puts("输入的域名不正确，或者不存在该域名");
			return false;
		}

		str = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);
		memmove(ipStr, str, strlen(str));
		ipStr[strlen(str)] = '\0';
		//puts(ipStr);
		/*for (int i = 0; host->h_addr_list[i]; i++)
		{
			printf("IP addr %d: %s\n", 
				i + 1, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
		}*/
	}
	return true;
}
int main()
{
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	char str[IPLEN];
	char ipStr[IPLEN];//ping的ip或者域名

	//初始化WSA
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		puts("初始化失败");
		return -1;
	}

	//输入ping 地址/域名
	if (!entryToIp(str, ipStr))//输入错误
	{
		return -1;
	}
	//puts(ipStr);

	ping(ipStr);

	WSACleanup();
	return 0;
}