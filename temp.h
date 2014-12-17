/** ����LDPU��������DPU */
struct DPU_PEER {
	ACE_UINT8 num;			/**< ��Ҫ��DPU��Ŀ */
	ACE_UINT8 * peerAddr;	/**< DPU���߼���ַ������Ϊnum�ֽ� */
};

/** ����DPNET�ľ�̬��Ϣ */
struct DPNET_ADDR {
	ACE_UINT8 baud_rate;			/**< Bit0:9600; Bit1:115200; Bit2:500K  Bit3:1M; Bit4:2.5M; Bit5:5M; Bit6:Rev; Bit7:Rev */
	ACE_UINT16 thw_max;				/**< ��վ�ȴ���վ��Ӧ�����ʱ�� */
	ACE_UINT16 thr_min;				/**< ��վ�������ݺ�������մ�վ��Ӧ���ݵ���С�ȴ�ʱ�� */
	ACE_UINT16 tsd_min;				/**< �����վ���͵Ĵ�վ��С�ӳ�ʱ�� */
	ACE_UINT16 tsd_max;				/**< �����վ��Ӧ��վ����ӳ�ʱ�� */
	ACE_UINT8 max_retry;			/**< ����ط����� */
	ACE_UINT8 operator_flag;		/**< ��������������޸ı� */
	ACE_UINT16 min_slave_interval;	/**< ɨ�����ڣ���СΪ100us */
	ACE_UINT16 poll_timeout;		/**< ��վ�ȴ���վ��Ӧ�����ȴ�ʱ�� */
	ACE_UINT16 data_control_time;	/**< ÿһ��վ������ʱ�䣬��СΪ10ms */
	ACE_UINT16 datalen_max;			/**< ���������ݳ��� */
	ACE_UINT8 emergency_max;		/**< ������֡���� */
	ACE_UINT8 outputdatalen;		/**< ��վһ�δ���������󳤶� */
	ACE_UINT8 inputdatalen;			/**< ��վһ�δ���������󳤶� */
	ACE_UINT8 * cycle_ref;			/**< �趨ÿ����վ��������(Ϊ��С��������10ms��������)������Ϊ2*Group_max*Slave_max�ֽ� */
};

/** ����ͨ��ģ��ľ�̬��Ϣ */
struct COMM_ATTR {
	ACE_UINT8 gpsserver;			/**< GPS������ */
	ACE_UINT8 commtaskused;			/**< ͨ���������� */
	ACE_UINT8 srvtablesize;			/**< ����(Э��)�������С */
	ACE_UINT8 drvtablesize;			/**< ͨ���������С */
	ACE_UINT16 gpstimestamp;		/**< GPS��ʱƵ�� */
	ACE_UINT16 cnetquicktimestamp;	/**< Cnet��������ʱ��Ƶ�� */
	ACE_UINT16 cnetlowtimestamp;	/**< Cnet��������ʱ��Ƶ�� */
	ACE_UINT8 connectornum;			/**< ����������Ŀ */
	ACE_UINT32 * ip;				/**< ��������Ŀ���ַ������Ϊ4*connectornum�ֽ� */
};

/** ����DPU�ж�̬�仯��״̬��Ϣ */
struct DPU_STATUS {
	ACE_UINT8 temp;							/**< �����¶�[��] */
	ACE_UINT8 cpu;							/**< CPU������[�G] */
	ACE_UINT8 mem;							/**< MEMʹ��״��[Mbyte] */
	ACE_UINT8 hardware;						/**< Ӳ���� */
	ACE_UINT8 dpbus;						/**< dpBus״̬ */
	ACE_UINT8 comm;							/**< Comm״̬ */
	ACE_UINT8 heartbeat;					/**< ͬ�������߹��� */
	ACE_UINT8 dpbus_state;					/**< DPBUS��վ״̬ */
	ACE_UINT8 baud_ratestatus;				/**< DPBUS��վ������ */
	ACE_UINT32 hostsendcount;				/**< ��վ���͵��ܴ���ͳ�� */
	ACE_UINT32 hostrecvcount;				/**< ��վ���յ��ܴ���ͳ�� */
	ACE_UINT32 hosterrorrecvcount;			/**< ��վ���յ���Ч��δ��Ӧ֡����ͳ�� */
	ACE_UINT32 primaryoverrun_count;		/**< ��ͨ�������е�OverRun�������ͳ�� */
	ACE_UINT32 secondaryoverrun_count;		/**< ��ͨ�������е�OverRun�������ͳ�� */
	ACE_UINT32 primaryparity_count;			/**< ��ͨ�������е�Parity�������ͳ�� */
	ACE_UINT32 secondaryparity_count;		/**< ��ͨ�������е�Parity�������ͳ�� */
	ACE_UINT32 primaryframeclosed_count;	/**< ��ͨ�������е�Frame Closed�������ͳ�� */
	ACE_UINT32 secondaryframeclosed_count;	/**< ��ͨ�������е�Frame Closed�������ͳ�� */
	ACE_UINT16 send_maxtimespend;			/**< CPU����һ֡�������ʱ�䳤�� */
	ACE_UINT16 recv_maxtimespend;			/**< CPU����һ֡�������ʱ�䳤�� */
};

