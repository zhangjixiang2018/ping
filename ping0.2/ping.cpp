#include"ping.h"

#define  PINGNUM 4

sockaddr_in destAddr;//目标地址
sockaddr_in srcAddr;
uint8_t TTL;
uint32_t sendTime;


//参数1：要校验的数据，这里是ICMP请求/应答报文，参数2：协议类型，根据不同的协议计算被校验数据的长度
uint16_t getCheckSum(void* protocol , const char* type)
{
	uint32_t size;//被校验数据的大小
	uint16_t *data = (uint16_t*)protocol;//被校验的数据
	uint32_t sum=0;//校验和
	uint16_t tmp;

	//根据不同的协议类型确定被校验数据的大小
	if (strcmp(type, "ICMP")==0)
	{	//ICMP数据的大小
		size = sizeof(ICMPrequest);
	}

	sum = 0;
	//求校验和
	while (size > 1)
	{
		sum = sum + *data++;
		size -= 2;
	}
	//当被求和数据的大小为奇数时，在数据末尾添0凑成偶数再相加
	if (size == 1)
	{
		tmp = *data;
		tmp = tmp & 0xff00;
		sum += tmp;
	}
	//将溢出到高位的数据加到低位
	while (sum >> 16)
	{
		sum = (sum >> 16) + (sum & 0x0000ffff);
	}

	return (~sum);
}

//发送ICMP请求数据
//参数1：socket 参数2：发送ICMP报文的序列号
bool sendICMP(SOCKET mysocket,int num)
{
	struct ICMPrequest sendMsg;//ICMP发送消息
	int ret;

	sendMsg.icmphead.type = 8;//类型
	sendMsg.icmphead.code = 0;//代码
	sendMsg.icmphead.cheaksum = 0;//初始化校验和为0
	//获取当前进程ID，这样就可以根据接收到的标识符判断是不是本进程发出的ICMP报文
	sendMsg.icmphead.ident = (uint16_t)GetCurrentProcessId();
	sendMsg.icmphead.seq = ++num;//序列号，当前序列号为0
	sendMsg.timeStamp = GetTickCount();//获取并保存发送时间
	//计算并保存校验和
	sendMsg.icmphead.cheaksum = getCheckSum((void*)&sendMsg, "ICMP");

	//printf("GetCurrentProcessId()1 =%d\n", GetCurrentProcessId());
	//printf("sendMsg.icmphead.ident=%d\n", sendMsg.icmphead.ident);
	ret=sendto(mysocket, (char*)&sendMsg, sizeof(sendMsg), 0, (sockaddr*)&destAddr, sizeof(sockaddr));
	
	if (ret == SOCKET_ERROR)
	{
		puts("发送消息错误");
		return false;
	}

	return true;
}

//等待套接字的数据
int waitForSocket(SOCKET mysocket)
{

	fd_set readfd;
	timeval timeout;//设置超时时间

	readfd.fd_count = 1;
	readfd.fd_array[0] = mysocket;
	timeout.tv_sec = 5; // 设置超时时间为5秒
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
		puts("发送socket失败");
		return false;
	}

	//printf("GetCurrentProcessId()2 =%d\n", GetCurrentProcessId());
	//printf("icmpAnswer.icmp.icmphead.ident=%d\n", icmpAnswer.icmp.icmphead.ident);
	if (icmpAnswer.icmp.icmphead.ident != GetCurrentProcessId())
	{
		puts("无法访问目标主机");
		return false;
	}

	//检查校验和
	cheakSum = icmpAnswer.icmp.icmphead.cheaksum;
	icmpAnswer.icmp.icmphead.cheaksum = 0;
	if (cheakSum != getCheckSum((void*)&icmpAnswer.icmp, "ICMP"))
	{
		puts("校验和错误");
		return false;
	}

	TTL = icmpAnswer.iphead.TTL;
	sendTime = icmpAnswer.icmp.timeStamp;

	return true;
}

//
//参数1：socket 参数2：发送ICMP报文的序列号
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
		puts("等待socket 发送错误");
		return false;
	}
	if (ret == 0)
	{
		puts("请求超时");
		return false;
	}

	if (!readICMPAnswer(mysocket))
	{
		return false;
	}
	
	//printf("GetTickCount()=%d   sendTime=%d   GetTickCount()-sendTime=%d\n", GetTickCount(), sendTime, GetTickCount() - sendTime);
	printf("来自 %s 的回复：字节=%d 时间=%d TTL=%d \n",
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

	//创建原生socket
	//TCP/IP协议簇，RAW模式，ICMP协议
	//RAW创建的是一个原生socket，它最低可以拿到数据链路层的数据。这里需要拿到IP层的数据所以要原生socket
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	//设置目标地址参数
	destAddr.sin_family = AF_INET;//协议
	destAddr.sin_port = htons(0);//端口
	destAddr.sin_addr.S_un.S_addr = inet_addr(destIP);//IP


	for (i = 0; i < PINGNUM; i++)
	{
		doping(raw_socket, i);
		Sleep(1000);
	}

	closesocket(raw_socket);

	return true;
}