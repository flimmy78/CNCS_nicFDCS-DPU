/**
 * @file
 * @brief 对外接口
 * @author Xiaosong Gao
 * @note 包括对标签点的操作
 */

#include "precomp.h"

/** 全局的连接集合 */
Connect conns[MAX_DPU_NUM];

/**
 * @author Xiaosong Gao
 * @note 初始化服务
 */
void init_service()
{
	for (ACE_UINT8 i = 1; i < MAX_DPU_NUM; i++)
		conns[i].setDestAddr(i);
}

/**
 * @author Xiaosong Gao
 * @param [in] iochannel 待获得的标签点信息
 * @param [in] iocard_num 返回的IO卡序号
 * @retval NIC_SUCCESS 查找成功
 * @retval ERROR_DPU_OUT_RANGE DPU序号超出范围
 * @retval ERROR_IO_OUT_RANGE IO板卡地址超出范围
 * @retval ERROR_CHANNEL_OUT_RANGE IO通道序号超出范围
 * @note 检查IO信息是否有错误
 */
long check_io_status(IO_T & iochannel, long & iocard_num)
{
	// DPU
	if (iochannel.dpu < 0 || iochannel.dpu >= MAX_DPU_NUM || (!conns[iochannel.dpu].isValid())) {
		ACE_ERROR((LM_ERROR, "(%P|%t) DPU number is wrong\n"));
		return ERROR_DPU_OUT_RANGE;
	}
	Connect * conn = &conns[iochannel.dpu];
	// IO Card
	if (iochannel.iocard < 0 || iochannel.iocard > 0xFF) {
		ACE_ERROR((LM_ERROR, "(%P|%t) IO Card address is out of range\n"));
		return ERROR_IO_OUT_RANGE;
	}
	conn->acquire();
	for (iocard_num = 0; iocard_num < conn->all_data_.io_num; iocard_num++) {	// 查找IO卡
		if (conn->all_data_.io_cards[iocard_num].ioaddr == iochannel.iocard)
			break;
	}
	if (iocard_num == conn->all_data_.io_num) {		// 没有找到
		ACE_ERROR((LM_ERROR, "(%P|%t) IO Card is not found on DPU\n"));
		conn->release();
		return ERROR_IO_OUT_RANGE;
	}
	// Channel
	if (iochannel.channel < 0 || iochannel.channel < iochannel.startchannel || iochannel.channel > iochannel.endchannel) {
		ACE_ERROR((LM_ERROR, "(%P|%t) IO Channel is out of range\n"));
		conn->release();
		return ERROR_CHANNEL_OUT_RANGE;
	}
	conn->release();
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] iochannel 待获得的标签点信息
 * @retval NIC_SUCCESS 查找成功
 * @retval ERROR_DPU_OUT_RANGE DPU序号超出范围
 * @retval ERROR_IO_OUT_RANGE IO板卡地址超出范围
 * @retval ERROR_CHANNEL_OUT_RANGE IO通道序号超出范围
 * @note 从缓冲区中获得标签点的实时值
 */
long get_value(IO_T & iochannel)
{
	static char buf[MAX_SIZE_BUF];
	long iocard_num, ret;
	// 检查IO状态
	if ((ret = check_io_status(iochannel, iocard_num)) != NIC_SUCCESS)
		return ret;
	Connect * conn = &conns[iochannel.dpu];
	conn->acquire();
	// 获得数据区
	ACE_UINT8 qlen;
	if (conn->all_data_.io_cards[iocard_num].type == AI_8_1 ||
		conn->all_data_.io_cards[iocard_num].type == AI_4_1)
		qlen = 2;
	else if (conn->all_data_.io_cards[iocard_num].iotype == VPT_CARD)
		qlen = 0;
	else
		qlen = 1;
	// 获得数值
	ACE_UINT8 * dbuf = conn->all_data_.io_cards[iocard_num].vvalue;
	ACE_UINT8 * qbuf = conn->all_data_.io_cards[iocard_num].quality;
	long pos = iochannel.channel - iochannel.startchannel;
	switch (iochannel.datatype) {
	case BOOL_1_BIT:
		iochannel.databuf.b = (dbuf[pos] & (0x1 << iochannel.subchannel)) ? true : false;
		break;
	case FLOAT_4_BYTE:
		iochannel.databuf.f = dbuf[pos] * 0x01000000 + dbuf[pos+1] * 0x010000 + dbuf[pos+2] * 0x0100 + dbuf[pos+3];
		break;
	case FLOAT_3_BYTE:
		iochannel.databuf.f = dbuf[pos] * 0x010000 + dbuf[pos+1] * 0x0100 + dbuf[pos+2];
		break;
	case FLOAT_2_BYTE:
		iochannel.databuf.f = dbuf[pos] * 0x0100 + dbuf[pos+1];
		break;
	case UNSIGNED_INT_4_BYTE:
		iochannel.databuf.ug = dbuf[pos] * 0x01000000 + dbuf[pos+1] * 0x010000 + dbuf[pos+2] * 0x0100 + dbuf[pos+3];
		break;
	case UNSIGNED_INT_2_BYTE:
		iochannel.databuf.us = dbuf[pos] * 0x0100 + dbuf[pos+1];
		break;
	case UNSIGNED_INT_1_BYTE:
		iochannel.databuf.uc = dbuf[pos];
		break;
	case INT_4_BYTE:
		iochannel.databuf.g = dbuf[pos] * 0x01000000 + dbuf[pos+1] * 0x010000 + dbuf[pos+2] * 0x0100 + dbuf[pos+3];
		break;
	case INT_2_BYTE:
		iochannel.databuf.s = dbuf[pos] * 0x0100 + dbuf[pos+1];
		break;
	case INT_1_BYTE:
		iochannel.databuf.c = dbuf[pos];
		break;
	}
	// 获得质量码
	if (qbuf != 0 && qlen > 0) {
		if (qlen == 1)
			iochannel.qbuf = (ACE_UINT16)qbuf[pos];
		else if (qlen == 2)
			iochannel.qbuf = *(ACE_UINT16 *)(qbuf+pos*2);
	}
	conn->release();
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] iochannel 待设置的标签点信息
 * @retval NIC_SUCCESS 查找成功
 * @retval ERROR_DPU_OUT_RANGE DPU序号超出范围
 * @retval ERROR_IO_OUT_RANGE IO板卡地址超出范围
 * @retval ERROR_CHANNEL_OUT_RANGE IO通道序号超出范围
 * @note 设置标签点的实时值(数据库)
 */
