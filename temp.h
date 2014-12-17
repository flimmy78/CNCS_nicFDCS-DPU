/** 描述LDPU中下网点DPU */
struct DPU_PEER {
	ACE_UINT8 num;			/**< 需要的DPU数目 */
	ACE_UINT8 * peerAddr;	/**< DPU的逻辑地址，长度为num字节 */
};

/** 描述DPNET的静态信息 */
struct DPNET_ADDR {
	ACE_UINT8 baud_rate;			/**< Bit0:9600; Bit1:115200; Bit2:500K  Bit3:1M; Bit4:2.5M; Bit5:5M; Bit6:Rev; Bit7:Rev */
	ACE_UINT16 thw_max;				/**< 主站等待从站响应的最大时间 */
	ACE_UINT16 thr_min;				/**< 主站发送数据后到允许接收从站回应数据的最小等待时间 */
	ACE_UINT16 tsd_min;				/**< 允许从站发送的从站最小延迟时间 */
	ACE_UINT16 tsd_max;				/**< 允许从站回应主站最大延迟时间 */
	ACE_UINT8 max_retry;			/**< 最大重发次数 */
	ACE_UINT8 operator_flag;		/**< 错误发生后操作有无改变 */
	ACE_UINT16 min_slave_interval;	/**< 扫描周期，最小为100us */
	ACE_UINT16 poll_timeout;		/**< 主站等待从站响应的最大等待时间 */
	ACE_UINT16 data_control_time;	/**< 每一从站最大采样时间，最小为10ms */
	ACE_UINT16 datalen_max;			/**< 最大传输的数据长度 */
	ACE_UINT8 emergency_max;		/**< 最大紧急帧个数 */
	ACE_UINT8 outputdatalen;		/**< 主站一次传输数据最大长度 */
	ACE_UINT8 inputdatalen;			/**< 从站一次传输数据最大长度 */
	ACE_UINT8 * cycle_ref;			/**< 设定每个从站采样周期(为最小采样周期10ms的整数倍)，长度为2*Group_max*Slave_max字节 */
};

/** 描述通信模块的静态信息 */
struct COMM_ATTR {
	ACE_UINT8 gpsserver;			/**< GPS服务器 */
	ACE_UINT8 commtaskused;			/**< 通信任务启动 */
	ACE_UINT8 srvtablesize;			/**< 服务(协议)驱动表大小 */
	ACE_UINT8 drvtablesize;			/**< 通信驱动表大小 */
	ACE_UINT16 gpstimestamp;		/**< GPS对时频率 */
	ACE_UINT16 cnetquicktimestamp;	/**< Cnet快速数据时间频率 */
	ACE_UINT16 cnetlowtimestamp;	/**< Cnet慢速数据时间频率 */
	ACE_UINT8 connectornum;			/**< 主动连接数目 */
	ACE_UINT32 * ip;				/**< 主动连接目标地址，长度为4*connectornum字节 */
};

/** 描述DPU中动态变化的状态信息 */
struct DPU_STATUS {
	ACE_UINT8 temp;							/**< 箱体温度[℃] */
	ACE_UINT8 cpu;							/**< CPU利用率[℅] */
	ACE_UINT8 mem;							/**< MEM使用状况[Mbyte] */
	ACE_UINT8 hardware;						/**< 硬件错 */
	ACE_UINT8 dpbus;						/**< dpBus状态 */
	ACE_UINT8 comm;							/**< Comm状态 */
	ACE_UINT8 heartbeat;					/**< 同步心跳线故障 */
	ACE_UINT8 dpbus_state;					/**< DPBUS主站状态 */
	ACE_UINT8 baud_ratestatus;				/**< DPBUS主站波特率 */
	ACE_UINT32 hostsendcount;				/**< 主站发送的总次数统计 */
	ACE_UINT32 hostrecvcount;				/**< 主站接收的总次数统计 */
	ACE_UINT32 hosterrorrecvcount;			/**< 主站接收的无效或未响应帧次数统计 */
	ACE_UINT32 primaryoverrun_count;		/**< 主通道接收中的OverRun错误次数统计 */
	ACE_UINT32 secondaryoverrun_count;		/**< 备通道接收中的OverRun错误次数统计 */
	ACE_UINT32 primaryparity_count;			/**< 主通道接收中的Parity错误次数统计 */
	ACE_UINT32 secondaryparity_count;		/**< 备通道接收中的Parity错误次数统计 */
	ACE_UINT32 primaryframeclosed_count;	/**< 主通道接收中的Frame Closed错误次数统计 */
	ACE_UINT32 secondaryframeclosed_count;	/**< 备通道接收中的Frame Closed错误次数统计 */
	ACE_UINT16 send_maxtimespend;			/**< CPU发送一帧所耗最大时间长度 */
	ACE_UINT16 recv_maxtimespend;			/**< CPU接收一帧所耗最大时间长度 */
};

