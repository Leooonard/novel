#include<stdio.h>
#include<string.h>
#include<string>
#include<cstring>
#include<stdlib.h>
#include<netinet/in.h>
#include<time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sstream>
#include <iostream>

#define BUFSIZE 10240
#define STRSIZE 1024

extern "C"{

using namespace std;

typedef long bpf_int32;
typedef unsigned long bpf_u_int32;
typedef unsigned short u_short;
typedef unsigned long u_int32;
typedef unsigned short u_int16;
typedef unsigned char u_int8;

/*PCAP结构*/
struct pcap_file_header
{
bpf_u_int32 magic;
u_short version_major;
u_short version_minor;
bpf_int32 thiszone;
bpf_u_int32 sigfigs;
bpf_u_int32 snaplen;
bpf_u_int32 linktype;
};

//时间濯
struct time_val
{
long tv_sec;
long tv_usec;
};

struct pcap_pkthdr
{
struct time_val ts;
bpf_u_int32 caplen;
bpf_u_int32 len;
};

//数据桢头
typedef struct PPPFrameHeader_t
{
/*unsigned char  DstMAC[6];
unsigned char  SrcMAC[6];*/
u_short FrameType;
}PPPFrameHeader_t;

typedef struct CSMAFrameHeader_t
{
	unsigned char DstMAC[6];
	unsigned char SrcMAC[6];
	u_int8 FrameType1;
	u_int8 FrameType2;
}CSMAFrameHeader_t;

typedef struct IPHeader_t
{
u_int8 Ver_HLen;
u_int8 TOS;
u_int16 TotalLen;
u_int16 ID;
u_int16 Flag_Segment;
u_int8 TTL;
u_int8 Protocol;
u_int16 Checksum;
u_int32 SrcIP;
u_int32 DstIP;
}IPHeader_t;

typedef struct ICMPHeader_t
{
	u_int8 type;
	u_int8 code;
	u_int16 checkSum;
	u_int16 id;
	u_int16 seg;
}ICMPHeader_t;

//TCP数据头
typedef struct TCPHeader_t
{
u_int16 SrcPort;
u_int16 DstPort;
u_int32 SeqNO;
u_int32 AckNO;
u_int8 HeaderLen;
u_int8 Flags;
u_int16 Window;
u_int16 Checksum;
u_int16 UrgentPointer;
}TCPHeader_t;

typedef struct Packet{
	u_int32 Sec;
	u_int32 MicSec;
	u_int16 length;
	string protocal;
	string srcIP;
	string dstIP;
	string extraInfo;
}Packet;


const char* checkPCAP(char* path)
{
	string result;
	stringstream StrStream;
	struct pcap_file_header *file_header;
	struct pcap_pkthdr *ptk_header;
	PPPFrameHeader_t *pppframeheader;
	CSMAFrameHeader_t * csmaframeheader;
	IPHeader_t *ip_header;
	ICMPHeader_t *icmp_header;
	TCPHeader_t *tcp_header;
	FILE *fp, *output;

	int pkt_offset,i=0;
	int len;
	int j;
	int ip_len,http_len,ip_proto;
	int src_port,dst_port,tcp_flags;
	char buf[BUFSIZE],my_time[STRSIZE];
	char src_ip[STRSIZE],dst_ip[STRSIZE];
	char host[STRSIZE],uri[BUFSIZE],AcceptLanguage[BUFSIZE],UserAgent[BUFSIZE];
	char Connection[STRSIZE],TransferEncoding[STRSIZE],ContentEncoding[STRSIZE];
	char Contenttype[STRSIZE],CacheControl[STRSIZE],AcceptEncoding[STRSIZE];


	file_header =(struct pcap_file_header *)malloc(sizeof(struct pcap_file_header));
	ptk_header =(struct pcap_pkthdr *)malloc(sizeof(struct pcap_pkthdr));
	pppframeheader=(struct PPPFrameHeader_t *)malloc(sizeof(PPPFrameHeader_t));
	csmaframeheader= (struct CSMAFrameHeader_t *)malloc(sizeof(CSMAFrameHeader_t));
	ip_header=(IPHeader_t *)malloc(sizeof(IPHeader_t));
	icmp_header= (ICMPHeader_t *)malloc(sizeof(ICMPHeader_t));
	tcp_header=(TCPHeader_t *)malloc(sizeof(TCPHeader_t));
	memset(buf,0,sizeof(buf));

	if((fp=fopen(path,"r"))==NULL){
			printf("error: can not open pcap file \n");
			return "open pcap file error";
	}

	if(fread(file_header, sizeof(pcap_file_header), 1, fp)!= 1){
		return "read pcap file header error";
	}

	switch(file_header->linktype){
		case 1:
			
			pkt_offset=24;
			while(fseek(fp,pkt_offset,SEEK_SET)==0){//遍历数据包
				i++;
				if(fread(ptk_header,16,1,fp)!=1){//读取PCAP数据包结构
					break;
				}
				pkt_offset+=16+ptk_header->caplen;  // 下一个数据包的偏移值  循环一次加一个caplen的长度，直接跳到下一个数据包。
			//读取了数据包头之后才能知道caplen的长度,所以上面那句必须在fread的下面。

			  //数据包头16字节
				StrStream<< i<< "|";
				result.append(StrStream.str());
				StrStream.str("");
				StrStream<< ptk_header->caplen<< "|";
				result.append(StrStream.str());
				StrStream.str("");

			//数据桢头14字节
				if(fread(csmaframeheader,sizeof(CSMAFrameHeader_t),1,fp)!=1){
					return "open csma_frame_header error";
				}
				if((csmaframeheader->FrameType1== 8)&&(csmaframeheader->FrameType2== 6)){
					result.append("ARP|");
				}
				if((csmaframeheader->FrameType1== 8)&&(csmaframeheader->FrameType2== 0)){
					result.append("IP|");
					if(fread(ip_header,sizeof(IPHeader_t),1,fp)!=1){
						return "open ip_header error";
					}
					inet_ntop(AF_INET,(void *)&(ip_header->SrcIP),src_ip,16);
					inet_ntop(AF_INET,(void *)&(ip_header->DstIP),dst_ip,16);
					result.append(src_ip);
					result.append("|");
					result.append(dst_ip);
					result.append("|");
					ip_proto=ip_header->Protocol;
					ip_len=ntohs(ip_header->TotalLen);
					switch(ip_proto){
						case 1:
							if(fread(icmp_header, sizeof(ICMPHeader_t), 1, fp)!= 1){
								return "open icmp_header error";
							}
							if((icmp_header->type== 8)&&(icmp_header->code== 0)){
								result.append("ICMP(PING) request|");
							}else{
								if((icmp_header->type== 0)&&(icmp_header->code== 0)){
									result.append("ICMP(PING) reply|");
								}
							}
							break;
						default:
							break;
					}
				}
			}
			break;
		case 9:
			return "not ping pac!";
			break;
		default:
			break;
	}
	fclose(fp);
	result.append("$");
	char * aaa= (char *)malloc(sizeof(char) * (result.length()+ 1));
	for(int i= 0; i< result.length(); i++){
		aaa[i]= result[i];
	}
	aaa[result.length()]= '\0';
	return aaa;
}
}

















