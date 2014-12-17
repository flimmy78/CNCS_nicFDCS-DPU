/**
* @file
* @brief ����
* @author Xiaosong Gao
* @note �������н������ݰ��Ĳ���
*/

#include "precomp.h"

/** �������ݵĳ�ʱʱ�� */
ACE_Time_Value Connect::timeout_(0, DEFAULT_TIMEOUT);

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS ���ճɹ�
 * @retval ERROR_RECV_TIMEOUT ���ճ�ʱ
 * @retval ERROR_IS_ENABLED �豸û�б�����
 * @retval ERROR_UNKNOW_SFDCS ��ͷ����0x68H
 * @note ����һ����Ϣ������data_buf_�У�����Ϊdata_len_��������Э��ͷ��������Ӧ�Ĵ�����
 */
long Connect::recv()
{
	// �ж��豸/�����Ƿ���Ч
	if (!isEnabled())
		return ERROR_IS_ENABLED;
	// �������ݣ��浽data_buf_��data_len_��
	recv_len_ = peer_.recv(recv_buf_, MAX_SIZE_RECV_BUF, &timeout_);
	if (recv_len_ == -1) {
		ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "recv"));
		return ERROR_RECV_TIMEOUT;
	}
	recv_buf_[recv_len_] = 0;
	// �������ݰ�ͷ����������Ӧ�ĺ���
	recv_read_ptr_ = recv_buf_;
	while (*recv_read_ptr_ == SYNC_HEADER) {
		FD_PACKET packet;
		ACE_OS::memcpy(&packet, recv_read_ptr_, 9);
		recv_read_ptr_ += 9;
		switch (packet.control_area_1 & FRAME_FORMAT_MASK) {	// ���ݿ�����1����λ�ж�֡�ĸ�ʽ
		case GENERAL_DATA_FRAME:	// һ������֡
			packet.info_code = *recv_read_ptr_;
			packet.description_code = *(recv_read_ptr_+1);
			switch (packet.info_code) {
			case ANSWER_FDCS_WHOLE_SYSTEM_OBSERVATION_POINT_DATA:		// IO����Ӧ��=0x21H
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
		case T1_TIMEOUT_FRAME:		// T1����֡
			if (packet.control_area_1 & T1_ACK_MASK)	// T1����ȷ�����ݰ�
				recv_start_t1_ack(packet);
			break;
		case T2_TIMEOUT_FRAME:		// T2����֡
			break;
		case LINK_FRAME:			// ��·�����ź�
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
 * @param [in] packet ���յ������ݰ�(�Ѿ�������Э��ͷ)
 * @retval NIC_SUCCESS ����ɹ�
 * @note START��·����ȷ�ϣ�DPU����ȷ���ź�
 */
long Connect::recv_start_link_ack(FD_PACKET & packet)
{
	linked_ = true;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet ���յ������ݰ�(�Ѿ�������Э��ͷ)
 * @retval NIC_SUCCESS ����ɹ�
 * @note STOP��·ֹͣȷ�ϣ�DPU����ȷ���ź�
 */
long Connect::recv_stop_link_ack(FD_PACKET & packet)
{
	linked_ = false;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet ���յ������ݰ�(�Ѿ�������Э��ͷ)
 * @retval NIC_SUCCESS ����ɹ�
 * @note T1��·����ȷ�ϣ�DPU����ȷ���ź�
 */
long Connect::recv_start_t1_ack(FD_PACKET & packet)
{
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet ���յ������ݰ�(�Ѿ�������Э��ͷ)
 * @retval NIC_SUCCESS ����ɹ�
 * @note ���������
 */
long Connect::recv_operation_failed_message(FD_PACKET & packet)
{
	switch (packet.description_code) {
	case ERROR_UNKNOW_SFDCS: ACE_DEBUG((LM_DEBUG, "��ͷ����0x68H")); break;
	case ERROR_NOBUILDLINK: ACE_DEBUG((LM_DEBUG, "û�н����������ӣ��ͷ����������������������")); break;
	case ERROR_SUBSC_INVALID: ACE_DEBUG((LM_DEBUG, "���붩����ʱ��ʧ��")); break;
	case ERROR_ALREADYHASTIMER: ACE_DEBUG((LM_DEBUG, "����")); break;
	case ERROR_TIMEROVERFLOW: ACE_DEBUG((LM_DEBUG, "����Ķ�ʱ��С��1")); break;
	case ERROR_HASNOTIMER: ACE_DEBUG((LM_DEBUG, "Ŀǰ����")); break;
	case ERROR_ALREADYLINK: ACE_DEBUG((LM_DEBUG, "�Ѿ����������ӣ���������������")); break;
	case ERROR_INVALIDPACKET: ACE_DEBUG((LM_DEBUG, "��ͷ����0x68H")); break;
	case ERROR_LENTOOSHORT: ACE_DEBUG((LM_DEBUG, "��ͷ�г���С�ڵ���0")); break;
	case ERROR_IODEAD: ACE_DEBUG((LM_DEBUG, "���ʵ�IO����")); break;
	case ERROR_IOADDR: ACE_DEBUG((LM_DEBUG, "IO��ַ��")); break;
	case ERROR_UNKNOWNDATATYPE: ACE_DEBUG((LM_DEBUG, "�������ݵ��������ͣ�DPUû�ж���")); break;
	case ERROR_MAKEFRAME: ACE_DEBUG((LM_DEBUG, "���������˴���")); break;
	case ERROR_NOVALIDIO: ACE_DEBUG((LM_DEBUG, "����������IO��ַ���Ҳ���")); break;
	case ERROR_INVALID_TARGET: ACE_DEBUG((LM_DEBUG, "DPU��ַ�����϶���")); break;
	case ERROR_INVALID_SRC: ACE_DEBUG((LM_DEBUG, "HMI�ڵ��ַ�����϶���")); break;
	case ERROR_REGISTER_DEV: ACE_DEBUG((LM_DEBUG, "ע��ͨ���豸��ַ����")); break;
	case ERROR_TARGET_NOTCORRECT: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_DPU_STATUS_NORUN: ACE_DEBUG((LM_DEBUG, "DPU״̬��������̬")); break;
	case ERROR_DEVNO_INVALID: ACE_DEBUG((LM_DEBUG, "�ڽڵ����豸�����Ч")); break;
	case ERROR_NODE_OVERLIMIT: ACE_DEBUG((LM_DEBUG, "����DPU��IO����Ŀ̫�࣬����������")); break;
	case ERROR_NO_PROTOCOL: ACE_DEBUG((LM_DEBUG, "Э��û�г�ʼ��")); break;
	case ERROR_NODE_HASNOADDR: ACE_DEBUG((LM_DEBUG, "�����г�������Ч��DPU��IO��ַ")); break;
	case ERROR_NODE_INVALID: ACE_DEBUG((LM_DEBUG, "������Ч�ĵ�ַ")); break;
	case SYS_MEM_ALLOC_ERR: ACE_DEBUG((LM_DEBUG, "ϵͳ�����ڴ����")); break;
	case SYS_QERR_QFULL: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_RIGHT_NOWRITE: ACE_DEBUG((LM_DEBUG, "û��дȨ��")); break;
	case ERROR_OPEN_FILENAME: ACE_DEBUG((LM_DEBUG, "���ļ���")); break;
	case ERROR_NOEVENT_ITEM: ACE_DEBUG((LM_DEBUG, "���¼����������գ�")); break;
	case ERROR_NOLOG_ITEM: ACE_DEBUG((LM_DEBUG, "��LOG���������գ�")); break;
	case ERROR_OVERLIMIT: ACE_DEBUG((LM_DEBUG, "")); break;
	case ERROR_NO_LBDATA: ACE_DEBUG((LM_DEBUG, "û��¼���ļ�")); break;
	case ERROR_SETTING_INVALID: ACE_DEBUG((LM_DEBUG, "��ֵ����Ч��ָ��ȡNVRAMʧ�ܡ���ֵ����Ч��")); break;
	case ERROR_SETTING_AREA: ACE_DEBUG((LM_DEBUG, "��ֵ���Ŵ�")); break;
	case ERROR_SETTING_NUM: ACE_DEBUG((LM_DEBUG, "��ֵ������")); break;
	case ERROR_SETTING_OVERFLOW: ACE_DEBUG((LM_DEBUG, "��ֵԽ��")); break;
	case ERROR_SETTING_CRC: ACE_DEBUG((LM_DEBUG, "��ֵcrcУ���")); break;
	case ERROR_SETTING_TYPE: ACE_DEBUG((LM_DEBUG, "��ֵ���ʹ�")); break;
	case ERROR_SETTING_STATE: ACE_DEBUG((LM_DEBUG, "����״̬��")); break;
	case ERROR_SETTING_WFILE: ACE_DEBUG((LM_DEBUG, "�������ļ�д��ֵ���Ŵ�")); break;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] packet ���յ������ݰ�(�Ѿ�������Э��ͷ)
 * @retval NIC_SUCCESS ����ɹ�
 * @note IO����Ӧ��=21H�������������ݣ���������Ӧ�Ļ�������
 */
long Connect::recv_answer_io_data(FD_PACKET & packet)
{
	char * p = data_buf_;
	if (packet.description_code & H21_TIME_MARK) {		// ����ʱ����Ϊ1������Ϊ0
		ACE_OS::memcpy(&time_mark_, p, H21_TIME_MARK_LENGTH);		// ����ʱ��
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