/** 描述IO卡的动态属性信息 */
struct IO_DYNA_ATTR {
	ACE_UINT8 ioid;		/**< 物理地址，槽号 */
	ACE_UINT8 ioaddr;	/**< IO板逻辑地址 */
	ACE_UINT8 status;	/**< 初始态0x01; 配置态0x02; 运行态0x11; 死态0x4F; 错误态0x41; 通讯中断0x42; 配置错误0x44 */
	ACE_UINT8 type;		/**< IO模板类型 */
};

/** 描述上送的事件，一共32个字节 */
struct IO_DYNA_EVENT {
	ACE_UINT8 dpuaddr;			/**< DPU地址 */
	ACE_UINT8 ioaddr;			/**< io地址，若是全装置，为FFH */
	ACE_UINT8 time56[7];		/**< 7字节时标 */
	ACE_UINT8 dputype;			/**< DPU类型 */
	ACE_UINT8 eventtype;		/**< 事件类型，分为告警、动作、运行三种，见"事件类型编码" */
	ACE_UINT8 eventcode;		/**< 事件编码，见“详细事件编码”，保留 */
	ACE_UINT8 eventdesp[40];	/**< 40个字节的文本描述事件，若参数不足40个将剩余空间置0，内容描述如EVENT_DESP */
};

/** 存储着DPU的状态，数据区中数据的具体定义见DPU_STATUS */
struct DPU_VDI {
	ACE_UINT8 vdilen;		/**< 数据长度 */
	ACE_UINT8 * vdidata;	/**< 数据区，长度为vdilen字节 */
};

/** 描述软件版本，用字符串表示，如DPU-V1.00-07.04.08，表示dpu软件版本 */
struct DPU_VERSION {
	ACE_UINT8 len;			/**< 版本数据长度 */
	ACE_UINT8 * version;	/**< 版本数据，长度len字节 */
};

/** 描述DPU中相关的静态信息 */
struct DPU_ATTR {
	ACE_UINT8 dpuId;			/**< ID=255 */
	ACE_UINT8 dpuAddr;			/**< 逻辑地址1-99 */
	ACE_UINT16 dpuStatus;		/**< Byte0:默认主CM的DPU状态; Bit0:主机; Bit1:备机; Bit2:错误态; Bit3:冗余使用\n
								* Byte1:默认备CM的DPU状态; Bit0:主机; Bit1:备机; Bit2:错误态; Bit3:冗余使用 */
	ACE_UINT8 dpuType;			/**< LDPU:0x01; GW:0x11; SDPU:0x21; HDPU:0x31 */
	ACE_UINT8 max_io_num;		/**< 最大IO板数目 */
	ACE_UINT8 max_real_io_num;	/**< 最大实际IO板数目 */
	ACE_UINT8 max_vir_io_num;	/**< 最大虚拟IO板数目 */
	ACE_UINT8 real_io_num;		/**< 运行着的实际IO板数目 */
	ACE_UINT8 vir_io_num;		/**< 使用着的虚拟IO板数目 */
	DPU_VERSION dpuVer;			/**< DPU软件版本号 */
	DPU_VERSION plcVer;			/**< PLC程序版本号 */
	ACE_UINT32 oNetIpAddr1;		/**< IP地址1 */
	ACE_UINT32 oNetIpAddr2;		/**< IP地址2 */
	ACE_UINT32 oNetIpAddr3;		/**< IP地址3 */
	ACE_UINT32 oNetIpAddr4;		/**< IP地址4 */
	ACE_UINT32 syncIpAddr;		/**< 同步IP地址 */
};

