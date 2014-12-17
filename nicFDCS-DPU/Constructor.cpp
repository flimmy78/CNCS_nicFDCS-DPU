/**
 * @file
 * @brief ����
 * @author Xiaosong Gao
 * @note Connect��Ĺ������������
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @note Ĭ�Ϲ��캯��
 */
Connect::Connect()
{
	enabled_ = true;
	linked_ = false;
	stop_thread_ = false;
	current_host_ = -1;
	ACE_OS::memset((void*)&all_data_, 0, sizeof(IO_DATA));
}

/**
 * @author Xiaosong Gao
 * @note ��������
 * @see Connect::isValid
 * @see Connect::disconnect
 */
Connect::~Connect()
{
	if (isValid())			// ������������Ӵ���
		disconnect();		// ����жϿ����Ӳ���
}