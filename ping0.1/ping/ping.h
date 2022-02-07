#ifndef PING_H_
#define PING_H_

#include<stdio.h>
#include <cstdint>
#include<cmath>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")


//IPͷ
struct IPhead
{
	//����ʹ����C���Ե�λ��Ҳ����˵��version���������ڴ���ռ��4bit��������8bit
	uint8_t version : 4;//4λ�汾����
	uint8_t headLength : 4;//4λͷ������
	uint8_t serverce;//8λ��������
	uint16_t totalLength;//16λ�ܳ���
	uint16_t flagbit;//16λ��ʶ
	uint16_t flag : 3;//3λ��־
	uint16_t fragmentOffset : 13;//13λƬƫ��
	char timetoLive;//8λ����ʱ��
	uint8_t protocol;//8λЭ��
	uint16_t headCheckSum;//16λͷ��У���
	uint32_t srcIP;//32λԴ��IP
	uint32_t dstIP;//32λĿ�Ķ�IP
};

//ICMP 
struct ICMPhead
{
	uint8_t type;//8λ����
	uint8_t code;//8λ����
	uint16_t checkSum;//16λУ���
	uint16_t ident;//16λ��ʶ��
	uint16_t seqNum;//16λ���
};

//ICMP���������ģ������ã�
struct ICMPReq
{
	ICMPhead icmphead;//
	uint32_t timeStamp;//ʱ���
	char data[32];//
};
//ICMPӦ���ģ������ã�
struct ICMPAns
{
	IPhead iphead;//IPͷ
	ICMPReq icmpanswer;//ICMP����
	char data[1024];//Ӧ����Я�������ݻ�����
};

void ping(char* destIP);


#endif // !PING_H_