/** 描述IO卡的静态属性信息 */
struct IO_ATTR {
	ACE_UINT8 ioid;			/**< 物理地址，槽号 */
	ACE_UINT8 ioaddr;		/**< IO板逻辑地址 */
	ACE_UINT8 status;		/**< 初始态0x01; 配置态0x02; 运行态0x11; 死态0x4F; 错误态0x41; 通讯中断0x42; 配置错误0x44 */
	ACE_UINT8 type;			/**< IO模板类型 */
	ACE_UINT16 iotype;		/**< IO板卡类型，低字节->高字节 */
	ACE_UINT8 iosoftver[7];	/**< IO板软件版本：软件版本号(低字节)->软件版本号(高字节)->软件写入日期:日->软件写入日期:月
							* ->软件写入日期:年(实际年与2000的差值)->CRC校验码低值->CRC校验码高值 */
	ACE_UINT8 ctrldpu;		/**<  */
};

/** 描述IO的状态信息，应由DPBUS协议确定具体结构 */
struct IO_VDI {
	ACE_UINT8 vdilen;		/**< 数据长度 */
	ACE_UINT8 * vdidata;	/**< 数据区，长度为vdilen字节 */
};

/** 描述IO卡的数据类型和访问描述 */
struct IO_DATA_ATTR {
	ACE_UINT8 type;	/**< 点类型：BIT位0x1F; 四字节符点0x24; 三字节符点0x23; 二字节符点0x22; 四字节无符号整型0x34;
					* 三字节无符号整型0x32; 二字节无符号整型0x31; 四字节符号整型0x44; 三字节符号整型0x42; 二字节符号整型0x41 */
	ACE_UINT8 mode;	/**< r=01;w=02; 读写权限 */
	ACE_UINT16 num;	/**< 点数目 */
};

/** 描述IO上送的数据和质量码，同时也存放着IO的数据类型描述 */
struct IO_VALUE {
	ACE_UINT8 qlen;			/**< 质量码长度 */
	ACE_UINT8 * quality;	/**< 质量码数据区，长度为qlen字节 */
	ACE_UINT8 vlen;			/**< 数据区长度 */
	ACE_UINT8 * vvalue;		/**< 数据区，长度为vlen字节 */
	ACE_UINT16 ptnum;		/**< 测点数目 */
	IO_DATA_ATTR* attr[4];	/**< 数据类型描述，最多支持4种类型 */
};

/** 描述IO上送的数据 */
struct IO_DYNA_VALUE {
	ACE_UINT8 qlen;			/**< 质量码长度 */
	ACE_UINT8 * quality;	/**< 质量码数据区，长度为qlen字节 */
	ACE_UINT8 vlen;			/**< 数据区长度 */
	ACE_UINT8 * vvalue;		/**< 数据区，长度为vlen字节 */
};

/** 描述IED卡的静态属性信息 */
struct IED_ATTR {
	ACE_UINT8 ioid;			/**< 物理地址，槽号 */
	ACE_UINT8 ioaddr;		/**< IED板逻辑地址 */
	ACE_UINT8 status;		/**< 初始态0x01; 配置态0x02; 运行态0x11; 死态0x4F; 错误态0x41; 通讯中断0x42; 配置错误0x44 */
	ACE_UINT8 type;			/**< IED模板类型 */
	ACE_UINT16 iotype;		/**< IED板卡类型，低字节->高字节 */
};

/** 描述IED的状态信息，应由DPBUS协议确定具体结构 */
struct IED_VDI {
	ACE_UINT8 vdilen;		/**< 数据长度 */
	ACE_UINT8 * vdidata;	/**< 数据区，长度为vdilen字节 */
};

/** 描述IED上送的数据和质量码，同时也存放着IED的数据类型描述 */
struct IED_VALUE {
	ACE_UINT8 qlenHigh;		/**< 质量码长度，高位 */
	ACE_UINT8 qlenLow;		/**< 质量码长度，低位 */
	ACE_UINT8 * quality;	/**< 质量码数据区，长度为qlen字节 */
	ACE_UINT8 vlenHigh;		/**< 数据区长度，高位 */
	ACE_UINT8 vlenLow;		/**< 数据区长度，低位 */
	ACE_UINT8 * vvalue;		/**< 数据区，长度为vlen字节 */
};

