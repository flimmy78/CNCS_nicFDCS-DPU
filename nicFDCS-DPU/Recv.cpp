/**
* @file
* @brief 接收
* @author Xiaosong Gao
* @note 包括所有接收数据包的操作
*/

#include "precomp.h"

/** 接收数据的超时时间 */
ACE_Time_Value Connect::timeout_(0, DEFAULT_TIMEOUT);

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS 接收成功
 * @retval ERROR_RECV_TIMEOUT 接收超时
 * @retval ERROR_IS_ENABLED 设备没有被启用
 * @retval ERROR_UNKNOW_SFDCS 包头不是0x68H
 * @note 接收一条消息，存入data_buf_中，长度为data_len_，并解析协议头，调用相应的处理函数
 */
long Connect::recv()
{
	// 判断设备/连接是否有效
	if (!isEnabled())
		return ERROR_IS_ENABLED;
	// 接收数据，存到data_buf_和data_len_中
	recv_len_ = peer_.recv(recv_buf_, MAX_SIZE_RECV_BUF, &timeout_);
	if (recv_len_ == -1) {
		ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "recv"));
		return ERROR_RECV_TIMEOUT;
	}
	recv_buf_[recv_len_] = 0;
	// 解析数据包头，并调用相应的函数
	recv_read_ptr_ = recv_buf_;
	while (*recv_read_ptr_ == SYNC_HEADER) {
		FD_PACKET packet;
		ACE_OS::memcpy(&packet, recv_read_ptr_, 9);
		recv_read_ptr_ += 9;
		switch (packet.control_area_1 & FRAME_FORMAT_MASK) {	// 根据控制域1后两位判断帧的格式
		case GENERAL_DATA_FRAME:	// 一般数据帧
			packet.info_code = *recv_read_ptr_;
			packet.description_code = *(recv_read_ptr_+1);
			switch (packet.info_code) {
			case ANSWER_FDCS_WHOLE_SYSTEM_OBSERVATION_POINT_DATA:		// IO数据应答=0x21H
				data_len_ = packet.length_low + 0x100 * (packet.length_high & LENGTH_HIGH_MASK) - 2;
				acquire();
				ACE_OS::memcpy(data_buf_, recv_read_ptr_ + 2, data_len_);
				recv_answer_io_data(packet);
				release();
				break;
			case RETURN_OPERATION_FAILED_MESSAGE:
				recv_operation_failed_message(packet);
				break;
			default:
				debug_packet_content(recv_read_ptr_ - 9, packet.length_low + 0x100 * (packet.length_high & LENGTH_HIGH_MASK) + 9);
			}
			break;
		case T1_TIMEOUT_FRAME:		// T1连接帧
			if (packet.control_area_1 & T1_ACK_MASK)	// T1连接确认数据包
				recv_start_t1_ack(packet);
			break;
		case T2_TIMEOUT_FRAME:		// T2连接帧
			break;
		case LINK_FRAME:			// 链路握手信号
			if (packet.control_area_1 & LINK_START_ACK)
				recv_start_link_ack(packet);
			else if (packet.control_area_1 & LINK_STOP_ACK)
				recv_stop_link_ack(packet);
			break;
		}
		recv_read_ptr_ += packet.length_low + 0x100 * (packet.length_high & LENGTH_HIGH_MASK);
	}
	if (*recv_read_ptr_ != 0) {
		ACE_ERROR((LM_ERROR, "Recv ptr does not point to a sync header\n"));
		return ERROR_UNKNOW_SFDCS;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 接收到的数据包(已经解析了协议头)
 * @retval NIC_SUCCESS 处理成功
 * @note START链路建立确认，DPU返回确认信号
 */
long Connect::recv_start_link_ack(FD_PACKET & packet)
{
	linked_ = true;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 接收到的数据包(已经解析了协议头)
 * @retval NIC_SUCCESS 处理成功
 * @note STOP链路停止确认，DPU返回确认信号
 */
long Connect::recv_stop_link_ack(FD_PACKET & packet)
{
	linked_ = false;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 接收到的数据包(已经解析了协议头)
 * @retval NIC_SUCCESS 处理成功
 * @note T1链路建立确认，DPU返回确认信号
 */
long Connect::recv_start_t1_ack(FD_PACKET & packet)
{
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 接收到的数据包(已经解析了协议头)
 * @retval NIC_SUCCESS 处理成功
 * @note 处理错误报文
 */
long Connect::recv_operation_failed_message(FD_PACKET & packet)
{
	switch (packet.description_code) {
	case ERROR_UNKNOW_SFDCS: ACE_DEBUG((LM_DEBUG, "包头不是0x68H")); break;
	case ERROR_NOBUILDLINK: ACE_DEBUG((LM_DEBUG, "没有建立握手连接，就发出了请求或其他数据请求")); break;
	case ERROR_SUBSC_INVALID: ACE_DEBUG((LM_DEBUG, "申请订购定时器失败")); break;
	case ERROR_ALREADYHASTIMER: ACE_DEBUG((LM_DEBUG, "无用")); break;
	case ERROR_TIMEROVERFLOW: ACE_DEBUG((LM_DEBUG, "申请的定时器小于1")); break;
	case ERROR_HASNOTIMER: ACE_DEBUG((LM_DEBUG, "目前无用")); break;
	case ERROR_ALREADYLINK: ACE_DEBUG((LM_DEBUG, "已经建立了连接，又重新申请连接")); break;
	case ERROR_INVALIDPACKET: ACE_DEBUG((LM_DEBUG, "报头不是0x68H")); break;
	case ERROR_LENTOOSHORT: ACE_DEBUG((LM_DEBUG, "报头中长度小于等于0")); break;
	case ERROR_IODEAD: ACE_DEBUG((LM_DEBUG, "访问的IO已死")); break;
	case ERROR_IOADDR: ACE_DEBUG((LM_DEBUG, "IO地址错")); break;
	case ERROR_UNKNOWNDATATYPE: ACE_DEBUG((LM_DEBUG, "下行数据的数据类型，DPU没有定义")); break;
	case ERROR_MAKEFRAME: ACE_DEBUG((LM_DEBUG, "组祯出现了错误")); break;
	case ERROR_NOVALIDIO: ACE_DEBUG((LM_DEBUG, "请求数据中IO地址都找不到")); break;
	case ERROR_INVALID_TARGET: ACE_DEBUG((LM_DEBUG, "DPU地址不符合定义")); break;
	case ERROR_INVALID_SRC: ACE_DEBUG((LM_DEBUG, "HMI节点地址不符合定义")); break;
	case ERROR_REGISTER_DEV: ACE_DEBUG((LM_DEBUG, "注册通信设备地址出错")); break;
	case ERROR_TARGET_NOTCORRECT: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_DPU_STATUS_NORUN: ACE_DEBUG((LM_DEBUG, "DPU状态不是运行态")); break;
	case ERROR_DEVNO_INVALID: ACE_DEBUG((LM_DEBUG, "在节点中设备序号无效")); break;
	case ERROR_NODE_OVERLIMIT: ACE_DEBUG((LM_DEBUG, "请求DPU或IO的数目太多，超过了限制")); break;
	case ERROR_NO_PROTOCOL: ACE_DEBUG((LM_DEBUG, "协议没有初始化")); break;
	case ERROR_NODE_HASNOADDR: ACE_DEBUG((LM_DEBUG, "请求中出现了无效的DPU或IO地址")); break;
	case ERROR_NODE_INVALID: ACE_DEBUG((LM_DEBUG, "返回无效的地址")); break;
	case SYS_MEM_ALLOC_ERR: ACE_DEBUG((LM_DEBUG, "系统分配内存出错")); break;
	case SYS_QERR_QFULL: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_RIGHT_NOWRITE: ACE_DEBUG((LM_DEBUG, "没有写权限")); break;
	case ERROR_OPEN_FILENAME: ACE_DEBUG((LM_DEBUG, "打开文件错")); break;
	case ERROR_NOEVENT_ITEM: ACE_DEBUG((LM_DEBUG, "读事件缓冲区错（空）")); break;
	case ERROR_NOLOG_ITEM: ACE_DEBUG((LM_DEBUG, "读LOG缓冲区错（空）")); break;
	case ERROR_OVERLIMIT: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_NO_LBDATA: ACE_DEBUG((LM_DEBUG, "没有录波文件")); break;
	case ERROR_SETTING_INVALID: ACE_DEBUG((LM_DEBUG, "定值区无效（指读取NVRAM失败、定值区无效）")); break;
	case ERROR_SETTING_AREA: ACE_DEBUG((LM_DEBUG, "定值区号错")); break;
	case ERROR_SETTING_NUM: ACE_DEBUG((LM_DEBUG, "定值个数错")); break;
	case ERROR_SETTING_OVERFLOW: ACE_DEBUG((LM_DEBUG, "定值越限")); break;
	case ERROR_SETTING_CRC: ACE_DEBUG((LM_DEBUG, "定值crc校验错")); break;
	case ERROR_SETTING_TYPE: ACE_DEBUG((LM_DEBUG, "定值类型错")); break;
	case ERROR_SETTING_STATE: ACE_DEBUG((LM_DEBUG, "操作状态错")); break;
	case ERROR_SETTING_WFILE: ACE_DEBUG((LM_DEBUG, "往配置文件写定值区号错")); break;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet 接收到的数据包(已经解析了协议头)
 * @retval NIC_SUCCESS 处理成功
 * @note IO数据应答=21H，解析数据内容，并存入相应的缓冲区中
 */
long Connect::recv_answer_io_data(FD_PACKET & packet)
{
	char * p = data_buf_;
	if (packet.description_code & H21_TIME_MARK) {		// 含有时标则为1，否则为0
		ACE_OS::memcpy(&time_mark_, p, H21_TIME_MARK_LENGTH);		// 拷贝时标
		p += H21_TIME_MARK_LENGTH;
	}
	// DPU_NUM
	if (*(p++) != 1)
		return ERROR_NODE_OVERLIMIT;
	// DPU_DYNA_ATTR
	ACE_OS::memcpy(&all_data_.dpu_attr, p, 6);
	p += 6;
	// io_num
	if (all_data_.io_num != *p) {
		if (all_data_.io_num != 0) {
			delete [] all_data_.io_cards;
		}
		all_data_.io_num = *p;
		all_data_.io_cards = new IO_DATA_UNION[*p];
		ACE_OS::memset((void*)all_data_.io_cards, 0, sizeof(IO_DATA_UNION) * (*p));
	}
	p++;
	// IO_CARDS
	for (long i = 0; i < all_data_.io_num; i++) {
		IO_DATA_UNION & iocard = all_data_.io_cards[i];
		// TYPE
		ACE_UINT8 type = *(p+3);
		if (type == VPTGROUP || type == RETVPTGROUP)
			iocard.iotype = VPT_CARD;
		else if (type == IEDMODEL)
			iocard.iotype = IED_CARD;
		else
			iocard.iotype = IO_CARD;
		// UNION
		ACE_UINT8 lenhigh = 0;
		ACE_UINT8 lenlow = 0;
		switch (iocard.iotype) {
		case IO_CARD:
			// ATTR
			iocard.ioid = *(p++);
			iocard.ioaddr = *(p++);
			iocard.status = *(p++);
			iocard.type = *(p++);
			p += 10;
			// VALUE
			iocard.qlen = *(p++);
			iocard.quality = (ACE_UINT8 *)p;
			p += iocard.qlen;
			iocard.vlen = *(p++);
			iocard.vvalue = (ACE_UINT8 *)p;
			p += iocard.vlen;
			break;
		case IED_CARD:
			// ATTR
			iocard.ioid = *(p++);
			iocard.ioaddr = *(p++);
			iocard.status = *(p++);
			iocard.type = *(p++);
			p += 2;
			// VALUE
			lenhigh = *(p++);
			lenlow = *(p++);
			iocard.qlen = 0x100 * lenhigh + lenlow;
			iocard.quality = (ACE_UINT8 *)p;
			p += iocard.qlen;
			lenhigh = *(p++);
			lenlow = *(p++);
			iocard.vlen = 0x100 * lenhigh + lenlow;
			iocard.vvalue = (ACE_UINT8 *)p;
			p += iocard.vlen;
			break;
		case VPT_CARD:
			// ATTR
			iocard.ioid = *(p++);
			iocard.ioaddr = *(p++);
			iocard.status = *(p++);
			iocard.type = *(p++);
			// QUALIFY
			p += 2;
			// VALUE
			lenhigh = *(p++);
			lenlow = *(p++);
			iocard.vlen = 0x100 * lenhigh + lenlow;
			iocard.vvalue = (ACE_UINT8 *)p;
			p += iocard.vlen;
			break;
		}
	}
	return NIC_SUCCESS;
}