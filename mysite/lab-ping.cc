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
	xmlNodePtr segNode; //缃戞
	xmlNodePtr eleNode1; //鑺傜偣1
	xmlNodePtr eleNode2; //鑺傜偣2
	xmlNodePtr curNode; //褰撳墠鑺傜偣

	xmlChar *szKey;
	const char *szDocName = filename.c_str();
	//cout<<filename<<endl;
	//cout << "寮€濮嬭鍙朮ML淇℃伅" << endl;
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue=1;
	doc = xmlReadFile(szDocName, "gb2312", XML_PARSE_NOBLANKS);
	if (NULL == doc) {
		return "0"; //空的xml文档.
	}
	curNode = xmlDocGetRootElement(doc);

	if (NULL == curNode) {	
		xmlFreeDoc(doc);
		return "1";
	}
	if (xmlStrcmp(curNode->name, BAD_CAST "root")) //鏍圭粨鐐?
	{
		xmlFreeDoc(doc);
		return "2";
	}
	//璇诲彇瀹為獙绫诲瀷
	curNode = curNode->xmlChildrenNode; //
	string experitype;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "experitype"))) //瀹為獙绫诲瀷
	{	
		szKey = xmlNodeGetContent(curNode);
		experitype.append((char*) szKey);
		//cout << "瀹為獙绫诲瀷涓猴細" << experitype << endl<<endl;
		xmlFree(szKey);
	} else {
		return "3";
	}
	//璇诲彇缁撶偣涓暟
	curNode = curNode->next;
	int nodecount;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "nodecount"))) //瀹為獙鑺傜偣涓暟
	{
		szKey = xmlNodeGetContent(curNode);
		nodecount = atoi((char*) szKey);
		//cout << "鑺傜偣涓暟涓猴細" << nodecount << endl<<endl;
		xmlFree(szKey);
	} else {
		return "4";
	}
	if (nodecount <= 0 || nodecount > 100) {   //鍙兘鍦?鍒?00涓妭鐐硅鍋氬疄楠?
		return "5";
	}
	//璇诲彇缃戞涓暟
	curNode = curNode->next;
	int segmentcount;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "segmentcount"))) //瀹為獙缃戞涓暟
	{
		szKey = xmlNodeGetContent(curNode);
		segmentcount = atoi((char*) szKey);
		//cout << "缃戞涓暟涓猴細" << segmentcount << endl<<endl;
		xmlFree(szKey);
	} else {
		return "6";
	}
	cout << "寮€濮嬪垱寤虹綉缁滆妭鐐?\n";
	NodeContainer nodes;                            //鎵€鏈夎妭鐐瑰瓨鏀惧湪杩欎釜container鍐?
	nodes.Create(nodecount);
	cout << "瀹夎缃戠粶鍗忚\n";
	InternetStackHelper internetv4;
	internetv4.Install(nodes);
	NodeContainer *nodecontainer = new NodeContainer[segmentcount];                 //涓€涓猚ontainer鏁扮粍, 澶у皬涓虹綉娈垫暟, 姣忎釜鍏冪礌鏈変袱涓妭鐐?
	if (nodecontainer == NULL) {
		return "7";
	}
	Ipv4InterfaceContainer *ipv4interfaceiontainer =
			new Ipv4InterfaceContainer[segmentcount];                       //璁板綍浜嗕竴鍧楃綉鍗′笂鐨処P鍦板潃闆? 杩樻槸鍗曠嫭鐨刬p鍦板潃?
	if (ipv4interfaceiontainer == NULL) {
		return "7";
	}
	NetDeviceContainer *netdevicecontainer =
			new NetDeviceContainer[segmentcount];                           //缃戝崱闆嗗悎, 姣忎釜璁惧鍙互鏈夊鍧楃綉鍗? 骞朵笖浼氬鍑轰竴鍧椾笉鍙敤缃戝崱. 涓嬫爣涓?.
	if (ipv4interfaceiontainer == NULL) {
		return "7";
	}
	int *segmentid=new int[segmentcount];
	for(int i=0;i<segmentcount;i++)
	{
		segmentid[i]= -1;
	}
	int *nodeid=new int[nodecount];                                                 //璁板綍浜嗗搴斾綅缃畁ode鏁扮粍鐨勮澶囩殑ID鍙? 鏈▼搴忓唴ID鍙蜂负node鏁扮粍涓嬫爣
	for(int i=0;i<nodecount;i++)
	{
		nodeid[i]= -1;                           
	}
	//cout << "寮€濮嬪垱寤轰俊閬? << endl<<endl;
	CsmaHelper csma; //淇￠亾
	csma.SetDeviceAttribute("Mtu", UintegerValue(1500));
	csma.SetChannelAttribute("DataRate", DataRateValue(5000000));
	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(2)));

	int localID;                                                            //璁板綍鏈湴璁惧淇℃伅
	string localipaddress = "null";
	string localmask;
	int remoteID;                                                           //璁板綍杩滅▼璁惧淇℃伅
	string remoteipaddress = "null";
	string remotemask;
	curNode = curNode->next;
	
    return "5";