/** ����IO���Ķ�̬������Ϣ */
struct IO_DYNA_ATTR {
	ACE_UINT8 ioid;		/**< �����ַ���ۺ� */
	ACE_UINT8 ioaddr;	/**< IO���߼���ַ */
	ACE_UINT8 status;	/**< ��ʼ̬0x01; ����̬0x02; ����̬0x11; ��̬0x4F; ����̬0x41; ͨѶ�ж�0x42; ���ô���0x44 */
	ACE_UINT8 type;		/**< IOģ������ */
};

/** �������͵��¼���һ��32���ֽ� */
struct IO_DYNA_EVENT {
	ACE_UINT8 dpuaddr;			/**< DPU��ַ */
	ACE_UINT8 ioaddr;			/**< io��ַ������ȫװ�ã�ΪFFH */
	ACE_UINT8 time56[7];		/**< 7�ֽ�ʱ�� */
	ACE_UINT8 dputype;			/**< DPU���� */
	ACE_UINT8 eventtype;		/**< �¼����ͣ���Ϊ�澯���������������֣���"�¼����ͱ���" */
	ACE_UINT8 eventcode;		/**< �¼����룬������ϸ�¼����롱������ */
	ACE_UINT8 eventdesp[40];	/**< 40���ֽڵ��ı������¼�������������40����ʣ��ռ���0������������EVENT_DESP */
};

/** �洢��DPU��״̬�������������ݵľ��嶨���DPU_STATUS */
struct DPU_VDI {
	ACE_UINT8 vdilen;		/**< ���ݳ��� */
	ACE_UINT8 * vdidata;	/**< ������������Ϊvdilen�ֽ� */
};

/** ��������汾�����ַ�����ʾ����DPU-V1.00-07.04.08����ʾdpu����汾 */
struct DPU_VERSION {
	ACE_UINT8 len;			/**< �汾���ݳ��� */
	ACE_UINT8 * version;	/**< �汾���ݣ�����len�ֽ� */
};

/** ����DPU����صľ�̬��Ϣ */
struct DPU_ATTR {
	ACE_UINT8 dpuId;			/**< ID=255 */
	ACE_UINT8 dpuAddr;			/**< �߼���ַ1-99 */
	ACE_UINT16 dpuStatus;		/**< Byte0:Ĭ����CM��DPU״̬; Bit0:����; Bit1:����; Bit2:����̬; Bit3:����ʹ��\n
								* Byte1:Ĭ�ϱ�CM��DPU״̬; Bit0:����; Bit1:����; Bit2:����̬; Bit3:����ʹ�� */
	ACE_UINT8 dpuType;			/**< LDPU:0x01; GW:0x11; SDPU:0x21; HDPU:0x31 */
	ACE_UINT8 max_io_num;		/**< ���IO����Ŀ */
	ACE_UINT8 max_real_io_num;	/**< ���ʵ��IO����Ŀ */
	ACE_UINT8 max_vir_io_num;	/**< �������IO����Ŀ */
	ACE_UINT8 real_io_num;		/**< �����ŵ�ʵ��IO����Ŀ */
	ACE_UINT8 vir_io_num;		/**< ʹ���ŵ�����IO����Ŀ */
	DPU_VERSION dpuVer;			/**< DPU����汾�� */
	DPU_VERSION plcVer;			/**< PLC����汾�� */
	ACE_UINT32 oNetIpAddr1;		/**< IP��ַ1 */
	ACE_UINT32 oNetIpAddr2;		/**< IP��ַ2 */
	ACE_UINT32 oNetIpAddr3;		/**< IP��ַ3 */
	ACE_UINT32 oNetIpAddr4;		/**< IP��ַ4 */
	ACE_UINT32 syncIpAddr;		/**< ͬ��IP��ַ */
};

