/**
 * @file
 * @brief ���ò���
 * @author Xiaosong Gao
 * @note ���ò����ͻ�ò����Ľӿ�
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] addr Դ��ַ��������100-131֮��
 * @retval NIC_SUCCESS ���óɹ�
 * @retval ERROR_INVALID_SRC ��Ч��Դ��ַ
 * @note ����Դ�߼���ַ
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
 * @param [in] addr Ŀ�ĵ�ַ��������1-99֮��
 * @retval NIC_SUCCESS ���óɹ�
 * @retval ERROR_INVALID_TARGET ��Ч��Ŀ�ĵ�ַ
 * @note ����Ŀ���߼���ַ
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
 * @retval true ��ǰ����������
 * @retval false ��ǰ�����ѽ��ã��ȼ���ɾ��
 * @note �鿴��ǰ��������/����
 */
bool Connect::isEnabled() const
{
	return enabled_;
}

/**
 * @author Xiaosong Gao
 * @retval true ��ǰ������Ч
 * @retval false ��ǰ������Ч
 * @note �鿴��ǰ�����Ƿ���Ч����Ч����Ҫ�ֶ���������
 */
bool Connect::isValid() const
{
	return enabled_ && linked_;
}

/**
 * @author Xiaosong Gao
 * @param [in] enabled ����/����
 * - true������
 * - false������
 * @note ������Ч��
 */
void Connect::setEnabled(bool enabled)
{
	if (!enabled)
		disconnect();
	enabled_ = enabled;
}

/**
 * @author Xiaosong Gao
 * @param [in] timeout �������ݳ�ʱʱ�䣬��λ��΢��
 * @note ���ý�������ʱ�ĳ�ʱʱ��
 */
void Connect::setTimeout(long timeout)
{
	Connect::timeout_.set(0, timeout);
}

/**
 * @author Xiaosong Gao
 * @param [in] t1 ��ʱʱ��
 * @note ����T1���ӵĳ�ʱʱ��
 */
void Connect::setT1(ACE_UINT8 t1)
{
	Connect::t1_ = t1;
}

/**
 * @author Xiaosong Gao
 * @param [in] periodtime ����ʱ��
 * @note ���ÿ��������ĵķ�������
 */
void Connect::setPeriodTime(long periodtime)
{
	Connect::period_time_ = periodtime;
}

/**
 * @author Xiaosong Gao
 * @retval value valuedescription
 * @note ������绺��������
 */
int Connect::acquire()
{
	return net_mutex_.acquire();
}

/**
 * @author Xiaosong Gao
 * @retval value valuedescription
 * @note �ͷ����绺��������
 */
int Connect::release()
{
	return net_mutex_.release();
}

/**
 * @author Xiaosong Gao
 * @param [in] buf ���ݰ�����
 * @param [in] len ���ݰ�����
 * @note ������ݰ������ݣ�ʮ�����Ƹ�ʽ
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
 * @note ������յ���all_data_��ֵ
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