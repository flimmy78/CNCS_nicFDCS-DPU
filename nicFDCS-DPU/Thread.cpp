/**
 * @file
 * @brief ����
 * @author Xiaosong Gao
 * @note ����Ŀ�ʼ�����С������Ƚӿ�
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] p ����ָ�룬д��0����
 * @retval NIC_SUCCESS �����̳߳ɹ�
 * @retval ERROR_CREATE_THREAD �����߳�ʧ��
 * @note ������̨�̣߳�ִ�м���
 */
int Connect::open(void * p)
{
	stop_thread_ = false;
	int ret = activate();		// ������̨�߳�
	if (ret == -1) {			// �����߳�ʧ��
		return ERROR_CREATE_THREAD;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS �߳����гɹ�����
 * @note �߳���ڵ�
 * @see Connect::send
 * @see Connect::recv
 * @see Connect::isValid
 */
int Connect::svc()
{
	ACE_Time_Value t1, t2;
	send_start_t1_request(t1_);
	t1 = ACE_OS::gettimeofday();
	send_call_io_data(H11_DATA_SUBSCRIPTION, Connect::period_time_);
	long count;
	count = 0;
	while (!stop_thread_ && isValid()) {
		long ret = recv();
		if (ret != NIC_SUCCESS) {
		}
		t2 = ACE_OS::gettimeofday();
		if ((t2-t1).msec() > (ACE_UINT32)(t1_ * 1000 / 2)) {
			send_start_t1_request(t1_);
			t1 = t2;
		}
	}
	stop_thread_ = true;
	return NIC_SUCCESS;
}