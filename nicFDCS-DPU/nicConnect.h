/**
 * @file
 * @brief 连接类
 * @author Xiaosong Gao
 * @note 连接到控制器的类接口
 */

#ifndef __NICCONNECT_H__
#define __NICCONNECT_H__

#include "nicFDCS-DPU_type.h"
#include "nicFDCS-DPU_const.h"
#include <vector>
#include <ace/Basic_Types.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Connector.h>
#include <ace/Time_Value.h>
#include <ace/Thread_Mutex.h>
#include <ace/Task.h>

using namespace std;

/** 导出符号 */
#ifdef NICFDCSDPU_EXPORTS
#define NICFDCSDPU_API __declspec(dllexport)
#else
#define NICFDCSDPU_API __declspec(dllimport)
#endif

#define NO_CRC_SEND					/**< 发送过程中不发送CRC信息 */
#define MAX_SIZE_BUF 4096			/**< 单个数据包最大长度 */
#define MAX_SIZE_RECV_BUF 4096*10	/**< 接收缓冲区最大长度 */
#define SERV_PORT 2008				/**< 服务端DPU的端口号 */
#define DEFAULT_TIMEOUT 500*1000	/**< 接收数据的默认超时时间为30*1000us，即30ms */
#define DEFAULT_T1 3				/**< 默认T1心跳超时时间为3s */
#define DEFAULT_PERIOD_TIME 100		/**< 默认对控制器设定的定时发送全数据时间 */
#define MAX_DPU_NUM 100				/**< DPU逻辑地址上限，实际DPU逻辑地址为1~MAX_DPU_NUM-1 */

#define LOW_8_BIT(x) (char)(x & 0xFF)			/**< 获得一个数的低8bit位 */
#define HIGH_8_BIT(x) (char)((x & 0xFF00) >> 8)	/**< 获得一个16位数的高8bit位 */

/** @brief 初始化服务 */
NICFDCSDPU_API void init_service();
/** @brief 获得标签实时值 */
NICFDCSDPU_API long get_value(IO_T &);
/** @brief 设置标签实时值(数据库) */
NICFDCSDPU_API long set_value_library(IO_T &);
/** @brief 设置标签实时值(IO卡) */
NICFDCSDPU_API long set_value_iocard(IO_T &);

/**
 * @class Connect
 * @brief 连接
 * @author Xiaosong Gao
 * @note 上位机到控制器的单个连接类，提供发送/接收等接口
 */
class NICFDCSDPU_API Connect : public ACE_Task<ACE_MT_SYNCH>
{
public:
	/**
	 * @name 构造
	 * @{
	 */
	Connect();	/**< @brief 构造 */
	~Connect();	/**< @brief 析构 */
	/** @} */

	/**
	 * @name 连接
	 * @{
	 */
	long connect(char *, char *, bool thread = true);		/**< @brief 连接 */
	long disconnect();										/**< @brief 断开连接 */
	long reconnect(bool thread = true);						/**< @brief 重连 */
	long change_connect(bool thread = true);				/**< @brief 主备切换 */
	long connect_to_dpu(long priority = 0, bool connect_only = false);		/**< @brief 连接到控制器 */
	/** @} */

	/**
	 * @name 发送
	 * @{
	 */
	void generate_packet(FD_PACKET &, long, char *, long &);							/**< @brief 生成数据包 */
	long send(char * buf, long len);													/**< @brief 发送一条消息 */
	long send_start_link_request();														/**< @brief START建立链路请求 */
	long send_start_t1_request(ACE_UINT8);												/**< @brief START建立T1请求 */
	long send_stop_link_request();														/**< @brief STOP建立链路请求 */
	long send_call_io_data(char, long);													/**< @brief IO数据召唤=11H */
	long send_control_ao(ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8 *, ACE_UINT8);		/**< @brief 下行控制AO=33H */
	long send_control_do(ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8);					/**< @brief 下行控制DO=34H */
	long send_io_transparent_data_down(ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8, ACE_UINT8 *);		/**< @brief IO透明数据下行=52H */
	long send_control_vpt(ACE_UINT8 ioaddr, IO_T & iot);								/**< @brief 设置VPT=3AH */
	/** @} */

