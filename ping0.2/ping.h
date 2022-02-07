#ifndef PING_H_
#define  PING_H_

#include<stdio.h>
#include<string.h>
#include<cstdint>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

//IP头部
struct IPhead
{	//这里使用位域，例如version在内存占4个字节而不是8个字节
	uint8_t version : 4;//4位版本号
	uint8_t headLen : 4;//4位头部长度
	uint8_t servertype;//8位服务类型
	uint16_t totalLen;//16位总长度
	uint16_t ident;//16位标识
	uint16_t flag:3;//3位标志
	uint16_t fragmentoffset:13;//13位片偏移
	uint8_t TTL;//8位生存时间
	uint8_t protocol;//8位协议
	uint16_t headcheaksum;//16位头部校验和
	uint32_t srcIP;//32位源ip地址
	uint32_t destIP;//32位目标ip地址
};

//ICMP头部
struct ICMPhead
{
	uint8_t type;//8位类型
	uint8_t code;//8位代码
	uint16_t cheaksum;//16位校验和
	uint16_t ident;//16位标识符
	uint16_t seq;//16位序列号
};

//ICMP请求报文（发送用）
struct ICMPrequest
{
	ICMPhead icmphead;//ICMP头部
	uint32_t timeStamp;//时间戳
	char data[32];//数据
};

//ICMP回应报文（接收用）
struct ICMPAnwser
{
	struct IPhead iphead;//IP头部
	struct ICMPrequest icmp;//ICMP报文
	char data[1024];//应答报文数据缓冲区
};


bool ping(char* destIP);

#endif // !PING_H_