/** ����IO���ľ�̬������Ϣ */
struct IO_ATTR {
	ACE_UINT8 ioid;			/**< �����ַ���ۺ� */
	ACE_UINT8 ioaddr;		/**< IO���߼���ַ */
	ACE_UINT8 status;		/**< ��ʼ̬0x01; ����̬0x02; ����̬0x11; ��̬0x4F; ����̬0x41; ͨѶ�ж�0x42; ���ô���0x44 */
	ACE_UINT8 type;			/**< IOģ������ */
	ACE_UINT16 iotype;		/**< IO�忨���ͣ����ֽ�->���ֽ� */
	ACE_UINT8 iosoftver[7];	/**< IO������汾������汾��(���ֽ�)->����汾��(���ֽ�)->���д������:��->���д������:��
							* ->���д������:��(ʵ������2000�Ĳ�ֵ)->CRCУ�����ֵ->CRCУ�����ֵ */
	ACE_UINT8 ctrldpu;		/**<  */
};

/** ����IO��״̬��Ϣ��Ӧ��DPBUSЭ��ȷ������ṹ */
struct IO_VDI {
	ACE_UINT8 vdilen;		/**< ���ݳ��� */
	ACE_UINT8 * vdidata;	/**< ������������Ϊvdilen�ֽ� */
};

/** ����IO�����������ͺͷ������� */
struct IO_DATA_ATTR {
	ACE_UINT8 type;	/**< �����ͣ�BITλ0x1F; ���ֽڷ���0x24; ���ֽڷ���0x23; ���ֽڷ���0x22; ���ֽ��޷�������0x34;
					* ���ֽ��޷�������0x32; ���ֽ��޷�������0x31; ���ֽڷ�������0x44; ���ֽڷ�������0x42; ���ֽڷ�������0x41 */
	ACE_UINT8 mode;	/**< r=01;w=02; ��дȨ�� */
	ACE_UINT16 num;	/**< ����Ŀ */
};

/** ����IO���͵����ݺ������룬ͬʱҲ�����IO�������������� */
struct IO_VALUE {
	ACE_UINT8 qlen;			/**< �����볤�� */
	ACE_UINT8 * quality;	/**< ������������������Ϊqlen�ֽ� */
	ACE_UINT8 vlen;			/**< ���������� */
	ACE_UINT8 * vvalue;		/**< ������������Ϊvlen�ֽ� */
	ACE_UINT16 ptnum;		/**< �����Ŀ */
	IO_DATA_ATTR* attr[4];	/**< �����������������֧��4������ */
};

/** ����IO���͵����� */
struct IO_DYNA_VALUE {
	ACE_UINT8 qlen;			/**< �����볤�� */
	ACE_UINT8 * quality;	/**< ������������������Ϊqlen�ֽ� */
	ACE_UINT8 vlen;			/**< ���������� */
	ACE_UINT8 * vvalue;		/**< ������������Ϊvlen�ֽ� */
};

/** ����IED���ľ�̬������Ϣ */
struct IED_ATTR {
	ACE_UINT8 ioid;			/**< �����ַ���ۺ� */
	ACE_UINT8 ioaddr;		/**< IED���߼���ַ */
	ACE_UINT8 status;		/**< ��ʼ̬0x01; ����̬0x02; ����̬0x11; ��̬0x4F; ����̬0x41; ͨѶ�ж�0x42; ���ô���0x44 */
	ACE_UINT8 type;			/**< IEDģ������ */
	ACE_UINT16 iotype;		/**< IED�忨���ͣ����ֽ�->���ֽ� */
};

/** ����IED��״̬��Ϣ��Ӧ��DPBUSЭ��ȷ������ṹ */
struct IED_VDI {
	ACE_UINT8 vdilen;		/**< ���ݳ��� */
	ACE_UINT8 * vdidata;	/**< ������������Ϊvdilen�ֽ� */
};