	/**
	 * @name 接收
	 * @{
	 */
	void debug_all_data();								/**< @brief 输出all_data_ */
	void debug_packet_content(char *, int);				/**< @brief 输出包内容 */
	long recv();										/**< @brief 接收一条消息 */
	long recv_start_link_ack(FD_PACKET &);				/**< @brief START链路建立确认 */
	long recv_stop_link_ack(FD_PACKET &);				/**< @brief STOP链路停止确认 */
	long recv_start_t1_ack(FD_PACKET &);				/**< @brief T1链路建立确认 */
	long recv_answer_io_data(FD_PACKET &);				/**< @brief IO数据应答=21H */
	long recv_operation_failed_message(FD_PACKET &);	/**< @brief 错误报文=15H */
	/** @} */

	/**
	 * @name 任务
	 * @{
	 */
	int open(void *);		/**< @brief 创建线程 */
	int svc();				/**< @brief 线程运行 */
	/** @} */

	/**
	 * @name 参数
	 * @{
	 */
	long setDestAddr(ACE_UINT8 addr);	/**< @brief 设置目的逻辑地址 */
	inline bool isEnabled() const;		/**< @brief 查看启用/禁用 */
	inline bool isValid() const;		/**< @brief 查看连接是否有效 */
	void setEnabled(bool);				/**< @brief 设置有效性 */
	/** @} */

	/**
	 * @name 互斥锁
	 * @{ 
	 */
	inline int acquire();		/**< @brief 获得网络缓冲区的锁 */
	inline int release();		/**< @brief 释放网络缓冲区的锁 */
	/** @} */

	static long setSourceAddr(ACE_UINT8 addr);	/**< @brief 设置源逻辑地址 */
	static void setTimeout(long);				/**< @brief 设置接收超时 */
	static void setT1(ACE_UINT8);				/**< @brief 设置T1超时 */
	static void setPeriodTime(long);			/**< @brief 设置控制器发送周期 */

	/**
	 * @name 连接参数
	 * @{
	 */
	ACE_Thread_Mutex net_mutex_;	/**< data_buf_的读写锁 */
	ACE_INET_Addr addr_[2];			/**< 记录下主(0)、备(1)机地址，便于查找 */
	ACE_SOCK_Stream peer_;			/**< socket连接 */
	ACE_SOCK_Connector connector_;	/**< 连接器 */
	long current_host_;				/**< 当前连接的主机序号，0为主机，1为备机 */
	bool stop_thread_;				/**< 是否停止线程，true表示立刻停止，false表示线程将一直运行 */
	bool enabled_;					/**< 当前连接是否有效 */
	bool linked_;					/**< 是否已经建立了数据链路 */
	static ACE_Time_Value timeout_;	/**< 接收数据的超时时间，单位是ms(毫秒) */
	static ACE_UINT8 t1_;			/**< T1连接，在DPU中建立T1为超时时间。范围为2-16(秒) */
	static long period_time_;		/**< 定时接收控制器信息的周期，单位是ms */
	/** @} */

	/**
	 * @name 数据参数
	 * @{
	 */
	static ACE_UINT8 source_addr_;		/**< 协议头里面的源地址一项，100-131 */
	ACE_UINT8 dest_addr_;				/**< 协议头里面的目的地址一项，1-99 */
	IO_DATA all_data_;					/**< 定时从控制器接收到的全数据召唤返回值 */
	TIME_MARK time_mark_;				/**< 定时从控制器接收到的返回值的时标 */
	char recv_buf_[MAX_SIZE_RECV_BUF];	/**< 接收的数据缓冲区 */
	long recv_len_;						/**< recv_buf_的数据长度 */
	char data_buf_[MAX_SIZE_BUF];		/**< all_data_对应的数据缓冲区 */
	long data_len_;						/**< data_buf_的数据长度 */
	char * recv_read_ptr_;				/**< 正在解析的数据 */
	/** @} */
};

/** @brief 全局的连接集合 */
NICFDCSDPU_API extern Connect conns[MAX_DPU_NUM];

#endif	// __NICCONNECT_H__