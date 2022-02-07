#include "ping.h"
#define PINGNUM 4 //ping�Ĵ���

//����У��ͣ�����У������ݽ���16λ��ӣ�Ȼ��ȡ�����������ֽ���Ϊ��������������β��һ���ֽڵ�0�ճ�ż��
//����1��ҪУ������ݣ�������ICMP����/Ӧ���ģ�����2��Э�����ͣ����ݲ�ͬ��Э����㱻У�����ݵĳ���
uint16_t getCheckSum(void* protocol, const char* type)
{
	uint32_t checkNum;//��32λ�洢����Ϊ�����ݽ���16λ��ӿ��ܻ���������Ҫ������ĸ�λ�ӵ���λ
	uint16_t* data = (uint16_t*)protocol;//Ҫ���������
	uint32_t size;
	uint16_t tmp;
	//���㱻У�����ݵĳ���,�ж��ٸ��ֽ�
	if (strcmp((char*)type, "ICMP")==0)
	{
		size = sizeof(ICMPReq);
	}

	checkNum = 0;
	while (size > 1)//�����ݽ���16λ���
	{
		checkNum += *data++;
		size -= 2;
	}
	//�������ֽ���Ϊ�����Ĵ���
	if (size == 1)
	{
		tmp = *data;
		tmp = tmp & 0xff00;
		checkNum = checkNum + tmp;
	}
	//���������λ�����ӵ���λ
	while (checkNum >> 16)
	{
		checkNum = (checkNum >> 16) + (checkNum & 0x0000FFFF);
	}

	//ȡ��
	return (~checkNum);
}

//����ICMP������.
//����1��socket������2��Ŀ���ַ������3���������
//���ͳɹ�����true,ʧ�ܷ���false
bool sendICMPReq(SOCKET mysocket,sockaddr_in destAddr,int num)
{
	int ret;
	//����ICMP������
	struct ICMPReq icmpReq;
	//����ICMP������
	icmpReq.icmphead.type = 8;//���ñ������ͣ�ICMP������Ժͻ���Ӧ������Ͷ�Ϊ8
	icmpReq.icmphead.code = 0;//����
	icmpReq.icmphead.checkSum = 0;//У��ͳ�ʼΪ0
	//��ȡ���̺ţ��������ܸ���У������ʶ����յ����ǲ���������̷���ȥ��ICMP����
	icmpReq.icmphead.ident = (uint16_t)GetCurrentProcessId();
	icmpReq.icmphead.seqNum = ++num;//�������
	//���淢�͵�ʱ��
	icmpReq.timeStamp = GetTickCount();
	icmpReq.icmphead.checkSum = getCheckSum((void*)&icmpReq, "ICMP");
	//printf("send icmpReq.icmphead.ident=%d\n", icmpReq.icmphead.ident);
	//printf("sendStamp=%d\n", icmpReq.timeStamp);
	//printf("sendCheakSum = %d\n", icmpReq.icmphead.checkSum);

	//printf("getCheckSum()=%d\n", getCheckSum((void*)&icmpReq, "ICMP"));
	//printf("getCheckSum()1=%d\n", getCheckSum1((void*)&icmpReq, "ICMP"));
	//printf("icmpReq.timeStamp=%u\n", icmpReq.timeStamp);

	ret=sendto(mysocket, (char*)&icmpReq, sizeof(icmpReq), 0, (sockaddr*)&destAddr, sizeof(sockaddr));

	if (ret == SOCKET_ERROR)
	{
		puts("����ʧ��");
		return false;
	}

	return true;
}

//�ȴ��׽��ֵ�����
int waitForSocekt(SOCKET mysocket)
{
	fd_set readfd;
	timeval timeOut;
	readfd.fd_count = 1;
	readfd.fd_array[0] = mysocket;
	//���ó�ʱʱ��Ϊ5��
	timeOut.tv_sec = 5;
	timeOut.tv_usec = 0;
	return (select(1,& readfd, 0, 0,& timeOut));
}

