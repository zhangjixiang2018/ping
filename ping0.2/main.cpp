#include"ping.h"


#define STR_LEN 50//
#define IP_LEN 20//

//�Ӽ��������ַ�����ping ip
//�ַ������봦����
char* s_fputs(char *st ,int n)
{
	char* ret;
	char* find;

	ret = fgets(st, n, stdin);

	if (ret)//��ret!=NULL��
	{
		find = strchr(st, '\n');//���һ��з�
		if (find)//���ҵ����з����任���ַ���������־
		{
			*find = '\0';
		}
		else//û���ҵ����з�˵��������ַ��������޶��ĳ��ȣ���������ַ�������
		{
			while (getchar() != '\n')
				continue;
		}
	}
	return ret;
}

//����������ַ���������ת��ΪIP��ַ
//����1��������ַ���
//����2�����ڴ洢������IP
bool entryToIPaddr(char* str,char* destIP)
{
	struct hostent *host;
	char* temIP;
	
	//������ַ�������С��5
	if (strlen(str)<5) 
	{
		puts("��������");
		return false;
	}

	//��ping������ַ������Ƹ�destip,����ַ���������ipҲ����������,
	//������ַ����ĵ�һ���ַ��ǿո�����ǽ�����˵���������
	memmove(destIP, str+5, strlen(str) - 5);
	destIP[strlen(str) - 5] = '\0';
	if (destIP[0] == ' ' || destIP[0] == '\0')
	{
		puts("��������");
		return false;
	}

	//destIP�ĵ�һ���ַ�Ϊ��ĸ˵������ַ���Ϊ������������ת��ΪIP
	if (isalpha(destIP[0]))
	{
		host = gethostbyname(destIP);
		if (!host)
		{
			puts("����ת��IPʧ��");
			return false;
		}
		temIP = inet_ntoa(*(struct in_addr*)host->h_addr_list[0]);//�������ֽ���ת��Ϊ�����ֽ���
		memmove(destIP, temIP, strlen(temIP));//��ת������IP����destip
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

	//��ʼ��WSA
	if (WSAStartup(sockVersion, &wsadata) != 0)
	{
		puts("��ʼ��ʧ��");
		return -1;
	}

	//�Ӽ��������ַ�����ping ip
	s_fputs(str, STR_LEN);

	puts(str);
	//����������ַ���ΪIP
	if (!entryToIPaddr(str, destIP))
	{
		puts("ת��IP���ִ���");
		return -1;
	}
	puts(destIP);

	ping(destIP);
	WSACleanup();
	return 0;
}