/** 描述VPT卡的静态属性信息 */
struct VPT_ATTR {
	ACE_UINT8 ioid;			/**< 物理地址，槽号 */
	ACE_UINT8 ioaddr;		/**< VPT板逻辑地址 */
	ACE_UINT8 status;		/**< 初始态0x01; 配置态0x02; 运行态0x11; 死态0x4F; 错误态0x41; 通讯中断0x42; 配置错误0x44 */
	ACE_UINT8 type;			/**< VPT模板类型 */
};

/** 描述VPT的质量码长度，为"0x0000" */
struct VPT_QUALIFY {
	ACE_UINT16 qualify;		/**< 质量码长度 */
};

/** 描述VPT上送的数据 */
struct VPT_VALUE {
	ACE_UINT8 vlenHigh;		/**< 数据区长度，高位 */
	ACE_UINT8 vlenLow;		/**< 数据区长度，低位 */
	ACE_UINT8 * vvalue;		/**< 数据区，长度为vlen字节 */
};

#define PI_16_1		22
#define LC_1		36
#define VC_1		34
#define VIR_64BOOL	109
#define VIR_256BOOL	110
#define VIR_512BOOL	102
#define VIR_DINT	104
#define VIR_INT		103
#define VIR_REAL	101
#define VIR_SINT	108
#define VIR_VDINT	106
#define VIR_UINT	105
#define VIR_USINT	107

