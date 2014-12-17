/**
 * @file
 * @brief 发送
 * @author Xiaosong Gao
 * @note 包含所有发送数据包的操作
 */

#include "precomp.h"

/** 本机的逻辑地址(源地址) */
ACE_UINT8 Connect::source_addr_ = 0;
/** T1连接超时时间。范围为2-16(秒) */
ACE_UINT8 Connect::t1_ = DEFAULT_T1;
/** 定时接收控制器信息的周期，单位是ms */
long Connect::period_time_ = DEFAULT_PERIOD_TIME;

/**
 * @author Xiaosong Gao
 * @param [in] buf 待发送的数据缓冲区
 * @param [in] len 待发送的数据长度
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @note 发送一条消息到控制器
 */
long Connect::send(char * buf, long len)
{
	// 发送数据包
	size_t byte_transferred;
	if (peer_.send_n(buf, len, 0, &timeout_, &byte_transferred) == -1) {
		ACE_ERROR((LM_ERROR , "(%P|%t) %p\n", "send"));
		return ERROR_SEND_FAILED;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 待生成的数据包
 * @param [in] data_len 数据包中的data区域长度
 * @param [out] buf 存放生成数据的缓冲区
 * @param [out] len 缓冲区的长度
 * @note 生成数据包到缓冲区，根据FD_PACKET格式
 */
void Connect::generate_packet(FD_PACKET & packet, long data_len, char * buf, long & len)
{
	// 缓冲区清零
	ACE_OS::memset(buf, 0, MAX_SIZE_BUF);
	// 拷贝协议头，从同步头到协议类型，共9个字节
	char * p = buf;
	long header_len = (char*)(&(packet.info_code)) - (char*)(&packet);	// 协议头长度
	ACE_OS::memcpy(p, &packet, header_len);
	p += header_len;
	// 信息码不等于0，则拷贝信息码和描述码
	if (packet.info_code != 0) {
		*(p++) = packet.info_code;
		*(p++) = packet.description_code;
		// 数据区不为空，则拷贝数据区
		if (packet.data != 0) {
			ACE_OS::memcpy(p, packet.data, data_len);
			p += data_len;
			delete [] packet.data;
		}
	}
	// 拷贝CRC校验码
#ifndef NO_CRC_SEND
	ACE_OS::memcpy(p, &packet.crc16, sizeof(NO_CRC_CHECK));
	p += sizeof(NO_CRC_CHECK);
#endif
	// 计算缓冲区总长度
	len = p - buf;
}

/**
 * @author Xiaosong Gao
 * @param [in] description_code 描述码
 * - H11_DATA_CALL：子站收到该请求后，将数据发送给HMI主站，一次会话结束
 * - H11_DATA_SUBSCRIPTION：子站将数据上送的同时，为该命令开启定时器，将按照设定的节拍主动上送数据
 * - H11_CANCAL_DATA_SUBSCRIPTION：子站将数据上送的同时，取消数据订购
 * @param [in] time_ms 订阅周期
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_H11_TIMER_OUTRANGE 订购时间超过范围
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note DPU数据总召唤=10H
 */
long Connect::send_call_io_data(char description_code, long time_ms)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	packet.control_area_1 = GENERAL_DATA_FRAME;
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	ACE_UINT16 datalen = 2;	// 数据区长度为信息码和描述码长度之和
	packet.length_low = LOW_8_BIT(datalen);
	packet.length_high = HIGH_8_BIT(datalen) | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 封装数据包内容
	packet.info_code = CALL_FDCS_OBSERVATION_POINT_DATA;
	packet.description_code = description_code;
	switch (description_code) {
	case H11_DATA_CALL:
	case H11_CANCAL_DATA_SUBSCRIPTION:
		break;
	case H11_DATA_SUBSCRIPTION:
		if ((time_ms < 10 ) || (time_ms > 65535)) {
			ACE_ERROR((LM_ERROR, "(%P|%t) send_call_io_data time_ms is out of boundary\n"));
			return ERROR_H11_TIMER_OUTRANGE;
		}
		packet.control_area_1 |= (LOW_8_BIT(time_ms) & H11_TIMER_MASK);
		packet.control_area_2 |= HIGH_8_BIT(time_ms);
		break;
	}
	// 产生数据包的缓冲区
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 0, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_IS_ENABLED 设备没有被启用
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note START建立链路请求，HMI发送请求信号
 */
long Connect::send_start_link_request()
{
	// 判断设备/连接是否有效
	if (!isEnabled()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Controller is disabled\n"));
		return ERROR_IS_ENABLED;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = LINK_FRAME | LINK_START_REQUEST;
	packet.control_area_2 = 0x01 << current_host_;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	packet.length_low = 0x00;
	packet.length_high = 0x00 | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 0, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note STOP建立链路请求，HMI发送请求信号
 */
long Connect::send_stop_link_request()
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = LINK_FRAME | LINK_STOP_REQUEST;
	packet.control_area_2 = 0x01 << current_host_;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	packet.length_low = 0x00;
	packet.length_high = 0x00 | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 0, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @param [in] t1 T1连接的超时时间，范围是2-16，单位是秒
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note START建立T1请求，HMI发送请求信号
 */
long Connect::send_start_t1_request(ACE_UINT8 t1)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = T1_TIMEOUT_FRAME | T1_REQUEST_MASK | (t1 << 4);
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	packet.length_low = 0x00;
	packet.length_high = 0x00 | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 0, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @param [in] ioaddr IO逻辑地址
 * @param [in] iotype IO卡类型
 * @param [in] srcaddr 源地址
 * @param [in] direction 数据流向
 * @param [in] priority 优先级
 * @param [in] command 命令码
 * @param [in] subcommand 子命令码
 * @param [in] data 数据区
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note HMI与IO板之间的通信，使用透明数据转发
 */
long Connect::send_io_transparent_data_down(ACE_UINT8 ioaddr, ACE_UINT8 iotype, ACE_UINT8 srcaddr, ACE_UINT8 direction, 
											ACE_UINT8 priority, ACE_UINT8 command, ACE_UINT8 subcommand, ACE_UINT8 * data)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = GENERAL_DATA_FRAME;
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	ACE_UINT8 channels = 0;
	switch (iotype) {
	case DO_8_1:
	case AO_8_1:
		channels = 8;
		break;
	case DO_16_1:
		channels = 16;
		break;
	}
	ACE_UINT16 datalen = 2 + 8 + (3 + 2 * channels);
	packet.length_low = LOW_8_BIT(datalen);
	packet.length_high = HIGH_8_BIT(datalen) | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 封装数据包内容
	packet.info_code = HMI_IO_TRANSPARENT_DATA_DOWN;
	packet.description_code = 0x01;
	packet.data = new ACE_UINT8[8 + (3 + 2 * channels)];
	ACE_UINT16 dl = 3 + 2 * channels;
	packet.data[0] = HIGH_8_BIT(dl);
	packet.data[1] = LOW_8_BIT(dl);
	packet.data[2] = ioaddr;
	packet.data[3] = srcaddr;
	packet.data[4] = direction;
	packet.data[5] = priority;
	packet.data[6] = command;
	packet.data[7] = subcommand;
	packet.data[8] = command;
	switch (iotype) {
	case DO_8_1:
	case DO_16_1:
		packet.data[9] = BOOL_1_BIT;
		break;
	case AO_8_1:
		packet.data[9] = UNSIGNED_INT_2_BYTE;
		break;
	default:
		packet.data[9] = 0;
	}
	packet.data[10] = channels;
	for (long i = 0; i < 2 * channels; i++)
		packet.data[11 + i] = data[i];
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 8 + (3 + 2 * channels), buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @param [in] ioaddr IO逻辑地址
 * @param [in] groupId 8位组号偏移,0-255
 * @param [in] ptId 每位表示一个通道号
 * @param [in] type 数据类型长度
 * @param [in] vvalue 真实值
 * @param [in] vlen 真实值长度
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note 设置AO(数据库)
 */
long Connect::send_control_ao(ACE_UINT8 ioaddr, ACE_UINT8 groupId, ACE_UINT8 ptId, ACE_UINT8 type, ACE_UINT8 * vvalue, ACE_UINT8 vlen)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = GENERAL_DATA_FRAME;
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	ACE_UINT16 datalen = 7 + vlen;
	packet.length_low = LOW_8_BIT(datalen);
	packet.length_high = HIGH_8_BIT(datalen) | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 封装数据包内容
	packet.info_code = DOWN_CONTROL_AO;
	packet.description_code = 0;
	packet.data = new ACE_UINT8[5 + vlen];
	packet.data[0] = dest_addr_;
	packet.data[1] = ioaddr;
	packet.data[2] = groupId;
	packet.data[3] = ptId;
	packet.data[4] = type;
	for (ACE_UINT8 i = 0; i < vlen; i++)
		packet.data[5 + i] = vvalue[i];
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 5 + vlen, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @param [in] ioaddr IO逻辑地址
 * @param [in] groupId 8位组号偏移,0-255
 * @param [in] ptId 每位表示一个通道号
 * @param [in] vvalue 真实值
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note 设置DO(数据库)
 */
long Connect::send_control_do(ACE_UINT8 ioaddr, ACE_UINT8 groupId, ACE_UINT8 ptId, ACE_UINT8 vvalue)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = GENERAL_DATA_FRAME;
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	ACE_UINT16 datalen = 7;
	packet.length_low = LOW_8_BIT(datalen);
	packet.length_high = HIGH_8_BIT(datalen) | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 封装数据包内容
	packet.info_code = DOWN_CONTROL_DO;
	packet.description_code = 0;
	packet.data = new ACE_UINT8[5];
	packet.data[0] = dest_addr_;
	packet.data[1] = ioaddr;
	packet.data[2] = groupId;
	packet.data[3] = ptId;
	packet.data[4] = vvalue;
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 5, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}

/**
 * @author Xiaosong Gao
 * @param [in] ioaddr IO逻辑地址
 * @param [in] iot 标签点信息
 * @retval NIC_SUCCESS 发送成功
 * @retval ERROR_SEND_FAILED 发送失败
 * @retval ERROR_NOBUILDLINK 没有建立握手连接，就发出了请求或其他数据请求
 * @note 设置VPT
 */
long Connect::send_control_vpt(ACE_UINT8 ioaddr, IO_T & iot)
{
	// 判断设备/连接是否有效
	if (!isValid()) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Connection is not created\n"));
		return ERROR_NOBUILDLINK;
	}
	ACE_DEBUG((LM_DEBUG, "send_control_ao : making packet.\n"));
	// 封装数据包头
	FD_PACKET packet;
	ACE_OS::memset(&packet, 0, sizeof(FD_PACKET));
	packet.sync_header = SYNC_HEADER;
	packet.dest_addr = dest_addr_;
	packet.source_addr = Connect::source_addr_;
	packet.control_area_1 = GENERAL_DATA_FRAME;
	packet.control_area_2 = 0x00;
	packet.control_area_3 = 0x00;
	packet.protocol_type = (ACE_UINT8)(ONET_REAL_TRANSFER_PROTOCOL | HIGH_BYTE_ORDER);
	//
	ACE_UINT16 vlen;
	switch (iot.datatype)
	{
	case INT_1_BYTE:
	case UNSIGNED_INT_1_BYTE:
		vlen = 1;
		break;
	case BOOL_1_BIT:
	case INT_2_BYTE:
	case UNSIGNED_INT_2_BYTE:
	case FLOAT_2_BYTE:
		vlen = 2;
		break;
	case FLOAT_3_BYTE:
		vlen = 3;
	case INT_4_BYTE:
	case UNSIGNED_INT_4_BYTE:
	case FLOAT_4_BYTE:
		vlen = 4;
		break;
	}
	ACE_UINT16 datalen = 7+vlen;
	packet.length_low = LOW_8_BIT(datalen);
	packet.length_high = HIGH_8_BIT(datalen) | (PROTOCOL_VERSION << 4);
	packet.crc16 = NO_CRC_CHECK;
	// 封装数据包内容
	packet.info_code = DOWN_CONTROL_VPT;
	packet.description_code = 0;
	packet.data = new ACE_UINT8[5 + vlen];
	packet.data[0] = dest_addr_;
	packet.data[1] = ioaddr;//254;//ioaddr;
	packet.data[2] = HIGH_8_BIT(iot.channel);//offset
	packet.data[3] = LOW_8_BIT(iot.channel);//若6007  startchannel = 7
	// 类型
	switch (iot.datatype)
	{
	case BOOL_1_BIT:
		packet.data[4] = 0x1f;
		break;
	default:
		packet.data[4] = 0x32;
	} 
	// 数值
	ACE_UINT8 fbuf[4];
	switch (iot.datatype)
	{
		// bool
	case BOOL_1_BIT:
		packet.data[5] = 0x1 << (iot.subchannel);
		packet.data[6] = (iot.databuf.b ? 0xff : 0x00);
		break;
		// 1 byte
	case INT_1_BYTE:
		packet.data[5] = iot.databuf.c;
		break;
	case UNSIGNED_INT_1_BYTE:
		packet.data[5] = iot.databuf.uc;
		break;
		// 2 byte
	case INT_2_BYTE:
		packet.data[5] = HIGH_8_BIT(iot.databuf.s);
		packet.data[6] = LOW_8_BIT(iot.databuf.s);
		break;
	case UNSIGNED_INT_2_BYTE:
		packet.data[5] = HIGH_8_BIT(iot.databuf.us);
		packet.data[6] = LOW_8_BIT(iot.databuf.us);
		break;
	case FLOAT_2_BYTE:
		memcpy(fbuf, &iot.databuf.f, sizeof(float));
		packet.data[5] = fbuf[1];
		packet.data[6] = fbuf[0];
		break;
		// 3 byte
	case FLOAT_3_BYTE:
		memcpy(fbuf, &iot.databuf.f, sizeof(float));
		packet.data[5] = fbuf[2];
		packet.data[6] = fbuf[1];
		packet.data[7] = fbuf[0];
		break;
		// 4 byte
	case INT_4_BYTE:
		packet.data[5] = (ACE_UINT8)(((iot.databuf.g) & 0xff000000) >> 24);
		packet.data[6] = (ACE_UINT8)(((iot.databuf.g) & 0xff0000) >> 16);
		packet.data[7] = (ACE_UINT8)(((iot.databuf.g) & 0xff00) >> 8);
		packet.data[8] = (ACE_UINT8)((iot.databuf.g) & 0xff);
		break;
	case UNSIGNED_INT_4_BYTE:
		packet.data[5] = (ACE_UINT8)(((iot.databuf.ug) & 0xff000000) >> 24);
		packet.data[6] = (ACE_UINT8)(((iot.databuf.ug) & 0xff0000) >> 16);
		packet.data[7] = (ACE_UINT8)(((iot.databuf.ug) & 0xff00) >> 8);
		packet.data[8] = (ACE_UINT8)((iot.databuf.ug) & 0xff);
		break;
	case FLOAT_4_BYTE:
		memcpy(fbuf, &iot.databuf.f, sizeof(float));
		packet.data[5] = fbuf[3];
		packet.data[6] = fbuf[2];
		packet.data[7] = fbuf[1];
		packet.data[8] = fbuf[0];
		break;
	}
	// 产生数据包
	char buf[MAX_SIZE_BUF];
	long buf_len;
	generate_packet(packet, 5 + 2, buf, buf_len);
	// 发送数据包
	return this->send(buf, buf_len);
}