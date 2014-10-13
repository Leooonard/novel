/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008-2009 Strasbourg University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Sebastien Vincent <vincent@clarinet.u-strasbg.fr>
 */

// Network topology
//
//       n0    n1
//       |     |
//       =================
//              LAN
//
// - ICMPv4 echo request flows from n0 to n1 and back with ICMPv4 echo reply
// - DropTail queues 
// - Tracing of queues and packet receptions to file "ping4.tr"
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "libxml/parser.h"
#include "string"
#include "cstdlib"
#include "arpa/inet.h"
#include "time.h"
extern "C" {
using namespace std;

using namespace ns3;
typedef unsigned char xmlChar;
extern char* inet_ntoa(struct in_addr);
void assignAddr(Ipv4InterfaceContainer&ic, const Ptr<NetDevice>& dev,
		const Ipv4Address &add, const Ipv4Mask &ma) {
	uint32_t address = add.Get();
	uint32_t mask = ma.Get();
	Ptr<Node> node = dev->GetNode();
	Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
	int32_t interface = ipv4->GetInterfaceForDevice(dev);
	if (interface == -1) {
		interface = ipv4->AddInterface(dev);
	}
	Ipv4Address addr(address);
	Ipv4AddressGenerator::AddAllocated(addr);
	Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress(addr, mask);
	ipv4->AddAddress(interface, ipv4Addr);
	ipv4->SetMetric(interface, 1);
	ipv4->SetUp(interface);
	ic.Add(ipv4, interface);
}
void SetDefaultRouter(const Ipv4InterfaceContainer& ic, uint32_t i,
		bool router) {
	std::pair<Ptr<Ipv4>, uint32_t> m = ic.Get(i);
	Ptr<Ipv4> ipv4 = m.first;
	ipv4->SetForwarding(m.second, router);

	if (router) {
		uint32_t other;
		/* assume first global address is index 1 (0 is link-local) */
		Ipv4Address routerAddress = ic.GetAddress(i);

		for (other = 0; other < ic.GetN(); other++) {
			if (other != i) {
				Ptr<Ipv4StaticRouting> routing = 0;
				Ipv4StaticRoutingHelper routingHelper;

				std::pair<Ptr<Ipv4>, uint32_t> m1 = ic.Get(other);
				ipv4 = m1.first;
				routing = routingHelper.GetStaticRouting(ipv4);
				routing->SetDefaultRoute(routerAddress, m1.second);
			}
		}
	}

}
void SetRouter(const Ipv4InterfaceContainer& ic, uint32_t i, bool router,
		const Ipv4Address& dest, const Ipv4Mask &mask,
		const Ipv4Address& routerAddress) {
	std::pair<Ptr<Ipv4>, uint32_t> m = ic.Get(i);
	Ptr<Ipv4> ipv4 = m.first;
	ipv4->SetForwarding(m.second, router);

	if (router) {
		uint32_t other;
		/* assume first global address is index 1 (0 is link-local) */
		//Ipv4Address routerAddress = ic.GetAddress(i);
		for (other = 0; other < ic.GetN(); other++) {
			if (other != i) {
				Ptr<Ipv4StaticRouting> routing = 0;
				Ipv4StaticRoutingHelper routingHelper;

				std::pair<Ptr<Ipv4>, uint32_t> m1 = ic.Get(other);
				ipv4 = m1.first;
				routing = routingHelper.GetStaticRouting(ipv4);
				routing->AddNetworkRouteTo(dest, mask, routerAddress,
						m1.second);
			}
		}
	}

}
//NS_LOG_COMPONENT_DEFINE("Ping6Example");

const char* ping(char* name) {

    int localIndex= 0;
    int remoteIndex= 0;
    NodeContainer testNc;
	string pcapname;
	pcapname.append(name);
	string filename;
	filename.append(name);
	filename.append(".xml");
	string trname;
	trname.append(name);
	trname.append(".tr");
	string logname;
	logname.append(name);
	logname.append(".log");
    //ofstream cout;
	//cout.open(logname.c_str(), ios::out);

	xmlDocPtr doc;
	xmlNodePtr segNode; //缂冩垶顔?
	xmlNodePtr eleNode1; //閼哄倻鍋?
	xmlNodePtr eleNode2; //閼哄倻鍋?
	xmlNodePtr curNode; //瑜版挸澧犻懞鍌滃仯

	xmlChar *szKey;
	const char *szDocName = filename.c_str();
	//cout<<filename<<endl;
	//cout << "瀵偓婵顕伴崣鏈甅L娣団剝浼? << endl;
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue=1;
	doc = xmlReadFile(szDocName, "gb2312", XML_PARSE_NOBLANKS);
	if (NULL == doc) {
		return "0"; //绌虹殑xml鏂囨。.
	}
	curNode = xmlDocGetRootElement(doc);

	if (NULL == curNode) {	
		xmlFreeDoc(doc);
		return "1";
	}
	if (xmlStrcmp(curNode->name, BAD_CAST "root")) //閺嶅湱绮ㄩ悙?
	{
		xmlFreeDoc(doc);
		return "2";
	}
	//鐠囪褰囩€圭偤鐛欑猾璇茬€?
	curNode = curNode->xmlChildrenNode; //
	string experitype;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "experitype"))) //鐎圭偤鐛欑猾璇茬€?
	{	
		szKey = xmlNodeGetContent(curNode);
		experitype.append((char*) szKey);
		//cout << "鐎圭偤鐛欑猾璇茬€锋稉鐚寸窗" << experitype << endl<<endl;
		xmlFree(szKey);
	} else {
		return "3";
	}
	//鐠囪褰囩紒鎾跺仯娑擃亝鏆?
	curNode = curNode->next;
	int nodecount;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "nodecount"))) //鐎圭偤鐛欓懞鍌滃仯娑擃亝鏆?
	{
		szKey = xmlNodeGetContent(curNode);
		nodecount = atoi((char*) szKey);
		//cout << "閼哄倻鍋ｆ稉顏呮殶娑撶尨绱? << nodecount << endl<<endl;
		xmlFree(szKey);
	} else {
		return "4";
	}
	if (nodecount <= 0 || nodecount > 100) {   //閸欘亣鍏橀崷?閸?00娑擃亣濡悙纭咁潌閸嬫艾鐤勬?
		return "5";
	}
	//鐠囪褰囩純鎴烆唽娑擃亝鏆?
	curNode = curNode->next;
	int segmentcount;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "segmentcount"))) //鐎圭偤鐛欑純鎴烆唽娑擃亝鏆?
	{
		szKey = xmlNodeGetContent(curNode);
		segmentcount = atoi((char*) szKey);
		//cout << "缂冩垶顔屾稉顏呮殶娑撶?寸窗" << segmentcount << endl<<endl;
		xmlFree(szKey);
	} else {
		return "6";
	}
	cout << "瀵偓婵鍨卞铏圭秹缂佹粏濡悙?\n";
	NodeContainer nodes;                            //閹碘偓閺堝濡悙鐟扮摠閺€鎯ф躬鏉╂瑤閲渃ontainer閸?
	nodes.Create(nodecount);
	cout << "鐎瑰顥婄純鎴犵捕閸楀繗顔匼n";
	InternetStackHelper internetv4;
	internetv4.Install(nodes);
	NodeContainer *nodecontainer = new NodeContainer[segmentcount];                 //娑撯偓娑撶寶ontainer閺佹壆绮? 婢堆冪毈娑撹櫣缍夊▓鍨殶, 濮ｅ繋閲滈崗鍐閺堝琚辨稉顏囧Ν閻?
	if (nodecontainer == NULL) {
		return "7";
	}
	Ipv4InterfaceContainer *ipv4interfaceiontainer =
			new Ipv4InterfaceContainer[segmentcount];                       //鐠佹澘缍嶆禍鍡曠閸ф缍夐崡鈥茬瑐閻ㄥ嚘P閸︽澘娼冮梿? 鏉╂ɑ妲搁崡鏇犲閻ㄥ埇p閸︽澘娼?
	if (ipv4interfaceiontainer == NULL) {
		return "7";
	}
	NetDeviceContainer *netdevicecontainer =
			new NetDeviceContainer[segmentcount];                           //缂冩垵宕遍梿鍡楁値, 濮ｅ繋閲滅拋鎯ь槵閸欘垯浜掗張澶婎樋閸ф缍夐崡? 楠炴湹绗栨导姘樋閸戣桨绔撮崸妞剧瑝閸欘垳鏁ょ純鎴濆幢. 娑撳鐖ｆ稉?.
	if (ipv4interfaceiontainer == NULL) {
		return "7";
	}
	int *segmentid=new int[segmentcount];
	for(int i=0;i<segmentcount;i++)
	{
		segmentid[i]= -1;
	}
	int *nodeid=new int[nodecount];                                                 //鐠佹澘缍嶆禍鍡楊嚠鎼存柧缍呯純鐣乷de閺佹壆绮嶉惃鍕啎婢跺洨娈慖D閸? 閺堫剛鈻兼惔蹇撳敶ID閸欒渹璐焠ode閺佹壆绮嶆稉瀣垼
	for(int i=0;i<nodecount;i++)
	{
		nodeid[i]= -1;                           
	}
	//cout << "瀵偓婵鍨卞杞颁繆闁? << endl<<endl;
	CsmaHelper csma; //娣囷繝浜?
	csma.SetDeviceAttribute("Mtu", UintegerValue(1500));
	csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

	int localID;                                                            //鐠佹澘缍嶉張顒€婀寸拋鎯ь槵娣団剝浼?
	string localipaddress = "null";
	string localmask;
	int remoteID;                                                           //鐠佹澘缍嶆潻婊呪柤鐠佹儳顦穱鈩冧紖
	string remoteipaddress = "null";
	string remotemask;
	curNode = curNode->next;


