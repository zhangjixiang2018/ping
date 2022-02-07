#include "ping.h"

#define IPLEN 30

//st:Ҫд����ַ�����n:Ҫд��ĳ���
//������ĳ��ȵ���n�����������س���ʱ��������
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
			//�յ��������ж�����ַ�
			while (getchar() != '\n')
			{
				continue;
			}
		}

	}
	return ret_val;
}

//��������ַ�������ȡip��ַ������Ƿ����룬������ת��ΪIP
bool entryToIp(char* str, char* ipStr)
{

	struct hostent* host;
	s_fgets(str, IPLEN);
	//puts(str);
	if (strlen(str) < 5)
	{
		puts("�������");
		return false;
	}
	memmove(ipStr, str + 5, strlen(str) - 5);
	ipStr[strlen(str) - 5] = '\0';
	if (ipStr[0] == ' ' || ipStr[0] == '\0')
	{
		puts("������󣬿�����ipǰ�����һ���ո�");
		return false;
	}

	//����ַ������ַ���˵��ping�������������ｫ����ת��ΪIP
	if (isalpha(ipStr[0]))
	{
		if ((host = gethostbyname(ipStr)) == NULL)
		{
			puts("�������������ȷ�����߲����ڸ�����");
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
	char ipStr[IPLEN];//ping��ip��������

	//��ʼ��WSA
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		puts("��ʼ��ʧ��");
		return -1;
	}

	//����ping ��ַ/����
	if (!entryToIp(str, ipStr))//�������
	{
		return -1;
	}
	//puts(ipStr);

	ping(ipStr);

	WSACleanup();
	return 0;
}