/** ����IED���͵����ݺ������룬ͬʱҲ�����IED�������������� */
struct IED_VALUE {
	ACE_UINT8 qlenHigh;		/**< �����볤�ȣ���λ */
	ACE_UINT8 qlenLow;		/**< �����볤�ȣ���λ */
	ACE_UINT8 * quality;	/**< ������������������Ϊqlen�ֽ� */
	ACE_UINT8 vlenHigh;		/**< ���������ȣ���λ */
	ACE_UINT8 vlenLow;		/**< ���������ȣ���λ */
	ACE_UINT8 * vvalue;		/**< ������������Ϊvlen�ֽ� */
};

/** ����VPT���ľ�̬������Ϣ */
struct VPT_ATTR {
	ACE_UINT8 ioid;			/**< �����ַ���ۺ� */
	ACE_UINT8 ioaddr;		/**< VPT���߼���ַ */
	ACE_UINT8 status;		/**< ��ʼ̬0x01; ����̬0x02; ����̬0x11; ��̬0x4F; ����̬0x41; ͨѶ�ж�0x42; ���ô���0x44 */
	ACE_UINT8 type;			/**< VPTģ������ */
};

/** ����VPT�������볤�ȣ�Ϊ"0x0000" */
struct VPT_QUALIFY {
	ACE_UINT16 qualify;		/**< �����볤�� */
};