//--------------------------閼惧嘲褰囬張顒€婀存稉缁樻簚閸滃矁绻欑粙瀣╁瘜閺堣櫣娈慖D閸?-------------------------------
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "local"))) //閺堫剙婀存稉缁樻簚
	{
                //cout<< curNode->name<< endl;
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			localID = atoi((char*) szAttr);
			//cout << "鐠囪褰囧┃鎰Ν閻愮D閿? << localID << endl;
			xmlFree(szAttr);
		}
	}else if((!xmlStrcmp(curNode->name, (const xmlChar *) "remote"))){
                xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			remoteID = atoi((char*) szAttr);
			//cout << "鐠囪褰囬惄顔界垼閼哄倻鍋D閿? << remoteID << endl<<endl;
                        //cout << "鐠囪褰囬惄顔界垼閼哄倻鍋D閿? << remoteID << endl<<endl;
			xmlFree(szAttr);
		}
        }
	//閻╊喗鐖ｆ稉缁樻簚
	curNode = curNode->next;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "remote"))) //閻╊喗鐖ｆ稉缁樻簚
	{
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			remoteID = atoi((char*) szAttr);
			//cout << "鐠囪褰囬惄顔界垼閼哄倻鍋D閿? << remoteID << endl<<endl;
                      //  cout << "鐠囪褰囬惄顔界垼閼哄倻鍋D閿? << remoteID << endl<<endl;
			xmlFree(szAttr);
		}
	}else if((!xmlStrcmp(curNode->name, (const xmlChar *) "local"))){
               // cout<< curNode->name<< endl;
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			localID = atoi((char*) szAttr);
			//cout << "鐠囪褰囧┃鎰Ν閻愮D閿? << localID << endl;
                       // cout << "鐠囪褰囧┃鎰Ν閻愮D閿? << localID << endl;
			xmlFree(szAttr);
		}
        }
