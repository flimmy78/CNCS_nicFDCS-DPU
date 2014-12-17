/**
 * @file
 * @brief 连接
 * @author Xiaosong Gao
 * @note 包含了对控制器的连接、重连、断开连接、主备切换等操作
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @param [in] host1 主机IP地址
 * @param [in] host2 备机IP地址
 * @param [in] thread 是否创建线程
 * - true：自动创建后台线程(默认)
 * - false：不创建后台线程，需要手动调用open函数
 * @retval NIC_SUCCESS 连接成功
 * @retval ERROR_CONNECT_FAILED 连接失败
 * @retval ERROR_CREATE_THREAD 建立后台线程失败
 * @retval ERROR_CREATE_LINK 建立链路连接失败
 * @note 连接控制器，不启动后台线程
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::connect(char * host1, char * host2, bool thread /* = true */)
{
	addr_[0].set(SERV_PORT, host1);
	addr_[1].set(SERV_PORT, host2);
	// 连接到控制器
	long ret = connect_to_dpu();
	if (ret != NIC_SUCCESS)
		return ret;
	// 发送链路建立请求包
	long count = 0;
	send_start_link_request();
	while (!linked_) {
		recv();		// 接收链路建立确认包
		if ((++count) > 5)
			return ERROR_CREATE_LINK;
	}
	if (thread)		// 如果要自动创建
		return open(0);	// 创建后台线程
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @retval NIC_SUCCESS 断开连接成功
 * @retval ERROR_STOP_THREAD 停止线程失败
 * @note 断开与控制器的连接
 */
long Connect::disconnect()
{
	// 线程未停止，且当前连接有效
	if (!stop_thread_ && linked_) {
		stop_thread_ = true;	// 停止线程标志
		ACE_Time_Value tv(3);
		if (ACE_Thread_Manager::instance()->wait(&tv, false, false) == -1)	 {		// 等待线程终止
			ACE_ERROR((LM_ERROR, "(%P|%t) %p\n", "thread_manager"));
			return ERROR_STOP_THREAD;
		}
	}
	// 发送断开链路连接请求的数据包
	if (linked_)
		send_stop_link_request();
	// 断开连接器连接
	peer_.close();
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] thread 是否创建线程
 * - true：自动创建后台线程(默认)
 * - false：不创建后台线程，需要手动调用open函数
 * @retval NIC_SUCCESS 连接成功
 * @retval ERROR_CONNECT_FAILED 连接失败
 * @note 重新连接到控制器，并启用后台线程，不切换主备
 * @see Connect::disconnect
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::reconnect(bool thread /* = true */)
{
	long current_dpu = current_host_;
	long ret = connect_to_dpu(current_dpu, true);	// 重新连接到控制器，只连接这一个
	if (ret == ERROR_CONNECT_FAILED)
		return ret;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] thread 是否创建线程
 * - true：自动创建后台线程(默认)
 * - false：不创建后台线程，需要手动调用open函数
 * @retval NIC_SUCCESS 切换成功
 * @retval ERROR_CONNECT_FAILED 切换失败
 * @note 进行主备机切换
 * @see Connect::disconnect
 * @see Connect::connect_to_dpu
 * @see Connect::open
 */
long Connect::change_connect(bool thread /* = true */)
{
	long current_dpu = current_host_;
	long ret = connect_to_dpu(1-current_dpu, true);	// 重新连接到另外一个控制器，只连接这一个
	if (ret == ERROR_CONNECT_FAILED)
		return ret;
	return NIC_SUCCESS;
}

/**
 * @author Xiaosong Gao
 * @param [in] priority 优先连接哪一个控制器
 * - 0：表示主控制器(默认)
 * - 1：表示备控制器
 * @param [in] connect_only 是否只连接一个控制器
 * - true：表示只连接priority对应的控制器
 * - false：表示主备控制器都尝试连接(默认)
 * @retval NIC_SUCCESS 连接成功
 * @retval ERROR_CONNECT_FAILED 连接失败
 * @note 连接到控制器，可以选择优先顺序以及尝试连接主/备/全部
 */
long Connect::connect_to_dpu(long priority /* = 0 */, bool connect_only /* = false */)
{
	// 根据priority依次尝试
	if (connector_.connect(peer_, addr_[priority]) == -1) {
		if (!connect_only) {		// 如果不是只连一个控制器
			ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", priority));
			if (connector_.connect(peer_, addr_[1-priority]) == -1) {
				ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", 1-priority));
				return ERROR_CONNECT_FAILED;
			} else {
				current_host_ = 1-priority;
				ACE_DEBUG((LM_DEBUG, "Connect to host %s.\n", addr_[1-priority].get_host_addr()));
			}
		} else {						// 如果只连接一个控制器
			ACE_ERROR((LM_ERROR, "(%P|%t) connect host%d error\n", priority));
			return ERROR_CONNECT_FAILED;
		}
	} else {
		current_host_ = priority;
		ACE_DEBUG((LM_DEBUG, "Connect to host %s.\n", addr_[priority].get_host_addr()));
	}
	return NIC_SUCCESS;
}