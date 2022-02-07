#include"ping.h"

#define  PINGNUM 4

sockaddr_in destAddr;//Ŀ���ַ
sockaddr_in srcAddr;
uint8_t TTL;
uint32_t sendTime;


//����1��ҪУ������ݣ�������ICMP����/Ӧ���ģ�����2��Э�����ͣ����ݲ�ͬ��Э����㱻У�����ݵĳ���
uint16_t getCheckSum(void* protocol , const char* type)
{
	uint32_t size;//��У�����ݵĴ�С
	uint16_t *data = (uint16_t*)protocol;//��У�������
	uint32_t sum=0;//У���
	uint16_t tmp;

	//���ݲ�ͬ��Э������ȷ����У�����ݵĴ�С
	if (strcmp(type, "ICMP")==0)
	{	//ICMP���ݵĴ�С
		size = sizeof(ICMPrequest);
	}

	sum = 0;
	//��У���
	while (size > 1)
	{
		sum = sum + *data++;
		size -= 2;
	}
	//����������ݵĴ�СΪ����ʱ��������ĩβ��0�ճ�ż�������
	if (size == 1)
	{
		tmp = *data;
		tmp = tmp & 0xff00;
		sum += tmp;
	}
	//���������λ�����ݼӵ���λ
	while (sum >> 16)
	{
		sum = (sum >> 16) + (sum & 0x0000ffff);
	}

	return (~sum);
}

//����ICMP��������
//����1��socket ����2������ICMP���ĵ����к�
bool sendICMP(SOCKET mysocket,int num)
{
	struct ICMPrequest sendMsg;//ICMP������Ϣ
	int ret;

	sendMsg.icmphead.type = 8;//����
	sendMsg.icmphead.code = 0;//����
	sendMsg.icmphead.cheaksum = 0;//��ʼ��У���Ϊ0
	//��ȡ��ǰ����ID�������Ϳ��Ը��ݽ��յ��ı�ʶ���ж��ǲ��Ǳ����̷�����ICMP����
	sendMsg.icmphead.ident = (uint16_t)GetCurrentProcessId();
	sendMsg.icmphead.seq = ++num;//���кţ���ǰ���к�Ϊ0
	sendMsg.timeStamp = GetTickCount();//��ȡ�����淢��ʱ��
	//���㲢����У���
	sendMsg.icmphead.cheaksum = getCheckSum((void*)&sendMsg, "ICMP");

	//printf("GetCurrentProcessId()1 =%d\n", GetCurrentProcessId());
	//printf("sendMsg.icmphead.ident=%d\n", sendMsg.icmphead.ident);
	ret=sendto(mysocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&destAddr, sizeof(sockaddr));
	
	if (ret == SOCKET_ERROR)
	{
		puts("������Ϣ����");
		return false;
	}

	return true;
}

//�ȴ��׽��ֵ�����
int waitForSocket(SOCKET mysocket)
{

	fd_set readfd;
	timeval timeout;//���ó�ʱʱ��

	readfd.fd_count = 1;
	readfd.fd_array[0] = mysocket;
	timeout.tv_sec = 5; // ���ó�ʱʱ��Ϊ5��
	timeout.tv_usec = 0;

	return(select(1, &readfd, 0, 0, &timeout));
}

bool readICMPAnswer(SOCKET mysocket)
{
	ICMPAnwser icmpAnswer;//
	int ret;
	int len;
	uint16_t cheakSum;

	len = sizeof(sockaddr);
	ret = recvfrom(mysocket, (char*)&icmpAnswer, sizeof(icmpAnswer), 0, (sockaddr*)&srcAddr, &len);

	if (ret == SOCKET_ERROR)
	{
		puts("����socketʧ��");
		return false;
	}

	//printf("GetCurrentProcessId()2 =%d\n", GetCurrentProcessId());
	//printf("icmpAnswer.icmp.icmphead.ident=%d\n", icmpAnswer.icmp.icmphead.ident);
	if (icmpAnswer.icmp.icmphead.ident != GetCurrentProcessId())
	{
		puts("�޷�����Ŀ������");
		return false;
	}

	//���У���
	cheakSum = icmpAnswer.icmp.icmphead.cheaksum;
	icmpAnswer.icmp.icmphead.cheaksum = 0;
	if (cheakSum != getCheckSum((void*)&icmpAnswer.icmp, "ICMP"))
	{
		puts("У��ʹ���");
		return false;
	}

	TTL = icmpAnswer.iphead.TTL;
	sendTime = icmpAnswer.icmp.timeStamp;

	return true;
}

//
//����1��socket ����2������ICMP���ĵ����к�
bool doping(SOCKET mysocket,int num)
{

	int ret;
	//
	if (!sendICMP(mysocket, num))
	{
		
		return false;
	}

	ret = waitForSocket(mysocket);
	if (ret == SOCKET_ERROR)
	{
		puts("�ȴ�socket ���ʹ���");
		return false;
	}
	if (ret == 0)
	{
		puts("����ʱ");
		return false;
	}

	if (!readICMPAnswer(mysocket))
	{
		return false;
	}
	
	//printf("GetTickCount()=%d   sendTime=%d   GetTickCount()-sendTime=%d\n", GetTickCount(), sendTime, GetTickCount() - sendTime);
	printf("���� %s �Ļظ����ֽ�=%d ʱ��=%d TTL=%d \n",
		inet_ntoa(destAddr.sin_addr),
		sizeof(ICMPrequest::data),
		GetTickCount() - sendTime,
		abs((int)TTL)
	);

	return true;
}

bool ping(char* destIP)
{
	SOCKET raw_socket;//
	int i;

	//����ԭ��socket
	//TCP/IPЭ��أ�RAWģʽ��ICMPЭ��
	//RAW��������һ��ԭ��socket������Ϳ����õ�������·������ݡ�������Ҫ�õ�IP�����������Ҫԭ��socket
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	//����Ŀ���ַ����
	destAddr.sin_family = AF_INET;//Э��
	destAddr.sin_port = htons(0);//�˿�
	destAddr.sin_addr.S_un.S_addr = inet_addr(destIP);//IP


	for (i = 0; i < PINGNUM; i++)
	{
		doping(raw_socket, i);
		Sleep(1000);
	}

	closesocket(raw_socket);

	return true;
}