/** ����VPT���͵����� */
struct VPT_VALUE {
	ACE_UINT8 vlenHigh;		/**< ���������ȣ���λ */
	ACE_UINT8 vlenLow;		/**< ���������ȣ���λ */
	ACE_UINT8 * vvalue;		/**< ������������Ϊvlen�ֽ� */
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
* @name ��Ϣ���
* @{
*/
#define CALL_FDCS_WHOLE_SYSTEM_DATA						0x10		/**< �ٻ�FDCSȫϵͳ���� */
#define CALL_FDCS_OBSERVATION_POINT_DATA				0x11		/**< �ٻ�FDCS������� */
#define CALL_FDCS_STATUS_DATA							0x12		/**< �ٻ�FDCS״̬���� */
#define CALL_FDCS_SOME_DPU_ALL_DATA						0x13		/**< �ٻ�FDCS�ж��DPUȫ���ݣ����16�� */
#define CALL_FDCS_SOME_DPU_OBSERVATION_POINT_DATA		0x14		/**< �ٻ�FDCS�ж��DPU������ݣ����16�� */
#define RETURN_OPERATION_FAILED_MESSAGE					0x15		/**< ���ز���ʧ�ܱ��� */
#define CALL_FDCS_SOME_DPU_STATUS_DATA					0x16		/**< �ٻ�FDCS�ж��DPU״̬���ݣ����16�� */
#define CALL_SOME_IO_BOARD_DATA							0x17		/**< �ٻ����IO������,���16�� */
#define CALL_SOME_DPU_STATUS_DATA						0x18		/**< �ٻ����DPU��״̬���ݣ����16�� */
#define CALL_SOME_IO_BOARD_STATUS_DATA					0x19		/**< �ٻ����IO��״̬���ݣ����16�� */
#define CALL_DATA										0x1A		/**< �ٻ���ֵ��¼������ʷ�¼� */
#define CALL_DPNET_DATA									0x1B		/**< �ٻ�DPNET���� */
#define CALL_DEST_DPU_ALL_DATA							0x1C		/**< �ٻ�Ŀ��ڵ�DPU���������ݣ�PHENIX370ʹ��(0x1Dͬ��Ҳʹ��) */
#define ANSWER_FDCS_WHOLE_SYSTEM_DATA					0x20		/**< Ӧ��FDCSȫϵͳ���� */
#define ANSWER_FDCS_WHOLE_SYSTEM_OBSERVATION_POINT_DATA	0x21		/**< Ӧ��FDCSȫϵͳ������� */
#define ANSWER_FDCS_WHOLE_SYSTEM_STATUS_DATA			0x22		/**< Ӧ��FDCSȫϵͳ״̬���� */
#define ANSWER_FDCS_SOME_DPU_DATA						0x23		/**< Ӧ��FDCSϵͳ�ж��DPU���� */
#define ANSWER_FDCS_SOME_DPU_OBSERVATION_POINT_DATA		0x24		/**< Ӧ��FDCSϵͳ���DPU������� */
#define ANSWER_FDCS_SOME_DPU_STATUS_DATA				0x25		/**< Ӧ��FDCSϵͳ���DPU״̬���� */
#define RETURN_OPERATION_SUCCEED_MESSAGE				0x26		/**< ���ز����ɹ���Ϣ */
#define ANSWER_FDCS_SOME_IO_BOARD_DATA					0x27		/**< Ӧ��FDCSϵͳ���IO������ */
#define RETURN_FDCS_SOME_DPU_STATUS_DATA_NO_IO_STATUS	0x28		/**< ����FDCS�ж��DPU״̬����(���������е�IO״̬����) */
#define RETURN_FDCS_SOME_IO_STATUS_DATA					0x29		/**< ����FDCS�ж��IO״̬���� */
#define ANSWER_DEST_DPU_ALL_DATA						0x2C		/**< Ӧ��Ŀ��ڵ�DPU���������ݣ�PHENIX370ʹ��(0x2Dͬ��Ҳʹ��) */
#define DI_POINT_CHANGE_UPLOAD							0x31		/**< DI��仯���� */
#define ADJUST_OR_CONTROL_RETURN						0x32		/**< ���ڻ���Ʒ��� */
#define DOWN_CONTROL_AI									0x35		/**< ���п���AI */
#define DOWN_CONTROL_DI									0x36		/**< ���п���DI */
#define DATA_FILE_REQUEST_DOWNLOAD						0x3C		/**< �����ļ��������� */
#define ALLOW_DATA_DOWNLOAD								0x3D		/**< ���Խ����������� */
#define DATA_DOWNLOAD_TRANSFER_COMPLETED				0x3E		/**< �������ش������ */
#define PHENIX_370_DOWN_MESSAGE							0x3F		/**< Phenix370���б��� */
#define	SOE_ACTIVELY_UPLOAD								0x4D		/**< SOE�������� */
#define HMI_IO_TRANSPARENT_DATA_DOWN					0x52		/**< HMI-IO͸���������� */
#define HMI_IO_TRANSPARENT_DATA_UP						0x57		/**< HMI-IO͸���������� */
#define SET_DPU_LOGICAL_ADDRESS							0x60		/**< ����DPU�߼���ַ */
#define SET_DPU_TYPE									0x61		/**< ����DPU���� */
#define SET_DPU_IP_ADDRESS								0x62		/**< ����DPU IP��ַ */
#define SET_DPU_ALL_PROPERTIES							0x63		/**< ����DPU�������� */
#define REQUEST_GET_DPU_PROPERTY_INFO					0x64		/**< ����õ�DPU������Ϣ */
#define UPLOAD_DPU_PROPERTY_INFO						0x65		/**< ����DPU������Ϣ */
#define REQUEST_IO_PROPERTY_DATA						0x66		/**< ����IO�������� */
#define UPLOAD_IO_PROPERTY_DATA							0x67		/**< ����IO�������� */
#define ONLINE_ADD_IO_TYPE								0x68		/**< ��������IO���� */
#define ONLINE_SET_IO_ADDRESS_TYPE						0x69		/**< ��������IO��ַ������ */
#define SEND_CHECK_TIME_COMMAND							0x71		/**< ������ʱ���� */
#define UPLOAD_WAVE_FILE_LIST							0x72		/**< ����¼���ļ��б� */
#define UPLOAD_WAVE_FILE								0x73		/**< ����¼���ļ� */
#define FIXED_VALUE_DOWNLOAD							0x74		/**< ��ֵ�´�(����) */
#define UPLOAD_FIXED_VALUE_DATA							0x75		/**< ���Ͷ�ֵ����(����) */
#define FIXED_VALUE_OPERATION							0x76		/**< ��ֵ����(����) */
/** @} */

/**
* @name �¼����ͱ����
* @{
*/
#define EVENT_WARNING		0x01		/**< �澯 */
#define EVENT_ACTION		0x02		/**< ���� */
#define EVENT_RUNNING		0x04		/**< ���� */
#define EVENT_MANIPULATE	0x08		/**< ���� */
/** @} */

/**
* @name ��ϸ�¼������
* @{
*/
/** @} */

/**
* @name �¼����������
* @{
*/
/** @} */