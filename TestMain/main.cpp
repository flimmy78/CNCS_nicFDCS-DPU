#include "nicConnect.h"
#include <ace/OS_NS_unistd.h>
#include <ace/Log_Msg.h>
#include <iostream>
#include <fstream>

using namespace std;

#pragma comment(lib, "nicFDCS-DPU.lib")

#define set_value set_value_library

int main(int, char *[])
{
	long CONTROL_LOGICAL_ADDR;
	char hostaddr[32];
	ifstream fin;
	fin.open("config.txt");
	fin >> CONTROL_LOGICAL_ADDR;
	fin >> hostaddr;
	fin.close();
	fin.clear();
	init_service();
	Connect::setSourceAddr(100);
	if (conns[CONTROL_LOGICAL_ADDR].connect(hostaddr, "") != NIC_SUCCESS) {
		cout << "cannot connect to the dpu." << endl;
		return -1;
	}
	char buf[1000];
	char filename[1000];
	long datalen = 0, ret;
	IO_T iochannel;
	long temp;
	while (1) {
		long choice;
		cout<< "Choose Options:" << endl
			<< "1. Output all data.\n"
			<< "2. Send a customized packet (file).\n"
			<< "3. Get a value (file).\n"
			<< "4. Set a library value (file).\n"
			<< "5. Set a iocard value (file).\n"
			<< "0. Exit!!!\n"
			<< "Please input your choice: ";
		cin >> choice;
		cout << endl;
		switch (choice) {
		case 1:
			conns[CONTROL_LOGICAL_ADDR].debug_all_data();
			break;
		case 3:
			cout << "Input the filename: ";		// ioaddr, startchannel, endchannel, channel, subchannel, datatype
			cin >> filename;
			fin.open(filename);
			fin >> temp;
			iochannel.iocard = temp;
			fin >> temp;
			iochannel.startchannel = temp;
			fin >> temp;
			iochannel.endchannel = temp;
			fin >> temp;
			iochannel.channel = temp;
			fin >> temp;
			iochannel.subchannel = temp;
			fin >> temp;
			iochannel.datatype = temp;
			fin.close();
			fin.clear();
			iochannel.dpu = CONTROL_LOGICAL_ADDR;
			if ((ret = get_value(iochannel)) != NIC_SUCCESS)
				cout << "get value failed.\n";
			else {
				switch (iochannel.datatype) {
				case BOOL_1_BIT: cout << (iochannel.databuf.b ? "true" : "false") << endl; break;
				case FLOAT_4_BYTE:
				case FLOAT_3_BYTE:
				case FLOAT_2_BYTE: cout << iochannel.databuf.f << endl; break;
				case INT_1_BYTE: cout << iochannel.databuf.c << endl; break;
				case INT_2_BYTE: cout << iochannel.databuf.s << endl; break;
				case INT_4_BYTE: cout << iochannel.databuf.g << endl; break;
				case UNSIGNED_INT_1_BYTE: cout << iochannel.databuf.uc << endl; break;
				case UNSIGNED_INT_2_BYTE: cout << iochannel.databuf.us << endl; break;
				case UNSIGNED_INT_4_BYTE: cout << iochannel.databuf.ug << endl; break;
				}
			}
			break;
		case 4:
		case 5:
			cout << "Input the filename: ";		// ioaddr, startchannel, endchannel, channel, subchannel, datatype, value
			cin >> filename;
			fin.open(filename);
			fin >> temp;
			iochannel.iocard = temp;
			fin >> temp;
			iochannel.startchannel = temp;
			fin >> temp;
			iochannel.endchannel = temp;
			fin >> temp;
			iochannel.channel = temp;
			fin >> temp;
			iochannel.subchannel = temp;
			fin >> temp;
			iochannel.datatype = temp;
			switch (iochannel.datatype) {
			case BOOL_1_BIT: fin >> iochannel.databuf.b; break;
			case FLOAT_4_BYTE:
			case FLOAT_3_BYTE:
			case FLOAT_2_BYTE: fin >> iochannel.databuf.f; break;
			case INT_1_BYTE: fin >> iochannel.databuf.c; break;
			case INT_2_BYTE: fin >> iochannel.databuf.s; break;
			case INT_4_BYTE: fin >> iochannel.databuf.g; break;
			case UNSIGNED_INT_1_BYTE: fin >> iochannel.databuf.uc; break;
			case UNSIGNED_INT_2_BYTE: fin >> iochannel.databuf.us; break;
			case UNSIGNED_INT_4_BYTE: fin >> iochannel.databuf.ug; break;
			}
			fin.close();
			fin.clear();
			iochannel.dpu = CONTROL_LOGICAL_ADDR;
			if (choice == 4) {
				if ((ret = set_value_library(iochannel)) != NIC_SUCCESS)
					cout << "set value library failed.\n";
			} else if (choice == 5) {
				if ((ret = set_value_iocard(iochannel)) != NIC_SUCCESS)
					cout << "set value iocard failed.\n";
			}
			break;
		case 2:
			cout << "Input the filename: ";
			cin >> filename;
			fin.open(filename);
			fin >> datalen;
			for (long i = 0; i < datalen; i++) {
				fin >> temp;
				buf[i] = temp;
			}
			conns[CONTROL_LOGICAL_ADDR].send(buf, datalen);
			fin.close();
			fin.clear();
			ACE_OS::sleep(1);
			break;
		case 0:
			break;
		default:
			cout << "Choice is wrong, please re-input.\n";
			break;
		}
		if (choice == 0)
			break;
		cin.clear();
		cout << endl;
	}
	conns[CONTROL_LOGICAL_ADDR].disconnect();
	return 0;
};