long set_value_library(IO_T & iochannel)
{
	long iocard_num, ret;
	// 检查IO状态
	if ((ret = check_io_status(iochannel, iocard_num)) != NIC_SUCCESS)
		return ret;
	Connect * conn = &conns[iochannel.dpu];
	// 获得IO逻辑地址和类型
	conn->acquire();
	ACE_UINT8 ioaddr = conn->all_data_.io_cards[iocard_num].ioaddr;
	ACE_UINT8 type = conn->all_data_.io_cards[iocard_num].type;
	conn->release();
	// 根据板卡的type发送
	ACE_UINT8 tempData[16], groupId = 0, ptId = 0, pos = 0;
	switch (type) {
	case DO_8_1:
	case DO_16_1:
		groupId = iochannel.channel - iochannel.startchannel;
		pos = iochannel.subchannel;
		ptId = 0x1 << pos;
		tempData[0] = (iochannel.databuf.b ? 0xFF : 0x00);
		ret = conn->send_control_do(ioaddr, groupId, ptId, tempData[0]);
		return ret;
	case AO_8_1:
		groupId = (iochannel.channel - iochannel.startchannel)/16;
		pos = ((iochannel.channel - iochannel.startchannel) % 16) / 2;
		ptId = 0x1 << pos;
		tempData[pos * 2] = HIGH_8_BIT(iochannel.databuf.us);
		tempData[pos * 2 + 1] = LOW_8_BIT(iochannel.databuf.us);
		ret = conn->send_control_ao(ioaddr, groupId, ptId, UNSIGNED_INT_2_BYTE, tempData, 16);
		return ret;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] iochannel 待设置的标签点信息
 * @retval NIC_SUCCESS 查找成功
 * @retval ERROR_DPU_OUT_RANGE DPU序号超出范围
 * @retval ERROR_IO_OUT_RANGE IO板卡地址超出范围
 * @retval ERROR_CHANNEL_OUT_RANGE IO通道序号超出范围
 * @note 设置标签点的实时值(IO卡)
 */
long set_value_iocard(IO_T & iochannel)
{
	long iocard_num, ret;
	// 检查IO状态
	if ((ret = check_io_status(iochannel, iocard_num)) != NIC_SUCCESS)
		return ret;
	Connect * conn = &conns[iochannel.dpu];
	// 获得IO逻辑地址和类型
	conn->acquire();
	ACE_UINT8 ioaddr = conn->all_data_.io_cards[iocard_num].ioaddr;
	ACE_UINT8 type = conn->all_data_.io_cards[iocard_num].type;
	conn->release();
	// 根据板卡的type发送
	ACE_UINT8 channels = 0;
	switch (type) {
	case DO_8_1:
	case AO_8_1:
		channels = 8;
		break;
	case DO_16_1:
		channels = 16;
		break;
	}
	ACE_UINT8 * tempData = new ACE_UINT8[2 * channels];
	ACE_OS::memset((void *)tempData, 0, 2 * channels);
	long pos = 0;
	switch (type) {
	case DO_8_1:
	case DO_16_1:
		pos = (iochannel.channel - iochannel.startchannel) * 8 + iochannel.subchannel;
		tempData[pos * 2] = (iochannel.databuf.b ? 1 : 0);
		tempData[pos * 2 + 1] = 0x80;
		break;
	case AO_8_1:
		pos = iochannel.channel - iochannel.startchannel;
		tempData[pos] = LOW_8_BIT(iochannel.databuf.us);
		tempData[pos + 1] = HIGH_8_BIT(iochannel.databuf.us) | 0x80;
		break;
	}
	if ((ret = conn->send_io_transparent_data_down(ioaddr, type, H52_HMI_ADDR1, H52_DATA_DOWN_TO_SLAVE, H52_PRIORITY_EMERGENCY, H52_CMD_WRITE, 0, tempData)) != NIC_SUCCESS)
		return ret;
	delete [] tempData;
	return NIC_SUCCESS;
}