/**
* @name 信息码表
* @{
*/
#define CALL_FDCS_WHOLE_SYSTEM_DATA						0x10		/**< 召唤FDCS全系统数据 */
#define CALL_FDCS_OBSERVATION_POINT_DATA				0x11		/**< 召唤FDCS测点数据 */
#define CALL_FDCS_STATUS_DATA							0x12		/**< 召唤FDCS状态数据 */
#define CALL_FDCS_SOME_DPU_ALL_DATA						0x13		/**< 召唤FDCS中多个DPU全数据，最多16个 */
#define CALL_FDCS_SOME_DPU_OBSERVATION_POINT_DATA		0x14		/**< 召唤FDCS中多个DPU测点数据，最多16个 */
#define RETURN_OPERATION_FAILED_MESSAGE					0x15		/**< 返回操作失败报文 */
#define CALL_FDCS_SOME_DPU_STATUS_DATA					0x16		/**< 召唤FDCS中多个DPU状态数据，最多16个 */
#define CALL_SOME_IO_BOARD_DATA							0x17		/**< 召唤多个IO板数据,最多16个 */
#define CALL_SOME_DPU_STATUS_DATA						0x18		/**< 召唤多个DPU的状态数据，最多16个 */
#define CALL_SOME_IO_BOARD_STATUS_DATA					0x19		/**< 召唤多个IO板状态数据，最多16个 */
#define CALL_DATA										0x1A		/**< 召唤定值、录波、历史事件 */
#define CALL_DPNET_DATA									0x1B		/**< 召唤DPNET数据 */
#define CALL_DEST_DPU_ALL_DATA							0x1C		/**< 召唤目标节点DPU中所有数据，PHENIX370使用(0x1D同样也使用) */
#define ANSWER_FDCS_WHOLE_SYSTEM_DATA					0x20		/**< 应答FDCS全系统数据 */
#define ANSWER_FDCS_WHOLE_SYSTEM_OBSERVATION_POINT_DATA	0x21		/**< 应答FDCS全系统测点数据 */
#define ANSWER_FDCS_WHOLE_SYSTEM_STATUS_DATA			0x22		/**< 应答FDCS全系统状态数据 */
#define ANSWER_FDCS_SOME_DPU_DATA						0x23		/**< 应答FDCS系统中多个DPU数据 */
#define ANSWER_FDCS_SOME_DPU_OBSERVATION_POINT_DATA		0x24		/**< 应答FDCS系统多个DPU测点数据 */
#define ANSWER_FDCS_SOME_DPU_STATUS_DATA				0x25		/**< 应答FDCS系统多个DPU状态数据 */
#define RETURN_OPERATION_SUCCEED_MESSAGE				0x26		/**< 返回操作成功信息 */
#define ANSWER_FDCS_SOME_IO_BOARD_DATA					0x27		/**< 应答FDCS系统多个IO板数据 */
#define RETURN_FDCS_SOME_DPU_STATUS_DATA_NO_IO_STATUS	0x28		/**< 返回FDCS中多个DPU状态数据(不包括其中的IO状态数据) */
#define RETURN_FDCS_SOME_IO_STATUS_DATA					0x29		/**< 返回FDCS中多个IO状态数据 */
#define ANSWER_DEST_DPU_ALL_DATA						0x2C		/**< 应答目标节点DPU中所有数据，PHENIX370使用(0x2D同样也使用) */
#define DI_POINT_CHANGE_UPLOAD							0x31		/**< DI点变化上送 */
#define ADJUST_OR_CONTROL_RETURN						0x32		/**< 调节或控制返回 */
#define DOWN_CONTROL_AI									0x35		/**< 下行控制AI */
#define DOWN_CONTROL_DI									0x36		/**< 下行控制DI */
#define DATA_FILE_REQUEST_DOWNLOAD						0x3C		/**< 数据文件请求下载 */
#define ALLOW_DATA_DOWNLOAD								0x3D		/**< 可以进行数据下载 */
#define DATA_DOWNLOAD_TRANSFER_COMPLETED				0x3E		/**< 数据下载传输完毕 */
#define PHENIX_370_DOWN_MESSAGE							0x3F		/**< Phenix370下行报文 */
#define	SOE_ACTIVELY_UPLOAD								0x4D		/**< SOE主动上送 */
#define HMI_IO_TRANSPARENT_DATA_DOWN					0x52		/**< HMI-IO透明数据下行 */
#define HMI_IO_TRANSPARENT_DATA_UP						0x57		/**< HMI-IO透明数据上行 */
#define SET_DPU_LOGICAL_ADDRESS							0x60		/**< 设置DPU逻辑地址 */
#define SET_DPU_TYPE									0x61		/**< 设置DPU类型 */
#define SET_DPU_IP_ADDRESS								0x62		/**< 设置DPU IP地址 */
#define SET_DPU_ALL_PROPERTIES							0x63		/**< 设置DPU所有属性 */
#define REQUEST_GET_DPU_PROPERTY_INFO					0x64		/**< 请求得到DPU属性信息 */
#define UPLOAD_DPU_PROPERTY_INFO						0x65		/**< 上送DPU属性信息 */
#define REQUEST_IO_PROPERTY_DATA						0x66		/**< 请求IO属性数据 */
#define UPLOAD_IO_PROPERTY_DATA							0x67		/**< 上送IO属性数据 */
#define ONLINE_ADD_IO_TYPE								0x68		/**< 在线增加IO类型 */
#define ONLINE_SET_IO_ADDRESS_TYPE						0x69		/**< 在线设置IO地址、类型 */
#define SEND_CHECK_TIME_COMMAND							0x71		/**< 发出对时命令 */
#define UPLOAD_WAVE_FILE_LIST							0x72		/**< 上送录波文件列表 */
#define UPLOAD_WAVE_FILE								0x73		/**< 上送录波文件 */
#define FIXED_VALUE_DOWNLOAD							0x74		/**< 定值下传(保留) */
#define UPLOAD_FIXED_VALUE_DATA							0x75		/**< 上送定值数据(保留) */
#define FIXED_VALUE_OPERATION							0x76		/**< 定值操作(保留) */
/** @} */

/**
* @name 事件类型编码表
* @{
*/
#define EVENT_WARNING		0x01		/**< 告警 */
#define EVENT_ACTION		0x02		/**< 动作 */
#define EVENT_RUNNING		0x04		/**< 运行 */
#define EVENT_MANIPULATE	0x08		/**< 操作 */
/** @} */

/**
* @name 详细事件编码表
* @{
*/
/** @} */

/**
* @name 事件参数编码表
* @{
*/
/** @} */