//����ICMPӦ����
uint32_t readICMPAnswer(SOCKET mysocket,char &TTL,sockaddr_in *srcAdd)
{
	ICMPAns ICMPReply;//ICMPӦ����
	int len;
	int ret;
	int cheakSum;

	len = sizeof(sockaddr_in);
	//����ICMPӦ����
	ret = recvfrom(mysocket, (char*)&ICMPReply, sizeof(ICMPReply), 0, (sockaddr*)&srcAdd, &len);

	if (ret == SOCKET_ERROR)
	{
		puts("����socket����");
		return -1;
	}
	//�жϽ��յ������Ƿ�Ϊ�Լ��������ĵ���Ӧ
	if (ICMPReply.icmpanswer.icmphead.ident != GetCurrentProcessId())
	{
		puts("�޷�����Ŀ������");
		return -1;
	}

	//printf("GetCurrentProcessId()=%d\n", GetCurrentProcessId());
	
	//printf("recv id=%d\n", ICMPReply.icmpanswer.icmphead.ident);
	//��ȡУ��Ͳ����¼���
	cheakSum = ICMPReply.icmpanswer.icmphead.checkSum;
	//printf("cheakSum=%d\n", cheakSum);
	//
	ICMPReply.icmpanswer.icmphead.checkSum = 0;
	//����У����Ƿ���ȷ
	if (cheakSum == getCheckSum((void*)&ICMPReply.icmpanswer, "ICMP"))
	{
		//printf("ICMPReply.iphead.timetoLive=%d\n", ICMPReply.iphead.timetoLive);
		//printf("reply cheaksum = %d\n", getCheckSum((void*)&ICMPReply.icmpanswer, "ICMP"));
		TTL = (ICMPReply.iphead.timetoLive);
		//printf("TTL=%d\n", TTL);
		//printf("ICMPReply.icmpanswer.timeStamp=%d\n", ICMPReply.icmpanswer.timeStamp);
		return ICMPReply.icmpanswer.timeStamp;
	}

	return -1;
}

bool doPing(SOCKET mysocket,sockaddr_in destAddr, sockaddr_in &scrAddr,int seqNum)
{
	uint32_t sendtime;//����ʱ��
	char TTL;//����ʱ��
	int ret;

	//printf("****************************************\n");
	//��������
	if (!sendICMPReq(mysocket, destAddr, seqNum))
	{
		//��������ʧ��
		return false;
	}
	
	//�ȴ�����
	ret = waitForSocekt(mysocket);
	if (ret == SOCKET_ERROR)
	{
		puts("socket ��������");
		return false;
	}
	if (!ret)
	{
		puts("����ʱ");
		return false;
	}

	TTL = 0;
	//printf("TTL1=%d\n", TTL);
	sendtime = readICMPAnswer(mysocket, TTL, &scrAddr);
	if (sendtime == -1)
	{
		return false;
	}

	//printf("TTL2=%d\n", TTL);
	printf("���� %s �Ļظ����ֽ�=%d ʱ��=%d TTL=%d \n",
		inet_ntoa(destAddr.sin_addr),
		sizeof(ICMPReq::data),
		GetTickCount() - sendtime,
		abs((int)TTL)
	);

	return true;
}


void ping(char* destIP)
{
	SOCKET raw_socket;//����ԭʼ�׽���
	sockaddr_in destAddr;//Ŀ��������ַ
	sockaddr_in scrAddr;//Ӧ�����е�Դ��ַ
	int ret;
	int i;
	int sendNum,recvNum;//ICMP���ķ��͸��������ո���

	//�����׽���
	//TCP/IPЭ��أ�RAWģʽ��ICMPģʽ
	//RAW��������һ��ԭʼ�׽��֣���Ϳ��Է��ʵ�������·������ݣ�Ҳ����˵��������IPͷ������Ҳ�����õ��ˡ�
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(0);
	destAddr.sin_addr.S_un.S_addr = inet_addr(destIP);

	sendNum = 0;
	recvNum = 0;
	printf("����Ping %s ����%d�ֽڵ�����\n",inet_ntoa(destAddr.sin_addr),sizeof(ICMPReq::data));
	for (i = 0; i < PINGNUM; i++)
	{
		sendNum++;
		if (doPing(raw_socket, destAddr, scrAddr, i))
		{
			recvNum++;
		}
		
	}
	printf("%s ��Pingͳ����Ϣ\n", inet_ntoa(destAddr.sin_addr));
	printf("\t���ݰ����ѷ��� = %d,�ѽ��� = %d,��ʧ = %d(%d%%��ʧ)\n", sendNum, recvNum, sendNum - recvNum,(sendNum - recvNum)*100/sendNum);

	ret = closesocket(raw_socket);

	if (ret == SOCKET_ERROR)
	{
		puts("�ر�socket��������");
		return ;
	}
	
}