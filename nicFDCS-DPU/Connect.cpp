/**
 * @file
 * @brief ����
 * @author Xiaosong Gao
 * @note �����˶Կ����������ӡ��������Ͽ����ӡ������л��Ȳ���
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] host1 ����IP��ַ
 * @param [in] host2 ����IP��ַ
 * @param [in] thread �Ƿ񴴽��߳�
 * - true���Զ�������̨�߳�(Ĭ��)
 * - false����������̨�̣߳���Ҫ�ֶ�����open����
 * @retval NIC_SUCCESS ���ӳɹ�
 * @retval ERROR_CONNECT_FAILED ����ʧ��
 * @retval ERROR_CREATE_THREAD ������̨�߳�ʧ��
 * @retval ERROR_CREATE_LINK ������·����ʧ��
 * @note ���ӿ���������������̨�߳�
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::connect(char * host1, char * host2, bool thread /* = true */)
{
	addr_[0].set(SERV_PORT, host1);
	addr_[1].set(SERV_PORT, host2);
	// ���ӵ�������
	long ret = connect_to_dpu();
	if (ret != NIC_SUCCESS)
		return ret;
	// ������·���������
	long count = 0;
	send_start_link_request();
	while (!linked_) {
		recv();		// ������·����ȷ�ϰ�
		if ((++count) > 5)
			return ERROR_CREATE_LINK;
	}
	if (thread)		// ���Ҫ�Զ�����
		return open(0);	// ������̨�߳�
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS �Ͽ����ӳɹ�
 * @retval ERROR_STOP_THREAD ֹͣ�߳�ʧ��
 * @note �Ͽ��������������
 */
long Connect::disconnect()
{
	// �߳�δֹͣ���ҵ�ǰ������Ч
	if (!stop_thread_ && linked_) {
		stop_thread_ = true;	// ֹͣ�̱߳�־
		ACE_Time_Value tv(3);
		if (ACE_Thread_Manager::instance()->wait(&tv, false, false) == -1)	 {		// �ȴ��߳���ֹ
			ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "thread_manager"));
			return ERROR_STOP_THREAD;
		}
	}
	// ���ͶϿ���·������������ݰ�
	if (linked_)
		send_stop_link_request();
	// �Ͽ�����������
	peer_.close();
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] thread �Ƿ񴴽��߳�
 * - true���Զ�������̨�߳�(Ĭ��)
 * - false����������̨�̣߳���Ҫ�ֶ�����open����
 * @retval NIC_SUCCESS ���ӳɹ�
 * @retval ERROR_CONNECT_FAILED ����ʧ��
 * @note �������ӵ��������������ú�̨�̣߳����л�����
 * @see Connect::disconnect
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::reconnect(bool thread /* = true */)
{
	long current_dpu = current_host_;
	long ret = connect_to_dpu(current_dpu, true);	// �������ӵ���������ֻ������һ��
	if (ret == ERROR_CONNECT_FAILED)
		return ret;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] thread �Ƿ񴴽��߳�
 * - true���Զ�������̨�߳�(Ĭ��)
 * - false����������̨�̣߳���Ҫ�ֶ�����open����
 * @retval NIC_SUCCESS �л��ɹ�
 * @retval ERROR_CONNECT_FAILED �л�ʧ��
 * @note �����������л�
 * @see Connect::disconnect
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::change_connect(bool thread /* = true */)
{
	long current_dpu = current_host_;
	long ret = connect_to_dpu(1-current_dpu, true);	// �������ӵ�����һ����������ֻ������һ��
	if (ret == ERROR_CONNECT_FAILED)
		return ret;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] priority ����������һ��������
 * - 0����ʾ��������(Ĭ��)
 * - 1����ʾ��������
 * @param [in] connect_only �Ƿ�ֻ����һ��������
 * - true����ʾֻ����priority��Ӧ�Ŀ�����
 * - false����ʾ��������������������(Ĭ��)
 * @retval NIC_SUCCESS ���ӳɹ�
 * @retval ERROR_CONNECT_FAILED ����ʧ��
 * @note ���ӵ�������������ѡ������˳���Լ�����������/��/ȫ��
 */
long Connect::connect_to_dpu(long priority /* = 0 */, bool connect_only /* = false */)
{
	// ����priority���γ���
	if (connector_.connect(peer_, addr_[priority]) == -1) {
		if (!connect_only) {		// �������ֻ��һ��������
			ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", priority));
			if (connector_.connect(peer_, addr_[1-priority]) == -1) {
				ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", 1-priority));
				return ERROR_CONNECT_FAILED;
			} else {
				current_host_ = 1-priority;
				ACE_DEBUG((LM_DEBUG, "Connect to host %s.\n", addr_[1-priority].get_host_addr()));
			}
		} else {						// ���ֻ����һ��������
			ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", priority));
			return ERROR_CONNECT_FAILED;
		}
	} else {
		current_host_ = priority;
		ACE_DEBUG((LM_DEBUG, "Connect to host %s.\n", addr_[priority].get_host_addr()));
	}
	return NIC_SUCCESS;
}