//--------------------------鑾峰彇鏈湴涓绘満鍜岃繙绋嬩富鏈虹殑ID鍙?-------------------------------
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "local"))) //鏈湴涓绘満
	{
                //cout<< curNode->name<< endl;
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			localID = atoi((char*) szAttr);
			//cout << "璇诲彇婧愯妭鐐笽D锛? << localID << endl;
			xmlFree(szAttr);
		}
	}else if((!xmlStrcmp(curNode->name, (const xmlChar *) "remote"))){
                xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			remoteID = atoi((char*) szAttr);
			//cout << "璇诲彇鐩爣鑺傜偣ID锛? << remoteID << endl<<endl;
                        //cout << "璇诲彇鐩爣鑺傜偣ID锛? << remoteID << endl<<endl;
			xmlFree(szAttr);
		}
        }
	//鐩爣涓绘満
	curNode = curNode->next;
	if ((!xmlStrcmp(curNode->name, (const xmlChar *) "remote"))) //鐩爣涓绘満
	{
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			remoteID = atoi((char*) szAttr);
			//cout << "璇诲彇鐩爣鑺傜偣ID锛? << remoteID << endl<<endl;
                      //  cout << "璇诲彇鐩爣鑺傜偣ID锛? << remoteID << endl<<endl;
			xmlFree(szAttr);
		}
	}else if((!xmlStrcmp(curNode->name, (const xmlChar *) "local"))){
               // cout<< curNode->name<< endl;
		xmlAttrPtr attrPtr = curNode->properties;
		if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣1ID
				{
			xmlChar* szAttr;
			szAttr = xmlGetProp(curNode, BAD_CAST "ID");
			localID = atoi((char*) szAttr);
			//cout << "璇诲彇婧愯妭鐐笽D锛? << localID << endl;
                       // cout << "璇诲彇婧愯妭鐐笽D锛? << localID << endl;
			xmlFree(szAttr);
		}
        }
//------------------------------------------------------------------------------

	curNode = curNode->next;
	segNode = curNode;
 


