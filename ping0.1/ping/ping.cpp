#include "ping.h"
#define PINGNUM 4 //ping的次数

//计算校验和，将被校验的数据进行16位相加，然后取反，若数据字节数为奇数，则在数据尾补一个字节的0凑成偶数
//参数1：要校验的数据，这里是ICMP请求/应答报文，参数2：协议类型，根据不同的协议计算被校验数据的长度
uint16_t getCheckSum(void* protocol, const char* type)
{
	uint32_t checkNum;//用32位存储是因为将数据进行16位相加可能会产生溢出，要将溢出的高位加到低位
	uint16_t* data = (uint16_t*)protocol;//要检验的数据
	uint32_t size;
	uint16_t tmp;
	//计算被校验数据的长度,有多少个字节
	if (strcmp((char*)type, "ICMP")==0)
	{
		size = sizeof(ICMPReq);
	}

	checkNum = 0;
	while (size > 1)//将数据进行16位相加
	{
		checkNum += *data++;
		size -= 2;
	}
	//对数据字节数为奇数的处理
	if (size == 1)
	{
		tmp = *data;
		tmp = tmp & 0xff00;
		checkNum = checkNum + tmp;
	}
	//将溢出到高位的数加到低位
	while (checkNum >> 16)
	{
		checkNum = (checkNum >> 16) + (checkNum & 0x0000FFFF);
	}

	//取反
	return (~checkNum);
}

//发送ICMP请求报文.
//参数1：socket，参数2：目标地址，参数3：报文序号
//发送成功返回true,失败返回false
bool sendICMPReq(SOCKET mysocket,sockaddr_in destAddr,int num)
{
	int ret;
	//创建ICMP请求报文
	struct ICMPReq icmpReq;
	//设置ICMP请求报文
	icmpReq.icmphead.type = 8;//设置报文类型，ICMP请求回显和回显应答的类型都为8
	icmpReq.icmphead.code = 0;//代码
	icmpReq.icmphead.checkSum = 0;//校验和初始为0
	//获取进程号，这样就能根据校验码来识别接收到的是不是这个进程发出去的ICMP报文
	icmpReq.icmphead.ident = (uint16_t)GetCurrentProcessId();
	icmpReq.icmphead.seqNum = ++num;//设置序号
	//保存发送的时间
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
		puts("发送失败");
		return false;
	}

	return true;
}

//等待套接字的数据
int waitForSocekt(SOCKET mysocket)
{
	fd_set readfd;
	timeval timeOut;
	readfd.fd_count = 1;
	readfd.fd_array[0] = mysocket;
	//设置超时时间为5秒
	timeOut.tv_sec = 5;
	timeOut.tv_usec = 0;
	return (select(1,& readfd, 0, 0,& timeOut));
}

//解析ICMP应答报文
uint32_t readICMPAnswer(SOCKET mysocket,char &TTL,sockaddr_in *srcAdd)
{
	ICMPAns ICMPReply;//ICMP应答报文
	int len;
	int ret;
	int cheakSum;

	len = sizeof(sockaddr_in);
	//接收ICMP应答报文
	ret = recvfrom(mysocket, (char*)&ICMPReply, sizeof(ICMPReply), 0, (sockaddr*)&srcAdd, &len);

	if (ret == SOCKET_ERROR)
	{
		puts("接收socket错误");
		return -1;
	}
	//判断接收到报文是否为自己发出报文的响应
	if (ICMPReply.icmpanswer.icmphead.ident != GetCurrentProcessId())
	{
		puts("无法访问目的主机");
		return -1;
	}

	//printf("GetCurrentProcessId()=%d\n", GetCurrentProcessId());
	
	//printf("recv id=%d\n", ICMPReply.icmpanswer.icmphead.ident);
	//读取校验和并重新计算
	cheakSum = ICMPReply.icmpanswer.icmphead.checkSum;
	//printf("cheakSum=%d\n", cheakSum);
	//
	ICMPReply.icmpanswer.icmphead.checkSum = 0;
	//计算校验和是否正确
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
	uint32_t sendtime;//发送时间
	char TTL;//生存时间
	int ret;

	//printf("****************************************\n");
	//发送数据
	if (!sendICMPReq(mysocket, destAddr, seqNum))
	{
		//发送数据失败
		return false;
	}
	
	//等待数据
	ret = waitForSocekt(mysocket);
	if (ret == SOCKET_ERROR)
	{
		puts("socket 发生错误");
		return false;
	}
	if (!ret)
	{
		puts("请求超时");
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
	printf("来自 %s 的回复：字节=%d 时间=%d TTL=%d \n",
		inet_ntoa(destAddr.sin_addr),
		sizeof(ICMPReq::data),
		GetTickCount() - sendtime,
		abs((int)TTL)
	);

	return true;
}


void ping(char* destIP)
{
	SOCKET raw_socket;//创建原始套接字
	sockaddr_in destAddr;//目标主机地址
	sockaddr_in scrAddr;//应答报文中的源地址
	int ret;
	int i;
	int sendNum,recvNum;//ICMP报文发送个数、接收个数

	//生成套接字
	//TCP/IP协议簇，RAW模式，ICMP模式
	//RAW创建的是一个原始套接字，最低可以访问到数据链路层的数据，也就是说在网络层的IP头的数据也可以拿到了。
	raw_socket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

	destAddr.sin_family = AF_INET;
	destAddr.sin_port = htons(0);
	destAddr.sin_addr.S_un.S_addr = inet_addr(destIP);

	sendNum = 0;
	recvNum = 0;
	printf("正在Ping %s 具有%d字节的数据\n",inet_ntoa(destAddr.sin_addr),sizeof(ICMPReq::data));
	for (i = 0; i < PINGNUM; i++)
	{
		sendNum++;
		if (doPing(raw_socket, destAddr, scrAddr, i))
		{
			recvNum++;
		}
		
	}
	printf("%s 的Ping统计信息\n", inet_ntoa(destAddr.sin_addr));
	printf("\t数据包：已发送 = %d,已接收 = %d,丢失 = %d(%d%%丢失)\n", sendNum, recvNum, sendNum - recvNum,(sendNum - recvNum)*100/sendNum);

	ret = closesocket(raw_socket);

	if (ret == SOCKET_ERROR)
	{
		puts("关闭socket发生错误");
		return ;
	}
	
}