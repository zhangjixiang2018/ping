#ifndef PING_H_
#define PING_H_

#include<stdio.h>
#include <cstdint>
#include<cmath>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")


//IP头
struct IPhead
{
	//这里使用了C语言的位域，也就是说像version变量它在内存中占用4bit，而不是8bit
	uint8_t version : 4;//4位版本长度
	uint8_t headLength : 4;//4位头部长度
	uint8_t serverce;//8位服务类型
	uint16_t totalLength;//16位总长度
	uint16_t flagbit;//16位标识
	uint16_t flag : 3;//3位标志
	uint16_t fragmentOffset : 13;//13位片偏移
	char timetoLive;//8位生存时间
	uint8_t protocol;//8位协议
	uint16_t headCheckSum;//16位头部校验和
	uint32_t srcIP;//32位源端IP
	uint32_t dstIP;//32位目的端IP
};

//ICMP 
struct ICMPhead
{
	uint8_t type;//8位类型
	uint8_t code;//8位代码
	uint16_t checkSum;//16位校验和
	uint16_t ident;//16位标识符
	uint16_t seqNum;//16位序号
};

//ICMP回显请求报文（发送用）
struct ICMPReq
{
	ICMPhead icmphead;//
	uint32_t timeStamp;//时间戳
	char data[32];//
};
//ICMP应答报文（接收用）
struct ICMPAns
{
	IPhead iphead;//IP头
	ICMPReq icmpanswer;//ICMP报文
	char data[1024];//应答报文携带的数据缓冲区
};

void ping(char* destIP);


#endif // !PING_H_

