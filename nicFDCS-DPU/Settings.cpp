/**
 * @file
 * @brief 设置参数
 * @author Xiaosong Gao
 * @note 设置参数和获得参数的接口
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] addr 源地址，必须在100-131之间
 * @retval NIC_SUCCESS 设置成功
 * @retval ERROR_INVALID_SRC 无效的源地址
 * @note 设置源逻辑地址
 */
long Connect::setSourceAddr(ACE_UINT8 addr)
{
	if (addr < 100 || addr > 131) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Source Addr is between 100 and 131.\n"));
		return ERROR_INVALID_SRC;
	}
	Connect::source_addr_ = addr;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] addr 目的地址，必须在1-99之间
 * @retval NIC_SUCCESS 设置成功
 * @retval ERROR_INVALID_TARGET 无效的目的地址
 * @note 设置目的逻辑地址
 */
long Connect::setDestAddr(ACE_UINT8 addr)
{
	if (addr < 1 || addr > 99) {
		ACE_ERROR((LM_ERROR, "(%P|%t) Dest Addr is between 1 and 99.\n"));
		return ERROR_INVALID_TARGET;
	}
	dest_addr_ = addr;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @retval true 当前连接已启动
 * @retval false 当前连接已禁用，等价于删除
 * @note 查看当前连接启用/禁用
 */
bool Connect::isEnabled() const
{
	return enabled_;
}

/**
 * @author Xiaosong Gao
 * @retval true 当前连接有效
 * @retval false 当前连接无效
 * @note 查看当前连接是否有效，无效则需要手动进行重连
 */
bool Connect::isValid() const
{
	return enabled_ && linked_;
}

/**
 * @author Xiaosong Gao
 * @param [in] enabled 启用/禁用
 * - true：启动
 * - false：禁用
 * @note 设置有效性
 */
void Connect::setEnabled(bool enabled)
{
	if (!enabled)
		disconnect();
	enabled_ = enabled;
}

/**
 * @author Xiaosong Gao
 * @param [in] timeout 接收数据超时时间，单位是微秒
 * @note 设置接收数据时的超时时间
 */
void Connect::setTimeout(long timeout)
{
	Connect::timeout_.set(0, timeout);
}

/**
 * @author Xiaosong Gao
 * @param [in] t1 超时时间
 * @note 设置T1连接的超时时间
 */
void Connect::setT1(ACE_UINT8 t1)
{
	Connect::t1_ = t1;
}

/**
 * @author Xiaosong Gao
 * @param [in] periodtime 周期时间
 * @note 设置控制器订阅的发送周期
 */
void Connect::setPeriodTime(long periodtime)
{
	Connect::period_time_ = periodtime;
}

/**
 * @author Xiaosong Gao
 * @retval value valuedescription
 * @note 获得网络缓冲区的锁
 */
int Connect::acquire()
{
	return net_mutex_.acquire();
}

/**
 * @author Xiaosong Gao
 * @retval value valuedescription
 * @note 释放网络缓冲区的锁
 */
int Connect::release()
{
	return net_mutex_.release();
}

/**
 * @author Xiaosong Gao
 * @param [in] buf 数据包内容
 * @param [in] len 数据包长度
 * @note 输出数据包的内容，十六进制格式
 */
void Connect::debug_packet_content(char * buf, int len)
{
	ACE_DEBUG((LM_DEBUG, "Packet content : "));
	for (long i = 0; i < len; i++) {
		if ((ACE_UINT8)buf[i]<16)
			ACE_DEBUG((LM_DEBUG, "0"));
		ACE_DEBUG((LM_DEBUG, "%x ", (ACE_UINT8)buf[i]));
	}
	ACE_DEBUG((LM_DEBUG, "\n"));
}

/**
 * @author Xiaosong Gao
 * @note 输出接收到的all_data_的值
 */
void Connect::debug_all_data()
{
	// DPU_ATTR
	ACE_DEBUG((LM_DEBUG, "DPU_ATTR:\n"));
	ACE_DEBUG((LM_DEBUG, "\tdpuAddr = %d\n", all_data_.dpu_attr.dpuAddr));
	ACE_DEBUG((LM_DEBUG, "\tdpuStatus = 0x %4x\n", all_data_.dpu_attr.dpuStatus));
	ACE_DEBUG((LM_DEBUG, "\tdpuType = 0x %2x\n", all_data_.dpu_attr.dpuType));
	ACE_DEBUG((LM_DEBUG, "\treal_io_num = %d\n", all_data_.dpu_attr.real_io_num));
	ACE_DEBUG((LM_DEBUG, "\tvir_io_num = %d\n", all_data_.dpu_attr.vir_io_num));
	// io_num
	ACE_DEBUG((LM_DEBUG, "io_num:\n"));
	ACE_DEBUG((LM_DEBUG, "\tio_num = %d\n", all_data_.io_num));
	for (long i = 0; i < all_data_.io_num; i++) {
		// ATTR
		ACE_DEBUG((LM_DEBUG, "IO_ATTR %d:\n", i));
		ACE_DEBUG((LM_DEBUG, "\tioid = %d\n", all_data_.io_cards[i].ioid));
		ACE_DEBUG((LM_DEBUG, "\tioaddr = %d\n", all_data_.io_cards[i].ioaddr));
		ACE_DEBUG((LM_DEBUG, "\tstatus = 0x %2x\n", all_data_.io_cards[i].status));
		ACE_DEBUG((LM_DEBUG, "\ttype = 0x %2x\n", all_data_.io_cards[i].type));
		// VALUE
		ACE_DEBUG((LM_DEBUG, "IO_VALUE %d:\n", i));
		ACE_DEBUG((LM_DEBUG, "\tqlen = %d\n", all_data_.io_cards[i].qlen));
		ACE_DEBUG((LM_DEBUG, "\tquality = 0x "));
		for (long j = 0; j < all_data_.io_cards[i].qlen; j++) ACE_DEBUG((LM_DEBUG, "%2x ", all_data_.io_cards[i].quality[j]));
		ACE_DEBUG((LM_DEBUG, "\n"));
		ACE_DEBUG((LM_DEBUG, "\tvlen = %d\n", all_data_.io_cards[i].vlen));
		ACE_DEBUG((LM_DEBUG, "\tvvalue = 0x "));
		for (long j = 0; j < all_data_.io_cards[i].vlen; j++) ACE_DEBUG((LM_DEBUG, "%2x ", all_data_.io_cards[i].vvalue[j]));
		ACE_DEBUG((LM_DEBUG, "\n"));
	}
	ACE_DEBUG((LM_DEBUG, "\n"));
}