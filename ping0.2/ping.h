#ifndef PING_H_
#define  PING_H_

#include<stdio.h>
#include<string.h>
#include<cstdint>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")

//IPͷ��
struct IPhead
{	//����ʹ��λ������version���ڴ�ռ4���ֽڶ�����8���ֽ�
	uint8_t version : 4;//4λ�汾��
	uint8_t headLen : 4;//4λͷ������
	uint8_t servertype;//8λ��������
	uint16_t totalLen;//16λ�ܳ���
	uint16_t ident;//16λ��ʶ
	uint16_t flag:3;//3λ��־
	uint16_t fragmentoffset:13;//13λƬƫ��
	uint8_t TTL;//8λ����ʱ��
	uint8_t protocol;//8λЭ��
	uint16_t headcheaksum;//16λͷ��У���
	uint32_t srcIP;//32λԴip��ַ
	uint32_t destIP;//32λĿ��ip��ַ
};

//ICMPͷ��
struct ICMPhead
{
	uint8_t type;//8λ����
	uint8_t code;//8λ����
	uint16_t cheaksum;//16λУ���
	uint16_t ident;//16λ��ʶ��
	uint16_t seq;//16λ���к�
};

//ICMP�����ģ������ã�
struct ICMPrequest
{
	ICMPhead icmphead;//ICMPͷ��
	uint32_t timeStamp;//ʱ���
	char data[32];//����
};

//ICMP��Ӧ���ģ������ã�
struct ICMPAnwser
{
	struct IPhead iphead;//IPͷ��
	struct ICMPrequest icmp;//ICMP����
	char data[1024];//Ӧ�������ݻ�����
};


bool ping(char* destIP);

#endif // !PING_H_

