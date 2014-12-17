/**
 * @file
 * @brief 构造
 * @author Xiaosong Gao
 * @note Connect类的构造和析构函数
 */

#include "precomp.h"

/**
 * @author Xiaosong Gao
 * @note 默认构造函数
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
 * @note 析构函数
 * @see Connect::isValid
 * @see Connect::disconnect
 */
Connect::~Connect()
{
	if (isValid())			// 如果依旧有连接存在
		disconnect();		// 则进行断开连接操作
}