//------------------------------------------------------------------------------

	curNode = curNode->next;
	segNode = curNode;
 


//---------------------瀵偓婵绻橀崗銉х秹濞堥潧鎯婇悳? 娑撹櫣缍夊▓鍏歌⒈缁旑垳娈戠拋鎯ь槵鏉╂稖顢戦柊宥囩枂-------------------------     
	try
	{   
	while (segNode != NULL
			&& (!xmlStrcmp(segNode->name, (const xmlChar *) "segment"))) {
		int segmentID;
		int segmentID1;
		curNode = segNode;
		if ((!xmlStrcmp(curNode->name, (const xmlChar *) "segment"))) //缂冩垶顔屽顏嗗箚
		{
			xmlNodePtr propSegPtr = curNode;
			xmlAttrPtr attrPtr = propSegPtr->properties;
			if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //缂冩垶顔孖D
					{
				xmlChar* szAttr = xmlGetProp(propSegPtr, BAD_CAST "ID");
				segmentID = atoi((char*) szAttr);
				segmentID1=segmentID;
				//cout << "缂冩垶顔? << segmentID<< "瀵偓婵鍨卞鐚寸窗" << endl;
				for(int i=0;i<segmentcount;i++)
				{
					if(segmentid[i]!=-1)
					{
						if(segmentid[i]==segmentID)
						{
							segmentID=i;
							break;
						}
					}
					else
					{
						segmentid[i]=segmentID;
						segmentID=i;
						break;
					}
				}
				xmlFree(szAttr);
			}
			curNode = curNode->xmlChildrenNode; //node
		}

		eleNode1 = curNode; //閼哄倻鍋?
		eleNode2 = curNode->next; //閼哄倻鍋?
		xmlNodePtr propNodePtr1 = eleNode1;
		xmlNodePtr propNodePtr2 = eleNode2;
		int ID1; //閼哄倻鍋?缂傛牕褰?
		int ID2; //閼哄倻鍋?缂傛牕褰?
		int ID11; //閼哄倻鍋?缂傛牕褰?
		int ID22; //閼哄倻鍋?缂傛牕褰?
		if ((!xmlStrcmp(eleNode1->name, (const xmlChar *) "node"))
				&& (!xmlStrcmp(eleNode2->name, (const xmlChar *) "node"))) //娑撱倓閲滈懞?倻??
				{
			xmlAttrPtr attrPtr1 = propNodePtr1->properties;
			xmlAttrPtr attrPtr2 = propNodePtr2->properties;
			xmlChar* szAttr1;
			xmlChar* szAttr2;
			Ptr<Node> n1;
			Ptr<Node> n2;
			if (!xmlStrcmp(attrPtr1->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯1ID
					{
				szAttr1 = xmlGetProp(propNodePtr1, BAD_CAST "ID");
				ID1 = atoi((char*) szAttr1);
				ID11=ID1;
                                if(ID1== remoteID){
                                        remoteIndex= ID1;
                                }else if(ID1== localID){
                                        localIndex= ID1;
                                }
				/*for(int i=0;i<nodecount;i++)
				{
					if(nodeid[i]!=-1)
					{
						if(nodeid[i]==ID1)
						{
							ID1=i;
							break;
						}
					}
					else
					{
						nodeid[i]=ID1;                          //绾喖鐣綢D閸?
                                                if(ID1== remoteID){
                                                        remoteIndex= i;
                                                }else if(ID1== localID){
                                                        localIndex= i;
                                                }
                                                cout<< ID1+ " \n"; 
						ID1=i;                                  //ID1閸欘垯浜掔涵顔肩暰閼哄倻鍋ｉ崷鈺猳de閺佹壆绮嶆稉顓犳畱娴ｅ秶鐤?
						break;
					}
				}*/
				xmlFree(szAttr1);
				n1 = nodes.Get(ID1);                                    //閼惧嘲绶遍懞鍌滃仯1
			}
			attrPtr1 = attrPtr1->next;

			if (!xmlStrcmp(attrPtr2->name, BAD_CAST "ID")) //閼惧嘲绶遍懞鍌滃仯2ID
					{
				szAttr2 = xmlGetProp(propNodePtr2, BAD_CAST "ID");
				ID2 = atoi((char*) szAttr2);
				ID22=ID2;
                                if(ID2== remoteID){
                                         remoteIndex= ID2;
                                }else if(ID2== localID){
                                         localIndex= ID2;
                                }
				/*for(int i=0;i<nodecount;i++)
				{
					if(nodeid[i]!=-1)
					{
						if(nodeid[i]==ID2)
						{
							ID2=i;
							break;
						}
					}
					else
					{
						nodeid[i]=ID2;
                                                if(ID2== remoteID){
                                                        remoteIndex= i;
                                                }else if(ID2== localID){
                                                        localIndex= i;
                                                }
                                                cout<< "2\n";
                                                //cout<< ID2<< endl;
						ID2=i;
						break;
					}
				}*/
				xmlFree(szAttr2);
				n2 = nodes.Get(ID2); //閼惧嘲绶遍懞鍌滃仯2
			}
			attrPtr2 = attrPtr2->next;

			NodeContainer nc = nodecontainer[segmentID]; //閼哄倻鍋ｇ€圭懓娅?
			Ipv4InterfaceContainer iic = ipv4interfaceiontainer[segmentID];         //iic閺勵垵绻栨稉顏嗙秹濞堝吀鑵戞稉銈勯嚋缁旑垳鍋ｇ拋鎯ь槵閻ㄥ嚘P閹恒儱褰涢惃鍕啇閸?
			nc.Add(n1);
			nc.Add(n2);

			NetDeviceContainer nd = netdevicecontainer[segmentID];
			nd = csma.Install(nc); //缂冩垹绮剁拋鎯ь槵

			Ptr<NetDevice> dev1 = nd.Get(0);
			Ptr<NetDevice> dev2 = nd.Get(1);

			string ipaddress1;
			string mask1;
			string dest1;
			string ma1;
			string nexthop1;
			string ipaddress2;
			string mask2;
			string dest2;
			string ma2;
			string nexthop2;
			xmlNodePtr routertable1;
			xmlNodePtr routertable2;
			int nodetype1;
			int nodetype2;
			//閼哄倻鍋?
			if (!xmlStrcmp(attrPtr1->name, BAD_CAST "type")) {

				szAttr1 = xmlGetProp(propNodePtr1, BAD_CAST "type");
				if ((!xmlStrcmp(szAttr1, (const xmlChar *) "host"))) //娑撶粯婧€
				{
					////////////////////////////////////////////////////////
					//鐠佸墽鐤咺P閸滃ask
                                        //cout<< "host node"<< nc.Get(0)->GetId()<< " register device!"<< endl;
					//cout << "娑撶粯婧€" << ID11 << "瀵偓婵鍨卞鐚寸窗" << endl;                //閻滄澘婀狪D11閹靛秵妲搁惇鐔割劀閻ㄥ嫯顔曟径鍥╂畱ID閸?
					nodetype1 = 1;                                  //noudetype娑?閺勵垯瀵岄張? 娑?閺勵垵鐭鹃悽?
					curNode = curNode->xmlChildrenNode;			//IP
					if ((!xmlStrcmp(curNode->name,
							(const xmlChar *) "ipaddress"))) {
						szKey = xmlNodeGetContent(curNode);
						ipaddress1.append((char*) szKey);
						xmlFree(szKey);
					}
					curNode = curNode->next;
					if ((!xmlStrcmp(curNode->name, (const xmlChar *) "mask"))) {
						szKey = xmlNodeGetContent(curNode);
						mask1.append((char*) szKey);
						xmlFree(szKey);
					}
					string temp = ipaddress1.substr(0, 4);
					if (temp != "null") {
						in_addr in;
						in_addr ma;
						unsigned long int ip = inet_addr(ipaddress1.c_str());
						unsigned long int m = inet_addr(mask1.c_str());
						in.s_addr = ip;
						ma.s_addr = m;
						ipaddress1.clear();	
						ipaddress1.assign(inet_ntoa(in));
						mask1.clear();			
						mask1.assign(inet_ntoa(ma));
						//cout << "娑撶粯婧€" << ID11 << "閸掑棝鍘P閸︽澘娼冮敍? << ipaddress1 << "  "
								//<< mask1 << endl;
		
						assignAddr(iic, dev1, ipaddress1.c_str(),
								mask1.c_str());			//鐠佸墽鐤?
					}
				}
				if ((!xmlStrcmp(szAttr1, (const xmlChar *) "router")))	//鐠侯垳鏁遍崳?
				{
					//cout << "鐠侯垳鏁遍崳? << ID11 << "瀵偓婵鍨卞鐚寸窗" << endl;
                                        //cout<< "route node"<< nc.Get(0)->GetId()<< " register device!"<< endl;
					nodetype1 = 2;
					//鐠佸墽鐤咺P閿涘ask閸滃矁鐭鹃悽杈€?
					curNode = curNode->xmlChildrenNode;			//IP
					if ((!xmlStrcmp(curNode->name,
							(const xmlChar *) "ipaddress"))) {
						szKey = xmlNodeGetContent(curNode);
						ipaddress1.append((char*) szKey);
						xmlFree(szKey);
					}
					curNode = curNode->next;
					if ((!xmlStrcmp(curNode->name, (const xmlChar *) "mask"))) {
						szKey = xmlNodeGetContent(curNode);
						mask1.append((char*) szKey);
						xmlFree(szKey);
					}
					string temp = ipaddress1.substr(0, 4);
					if (temp != "null") {
						in_addr in;
						in_addr ma;
						unsigned long int ip = inet_addr(ipaddress1.c_str());
						unsigned long int m = inet_addr(mask1.c_str());
						in.s_addr = ip;
						ma.s_addr = m;
						ipaddress1.clear();
						mask1.clear();
						mask1.assign(inet_ntoa(ma));
						ipaddress1.assign(inet_ntoa(in));
						//cout << "鐠侯垳鏁遍崳? << ID11 << "閸掑棝鍘P閸︽澘娼冮敍? << ipaddress1 << "  "
								//<< mask1 << endl;
						assignAddr(iic, dev1, ipaddress1.c_str(),
								mask1.c_str());			//鐠佸墽鐤?

						curNode = curNode->next;
						if ((!xmlStrcmp(curNode->name,
								(const xmlChar *) "routertable"))) {
							routertable1 = curNode;
						}
					}

				}
				xmlFree(szAttr1);
			}

			//閼哄倻鍋?
			curNode = eleNode2;
			if (!xmlStrcmp(attrPtr2->name, BAD_CAST "type")) {
				szAttr2 = xmlGetProp(propNodePtr2, BAD_CAST "type");
				if ((!xmlStrcmp(szAttr2, (const xmlChar *) "host")))	//娑撶粯婧€
				{
					//cout << "娑撶粯婧€" << ID22 << "瀵偓婵鍨卞鐚寸窗" << endl;
                                        //cout<< "host node"<< nc.Get(1)->GetId()<< " register device!"<< endl;
					nodetype2 = 1;
					////////////////////////////////////////////////////////
					//鐠佸墽鐤咺P閸滃ask
					curNode = curNode->xmlChildrenNode;			//IP
					if ((!xmlStrcmp(curNode->name,
							(const xmlChar *) "ipaddress"))) {
						szKey = xmlNodeGetContent(curNode);
						ipaddress2.append((char*) szKey);
						xmlFree(szKey);
					}
					curNode = curNode->next;
					if ((!xmlStrcmp(curNode->name, (const xmlChar *) "mask"))) {
						szKey = xmlNodeGetContent(curNode);
						mask2.append((char*) szKey);
						xmlFree(szKey);
					}
					string temp = ipaddress2.substr(0, 4);
					if (temp != "null") {
						in_addr in;
						in_addr ma;
						unsigned long int ip = inet_addr(ipaddress2.c_str());
						unsigned long int m = inet_addr(mask2.c_str());
						in.s_addr = ip;
						ma.s_addr = m;
						ipaddress2.clear();
						ipaddress2.assign(inet_ntoa(in));
						mask2.clear();
						mask2.assign(inet_ntoa(ma));
						//cout << "娑撶粯婧€" << ID22 << "閸掑棝鍘P閸︽澘娼冮敍? << ipaddress2 << "  "
								//<< mask2 << endl;
						assignAddr(iic, dev2, ipaddress2.c_str(),
								mask2.c_str());			//鐠佸墽鐤?
					}
				}
				if ((!xmlStrcmp(szAttr2, (const xmlChar *) "router")))	//鐠侯垳鏁遍崳?
				{
					//cout << "鐠侯垳鏁遍崳? << ID22 << "瀵偓婵鍨卞鐚寸窗" << endl;
                                        //cout<< "route node"<< nc.Get(1)->GetId()<< " register device!"<< endl;
					nodetype2 = 2;
					//鐠佸墽鐤咺P閿涘ask閸滃矁鐭鹃悽杈€?
					curNode = curNode->xmlChildrenNode;			//IP
					if ((!xmlStrcmp(curNode->name,
							(const xmlChar *) "ipaddress"))) {
						szKey = xmlNodeGetContent(curNode);
						ipaddress2.append((char*) szKey);
						xmlFree(szKey);
					}
					curNode = curNode->next;
					if ((!xmlStrcmp(curNode->name, (const xmlChar *) "mask"))) {
						szKey = xmlNodeGetContent(curNode);
						mask2.append((char*) szKey);
						xmlFree(szKey);
					}
					string temp = ipaddress2.substr(0, 4);
					if (temp != "null") {
						in_addr in;
						in_addr ma;
						unsigned long int ip = inet_addr(ipaddress2.c_str());
						unsigned long int m = inet_addr(mask2.c_str());
						in.s_addr = ip;
						ma.s_addr = m;
						ipaddress2.clear();
						ipaddress2.assign(inet_ntoa(in));
						mask2.clear();
						mask2.assign(inet_ntoa(ma));
						//cout << "鐠侯垳鏁遍崳? << ID22 << "閸掑棝鍘P閸︽澘娼冮敍? << ipaddress2 << "  "
								//<< mask2 << endl;
							cout<< "arrive"<<endl;
						assignAddr(iic, dev2, ipaddress2.c_str(),
								mask2.c_str());			//鐠佸墽鐤?
						curNode = curNode->next;
						if ((!xmlStrcmp(curNode->name,
								(const xmlChar *) "routertable"))) {
							routertable2 = curNode;
						}
					}

				}
				xmlFree(szAttr2);
			}
			///
			int setnumber1 = 0;
			int setnumber2 = 0;
			string temp1 = ipaddress1.substr(0, 4);
			string temp2 = ipaddress2.substr(0, 4);
			if (temp1 != "null" && temp2 != "null") {
				if (nodetype1 == 2) {                                   //婵″倹鐏夌拋鎯ь槵1閺勵垵鐭鹃悽鍗炴珤, 鏉╂稑鍙咺F
					curNode = routertable1->xmlChildrenNode;
					if((curNode!= NULL)&&(!xmlStrcmp(curNode->name, (const xmlChar *) "item"))) {
						//cout << "鐠侯垳鏁遍崳? << ID11 << "闁板秶鐤嗙捄顖滄暠鐞涱煉绱? << endl;
						xmlNodePtr itemnode = curNode;
						while (itemnode != NULL) {
							dest1.clear();                          //鐠囪褰囩捄顖滄暠鐞涖劑銆嶆稉顓犳畱IP閸︽澘娼?
							curNode = itemnode->xmlChildrenNode;
							szKey = xmlNodeGetContent(curNode);
							dest1.append((char*) szKey);
							xmlFree(szKey);

							string temp;
							temp = dest1.substr(0, 4);
							if (temp != "null") {
								ma1.clear();
								curNode = curNode->next;                       //鐠囪褰囩捄顖滄暠鐞涖劑銆嶆稉顓犳畱鐎涙劗缍夐幒鈺冪垳
								szKey = xmlNodeGetContent(curNode);
								ma1.append((char*) szKey);
								xmlFree(szKey);

								nexthop1.clear();
								curNode = curNode->next;		//鐠囪褰囩捄顖滄暠鐞涖劑銆嶆稉顓犳畱娑撳绔寸捄?
								szKey = xmlNodeGetContent(curNode);
								nexthop1.append((char*) szKey);
								xmlFree(szKey);

								temp = nexthop1.substr(0, 4);
                                                               // cout<< "temp:"<< temp<< endl;
								if (temp != "null") {
									in_addr d1;
									in_addr d2;
									in_addr m1;
									unsigned long int i2 = inet_addr(
											nexthop1.c_str());
									unsigned long int i1 = inet_addr(
											ipaddress2.c_str());
									if (i1 == i2) {
										unsigned long int i3 = inet_addr(
												dest1.c_str());
										unsigned long int m = inet_addr(
												ma1.c_str());
										d1.s_addr = i3;
										d2.s_addr = i2;
										m1.s_addr = m;
										dest1.clear();
										dest1.assign(inet_ntoa(d1));
										ma1.clear();
										ma1.assign(inet_ntoa(m1));
										nexthop1.clear();
										nexthop1.assign(inet_ntoa(d2));
										setnumber1 = 1;		//鐠侯垳鏁遍崳?瀹歌尪顔曠純顕€娼ら幀浣界熅閻㈣精銆?
										SetRouter(iic, setnumber1, true,
												dest1.c_str(), ma1.c_str(),
												nexthop1.c_str());
										//cout << dest1 << "  " << ma1 << "  "
												//<< nexthop1 << endl;
									}
								}
							}
							itemnode = itemnode->next;
						}
					}
				}
				if (nodetype2 == 2) {
					int tag = -1;			//鐠佹澘缍嶇粭顑跨癌娑擃亣鐭鹃悽鍗炴珤閺勵垰鎯佺拋鍓х枂闂堟瑦鈧浇鐭鹃悽?
					curNode = routertable2->xmlChildrenNode;		//item
					if ((curNode!= NULL)&& (!xmlStrcmp(curNode->name, (const xmlChar *) "item"))) {
						//cout << "鐠侯垳鏁遍崳? << ID22 << "闁板秶鐤嗙捄顖滄暠鐞涱煉绱? << endl;
						xmlNodePtr itemnode = curNode;
						while (itemnode != NULL) {
							dest2.clear();
							curNode = itemnode->xmlChildrenNode;
							szKey = xmlNodeGetContent(curNode);
							dest2.append((char*) szKey);
							xmlFree(szKey);

							string temp;
							temp = dest2.substr(0, 4);
							if (temp != "null") {
								ma2.clear();
								curNode = curNode->next;
								szKey = xmlNodeGetContent(curNode);
								ma2.append((char*) szKey);
								xmlFree(szKey);

								nexthop2.clear();
								curNode = curNode->next;		//nexthop
								szKey = xmlNodeGetContent(curNode);
								nexthop2.append((char*) szKey);
								xmlFree(szKey);

								temp = nexthop2.substr(0, 4);
                                                               // cout<< "temp:"<< temp<< endl;
								if (temp != "null") {
									in_addr d1;
									in_addr d2;
									in_addr m1;
									unsigned long int i2 = inet_addr(
											nexthop2.c_str());
									unsigned long int i1 = inet_addr(
											ipaddress1.c_str());
									if (i1 == i2) {
										unsigned long int i3 = inet_addr(
												dest2.c_str());
										unsigned long int m = inet_addr(
												ma2.c_str());
										d1.s_addr = i3;
										d2.s_addr = i2;
										m1.s_addr = m;
										dest2.clear();
										dest2.assign(inet_ntoa(d1));
										ma2.clear();
										ma2.assign(inet_ntoa(m1));
										nexthop2.clear();
										nexthop2.assign(inet_ntoa(d2));
										if (nodetype1 == 2) {
											if (setnumber1 == 1) {
												setnumber2 = 0;

												SetRouter(iic, setnumber2, true,
														dest2.c_str(),
														ma2.c_str(),
														nexthop2.c_str());
												//cout << dest2 << "  " << ma2
														//<< "  " << nexthop2
														//<< endl;
												tag = 1;
											} else {
												setnumber1 = 1;
												setnumber2 = 0;
												/*SetDefaultRouter(iic,
														setnumber1, true);*/
												SetRouter(iic, setnumber2, true,
														dest2.c_str(),
														ma2.c_str(),
														nexthop2.c_str());
												//cout << dest2 << "  " << ma2
														//<< "  " << nexthop2
														//<< endl;
												tag = 1;
											}
										} else {
											setnumber2 = 0;
											SetRouter(iic, setnumber2, true,
													dest2.c_str(), ma2.c_str(),
													nexthop2.c_str());
											//cout << dest2 << "  " << ma2 << "  "
													//<< nexthop2 << endl;
											tag = 1;
										}

									}
								}
							}

							itemnode = itemnode->next;
						}
						if (tag != 1) {

							if (setnumber1 == 1) {
								setnumber2 = 0;
								SetDefaultRouter(iic, setnumber2, true);
							} else {
								setnumber2 = 1;
								if (nodetype1 == 2) {
									SetDefaultRouter(iic, setnumber1, true);
								}
								SetDefaultRouter(iic, setnumber2, true);
							}
						}
					}
				} else {
					if (nodetype1 == 2) {
						setnumber1 = 0;
						SetDefaultRouter(iic, setnumber1, true);
					}
				}
			}
			string temp;
			if (ID11 == localID) {
				temp = ipaddress1.substr(0, 4);
				if (temp != "null") {
					localipaddress = ipaddress1;
					localmask = mask1;
					//cout<<"鐠囪褰囧┃鎰Ν閻?<<localID<<"IP閸︽澘娼冮敍?<<localipaddress<<"  "<<localmask<<endl;
				}
			}
			if (ID22 == localID) {
				temp = ipaddress2.substr(0, 4);
				if (temp != "null") {
					localipaddress = ipaddress2;
					localmask = mask2;
					//cout<<"鐠囪褰囧┃鎰Ν閻?<<localID<<"IP閸︽澘娼冮敍?<<localipaddress<<"  "<<localmask<<endl;
				}
			}
			if (ID11 == remoteID) {
				temp = ipaddress1.substr(0, 4);
				if (temp != "null") {
					remoteipaddress = ipaddress1;
					remotemask = mask1;
					//cout<<"鐠囪褰囬惄顔界垼閼哄倻鍋?<<remoteID<<"IP閸︽澘娼冮敍?<<remoteipaddress<<"  "<<remotemask<<endl;
				}
			}
			if (ID22 == remoteID) {
				temp = ipaddress2.substr(0, 4);
				if (temp != "null") {
					remoteipaddress = ipaddress2;
					remotemask = mask2;
					//cout<<"鐠囪褰囬惄顔界垼閼???<<remoteID<<"IP閸︽澘娼冮敍?<<remoteipaddress<<"  "<<remotemask<<endl;
				}
			}
		} else {
			//return "-4";
		}
		//cout << "缂冩垶顔? << segmentID1 << "閸掓稑缂撶紒鎾存将閵? << endl << endl;
		segNode = segNode->next;			//娑撳绔存稉顏嗙秹濞?
					cout<< "seg end"<< endl;
	}
	}catch(exception& e)
	{
		return "1";
	}catch(int i)
	{
		return "1";
	}catch(char* c)
	{
		return "1";
	}

	xmlFreeDoc(doc);
//閹稿鍙庣€圭偤鐛欑猾璇茬€烽崚娑樼紦鎼存梻鏁?
	if (localipaddress != "null" && remoteipaddress != "null") {
		uint32_t packetSize = 1024;
		Time interPacketInterval = Seconds(1.0);
		//cout << "瀵偓婵婀懞鍌滃仯娑撳﹤鍨卞鍝勭安閻㈩煉绱? << endl;
		unsigned long int ip = inet_addr(remoteipaddress.c_str());
		in_addr in;
		in.s_addr = ip;
		const char* p = inet_ntoa(in);
		V4PingHelper ping4 = V4PingHelper(Ipv4Address(p));
		ping4.SetAttribute("Verbose", BooleanValue(bool("True")));
		ping4.SetAttribute("Interval", TimeValue(interPacketInterval));
		ping4.SetAttribute("Size", UintegerValue(packetSize));
		ApplicationContainer apps = ping4.Install(nodes.Get(localIndex));
		apps.Start(Seconds(1.0));
		apps.Stop(Seconds(3.0));
	}
	//cout << "鐎规矮绠熼崠鍛繆閹? << endl;
	AsciiTraceHelper ascii;
       // cout << "A" << endl;
 	csma.EnableAsciiAll(ascii.CreateFileStream(trname));
        //cout << "B" << endl;
	//csma.EnablePcapAll("target", true);
        for(int i= 0; i< nodecount; i++){
                //cout<< "node"<< i<< " has "<< nodes.Get(i)->GetNDevices()<< " devices"<< " and node id is "<< nodes.Get(i)->GetId()<< endl;
                if(nodes.Get(i)->GetId()== remoteIndex){
                        //cout<< "node"<< i<< " is remote node"<< endl;
                }
                if(nodes.Get(i)->GetId()== localIndex){
                        //cout<< "node"<< i<< " is local node"<< endl;
                }
        }

        string date;
        time_t rawTime;
        struct tm * timeinfo;
        time(&rawTime);
        timeinfo= localtime(&rawTime);  
        srand((int)time(0));
        int randVal= rand()+ timeinfo->tm_year+ timeinfo->tm_mon+ timeinfo->tm_mday+ timeinfo->tm_hour+ timeinfo->tm_min+ timeinfo->tm_sec;
        string randStr= "";
        //sprintf(string, "%d", number); 
        char c[256];
        sprintf(c, "%d", randVal);
        randStr= c;
        string localIndexStr;
        sprintf(c, "%d", localIndex);
        localIndexStr= c;
        csma.EnablePcapAll(pcapname+ randStr, true);
        //csma.EnablePcap("/home/mymmoondt/mysite/mysite/"+ randStr, nodes.Get(localIndex)->GetId(), 1, true);
        //csma.EnablePcap("target", nodes.Get(2)->GetId(), 1, true);
        //csma.EnablePcap("target", nodes.Get(2)->GetId(), 2, true);
        //randStr= randStr+ '-'+ localIndexStr+ "-1.pcap";     
        randStr= pcapname+ randStr;
        for(int i= 0; i< nodecount; i++){
                char c[20];
                string count;
                sprintf(c, "%d", nodes.Get(i)->GetNDevices());
                count= c;
                randStr= randStr+ '|'+ count;
        }
        randStr= randStr+ '|';

//NS_LOG_INFO ("Run Simulation.");
	//cout << "濡剝瀚欑€圭偤鐛欏鈧慨瀣箥鐞涘矉绱? << endl;
	Simulator::Run();
	//cout << "濡剝瀚欑€圭偤鐛欐潻鎰攽缂佹挻娼敍? << endl;
	Simulator::Destroy();
	//cout << "闁插﹥鏂佺挧鍕爱" << endl;



	delete[] nodecontainer;
	delete[] ipv4interfaceiontainer;
	delete[] netdevicecontainer;
	delete[] nodeid;
	delete[] segmentid;
    cout<< randStr;
    return randStr.c_str();

}

int main(int argc, char **argv) {
	return 0;

}
}

