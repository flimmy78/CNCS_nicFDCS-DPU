/**
 * @file
 * @brief 任务
 * @author Xiaosong Gao
 * @note 任务的开始、运行、结束等接口
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] p 参数指针，写成0即可
 * @retval NIC_SUCCESS 启动线程成功
 * @retval ERROR_CREATE_THREAD 启动线程失败
 * @note 创建后台线程，执行监听
 */
int Connect::open(void * p)
{
	stop_thread_ = false;
	int ret = activate();		// 建立后台线程
	if (ret == -1) {			// 创建线程失败
		return ERROR_CREATE_THREAD;
	}
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS 线程运行成功结束
 * @note 线程入口点
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