//---------------------寮€濮嬭繘鍏ョ綉娈靛惊鐜? 涓虹綉娈典袱绔殑璁惧杩涜閰嶇疆-------------------------        
	while (segNode != NULL
			&& (!xmlStrcmp(segNode->name, (const xmlChar *) "segment"))) {
		int segmentID;
		int segmentID1;
		curNode = segNode;
		if ((!xmlStrcmp(curNode->name, (const xmlChar *) "segment"))) //缃戞寰幆
		{
			xmlNodePtr propSegPtr = curNode;
			xmlAttrPtr attrPtr = propSegPtr->properties;
			if (!xmlStrcmp(attrPtr->name, BAD_CAST "ID")) //缃戞ID
					{
				xmlChar* szAttr = xmlGetProp(propSegPtr, BAD_CAST "ID");
				segmentID = atoi((char*) szAttr);
				segmentID1=segmentID;
				//cout << "缃戞" << segmentID<< "寮€濮嬪垱寤猴細" << endl;
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

		eleNode1 = curNode; //鑺傜偣1
		eleNode2 = curNode->next; //鑺傜偣2
		xmlNodePtr propNodePtr1 = eleNode1;
		xmlNodePtr propNodePtr2 = eleNode2;
		int ID1; //鑺傜偣1缂栧彿
		int ID2; //鑺傜偣2缂栧彿
		int ID11; //鑺傜偣1缂栧彿
		int ID22; //鑺傜偣2缂栧彿
		if ((!xmlStrcmp(eleNode1->name, (const xmlChar *) "node"))
				&& (!xmlStrcmp(eleNode2->name, (const xmlChar *) "node"))) //涓や釜鑺傜偣
				{
			xmlAttrPtr attrPtr1 = propNodePtr1->properties;
			xmlAttrPtr attrPtr2 = propNodePtr2->properties;
			xmlChar* szAttr1;
			xmlChar* szAttr2;
			Ptr<Node> n1;
			Ptr<Node> n2;
			if (!xmlStrcmp(attrPtr1->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣1ID
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
						nodeid[i]=ID1;                          //纭畾ID鍙?
                                                if(ID1== remoteID){
                                                        remoteIndex= i;
                                                }else if(ID1== localID){
                                                        localIndex= i;
                                                }
                                                cout<< ID1+ " \n"; 
						ID1=i;                                  //ID1鍙互纭畾鑺傜偣鍦╪ode鏁扮粍涓殑浣嶇疆.
						break;
					}
				}*/
				xmlFree(szAttr1);
				n1 = nodes.Get(ID1);                                    //鑾峰緱鑺傜偣1
			}
			attrPtr1 = attrPtr1->next;

			if (!xmlStrcmp(attrPtr2->name, BAD_CAST "ID")) //鑾峰緱鑺傜偣2ID
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
				n2 = nodes.Get(ID2); //鑾峰緱鑺傜偣2
			}
			attrPtr2 = attrPtr2->next;

			NodeContainer nc = nodecontainer[segmentID]; //鑺傜偣瀹瑰櫒
			Ipv4InterfaceContainer iic = ipv4interfaceiontainer[segmentID];         //iic鏄繖涓綉娈典腑涓や釜绔偣璁惧鐨処P鎺ュ彛鐨勫鍣?
			nc.Add(n1);
			nc.Add(n2);

			NetDeviceContainer nd = netdevicecontainer[segmentID];
			nd = csma.Install(nc); //缃戠粶璁惧

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
			//鑺傜偣1
			if (!xmlStrcmp(attrPtr1->name, BAD_CAST "type")) {

				szAttr1 = xmlGetProp(propNodePtr1, BAD_CAST "type");
				if ((!xmlStrcmp(szAttr1, (const xmlChar *) "host"))) //涓绘満
				{
					////////////////////////////////////////////////////////
					//璁剧疆IP鍜宮ask
                                        //cout<< "host node"<< nc.Get(0)->GetId()<< " register device!"<< endl;
					//cout << "涓绘満" << ID11 << "寮€濮嬪垱寤猴細" << endl;                //鐜板湪ID11鎵嶆槸鐪熸鐨勮澶囩殑ID鍙?
					nodetype1 = 1;                                  //noudetype涓?鏄富鏈? 涓?鏄矾鐢?
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
						//cout << "涓绘満" << ID11 << "鍒嗛厤IP鍦板潃锛? << ipaddress1 << "  "
								//<< mask1 << endl;
						assignAddr(iic, dev1, ipaddress1.c_str(),
								mask1.c_str());			//璁剧疆
					}
				}
				if ((!xmlStrcmp(szAttr1, (const xmlChar *) "router")))	//璺敱鍣?
				{
					//cout << "璺敱鍣? << ID11 << "寮€濮嬪垱寤猴細" << endl;
                                        //cout<< "route node"<< nc.Get(0)->GetId()<< " register device!"<< endl;
					nodetype1 = 2;
					//璁剧疆IP锛宮ask鍜岃矾鐢辫〃
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
						//cout << "璺敱鍣? << ID11 << "鍒嗛厤IP鍦板潃锛? << ipaddress1 << "  "
								//<< mask1 << endl;
						assignAddr(iic, dev1, ipaddress1.c_str(),
								mask1.c_str());			//璁剧疆

						curNode = curNode->next;
						if ((!xmlStrcmp(curNode->name,
								(const xmlChar *) "routertable"))) {
							routertable1 = curNode;
						}
					}

				}
				xmlFree(szAttr1);
			}

			//鑺傜偣2
			curNode = eleNode2;
			if (!xmlStrcmp(attrPtr2->name, BAD_CAST "type")) {
				szAttr2 = xmlGetProp(propNodePtr2, BAD_CAST "type");
				if ((!xmlStrcmp(szAttr2, (const xmlChar *) "host")))	//涓绘満
				{
					//cout << "涓绘満" << ID22 << "寮€濮嬪垱寤猴細" << endl;
                                        //cout<< "host node"<< nc.Get(1)->GetId()<< " register device!"<< endl;
					nodetype2 = 1;
					////////////////////////////////////////////////////////
					//璁剧疆IP鍜宮ask
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
						//cout << "涓绘満" << ID22 << "鍒嗛厤IP鍦板潃锛? << ipaddress2 << "  "
								//<< mask2 << endl;
						assignAddr(iic, dev2, ipaddress2.c_str(),
								mask2.c_str());			//璁剧疆
					}
				}
				if ((!xmlStrcmp(szAttr2, (const xmlChar *) "router")))	//璺敱鍣?
				{
					//cout << "璺敱鍣? << ID22 << "寮€濮嬪垱寤猴細" << endl;
                                        //cout<< "route node"<< nc.Get(1)->GetId()<< " register device!"<< endl;
					nodetype2 = 2;
					//璁剧疆IP锛宮ask鍜岃矾鐢辫〃
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
						//cout << "璺敱鍣? << ID22 << "鍒嗛厤IP鍦板潃锛? << ipaddress2 << "  "
								//<< mask2 << endl;
						assignAddr(iic, dev2, ipaddress2.c_str(),
								mask2.c_str());			//璁剧疆
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
				if (nodetype1 == 2) {                                   //濡傛灉璁惧1鏄矾鐢卞櫒, 杩涘叆IF
					curNode = routertable1->xmlChildrenNode;
					if((curNode!= NULL)&&(!xmlStrcmp(curNode->name, (const xmlChar *) "item"))) {
						//cout << "璺敱鍣? << ID11 << "閰嶇疆璺敱琛細" << endl;
						xmlNodePtr itemnode = curNode;
						while (itemnode != NULL) {
							dest1.clear();                          //璇诲彇璺敱琛ㄩ」涓殑IP鍦板潃
							curNode = itemnode->xmlChildrenNode;
							szKey = xmlNodeGetContent(curNode);
							dest1.append((char*) szKey);
							xmlFree(szKey);

							string temp;
							temp = dest1.substr(0, 4);
							if (temp != "null") {
								ma1.clear();
								curNode = curNode->next;                       //璇诲彇璺敱琛ㄩ」涓殑瀛愮綉鎺╃爜
								szKey = xmlNodeGetContent(curNode);
								ma1.append((char*) szKey);
								xmlFree(szKey);

								nexthop1.clear();
								curNode = curNode->next;		//璇诲彇璺敱琛ㄩ」涓殑涓嬩竴璺?
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
										setnumber1 = 1;		//璺敱鍣?宸茶缃潤鎬佽矾鐢辫〃
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
					int tag = -1;			//璁板綍绗簩涓矾鐢卞櫒鏄惁璁剧疆闈欐€佽矾鐢?
					curNode = routertable2->xmlChildrenNode;		//item
					if ((curNode!= NULL)&& (!xmlStrcmp(curNode->name, (const xmlChar *) "item"))) {
						//cout << "璺敱鍣? << ID22 << "閰嶇疆璺敱琛細" << endl;
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
					//cout<<"璇诲彇婧愯妭鐐?<<localID<<"IP鍦板潃锛?<<localipaddress<<"  "<<localmask<<endl;
				}
			}
			if (ID22 == localID) {
				temp = ipaddress2.substr(0, 4);
				if (temp != "null") {
					localipaddress = ipaddress2;
					localmask = mask2;
					//cout<<"璇诲彇婧愯妭鐐?<<localID<<"IP鍦板潃锛?<<localipaddress<<"  "<<localmask<<endl;
				}
			}
			if (ID11 == remoteID) {
				temp = ipaddress1.substr(0, 4);
				if (temp != "null") {
					remoteipaddress = ipaddress1;
					remotemask = mask1;
					//cout<<"璇诲彇鐩爣鑺傜偣"<<remoteID<<"IP鍦板潃锛?<<remoteipaddress<<"  "<<remotemask<<endl;
				}
			}
			if (ID22 == remoteID) {
				temp = ipaddress2.substr(0, 4);
				if (temp != "null") {
					remoteipaddress = ipaddress2;
					remotemask = mask2;
					//cout<<"璇诲彇鐩爣鑺???<<remoteID<<"IP鍦板潃锛?<<remoteipaddress<<"  "<<remotemask<<endl;
				}
			}
		} else {
			//return "-4";
		}
		//cout << "缃戞" << segmentID1 << "鍒涘缓缁撴潫銆? << endl << endl;
		segNode = segNode->next;			//涓嬩竴涓綉娈?
	}

	xmlFreeDoc(doc);

//鎸夌収瀹為獙绫诲瀷鍒涘缓搴旂敤
	if (localipaddress != "null" && remoteipaddress != "null") {
		uint32_t packetSize = 1024;
		Time interPacketInterval = Seconds(1.0);
		//cout << "寮€濮嬪湪鑺傜偣涓婂垱寤哄簲鐢細" << endl;
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
	//cout << "瀹氫箟鍖呬俊鎭? << endl;
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
	//cout << "妯℃嫙瀹為獙寮€濮嬭繍琛岋細" << endl;
	Simulator::Run();
	//cout << "妯℃嫙瀹為獙杩愯缁撴潫锛? << endl;
	Simulator::Destroy();
	//cout << "閲婃斁璧勬簮" << endl;



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

