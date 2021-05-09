#ifndef _TOS_CONFIG_H_
#define _TOS_CONFIG_H_

#include "stm32g0xx.h" // Ŀ��оƬͷ�ļ����û���Ҫ�����������

#define TOS_CFG_TASK_PRIO_MAX                       10u // ����TencentOS tinyĬ��֧�ֵ�������ȼ�����

#define TOS_CFG_ROUND_ROBIN_EN                      1u // ����TencentOS tiny���ں��Ƿ���ʱ��Ƭ��ת

#define TOS_CFG_OBJECT_VERIFY_EN                    0u // ����TencentOS tiny�Ƿ�У��ָ��Ϸ�

#define TOS_CFG_TASK_DYNAMIC_CREATE_EN              1u // TencentOS tiny ��̬���񴴽����ܺ�

#define TOS_CFG_EVENT_EN                            1u // TencentOS tiny �¼�ģ�鹦�ܺ�

#define TOS_CFG_MMBLK_EN                            1u //����TencentOS tiny�Ƿ����ڴ�����ģ��

#define TOS_CFG_MMHEAP_EN                           1u //����TencentOS tiny�Ƿ�����̬�ڴ�ģ��

#define TOS_CFG_MMHEAP_DEFAULT_POOL_EN              1u // TencentOS tiny Ĭ�϶�̬�ڴ�ع��ܺ�

#define TOS_CFG_MMHEAP_DEFAULT_POOL_SIZE            0x3000 // ����TencentOS tinyĬ�϶�̬�ڴ�ش�С

#define TOS_CFG_MUTEX_EN                            1u // ����TencentOS tiny�Ƿ���������ģ��

#define TOS_CFG_MESSAGE_QUEUE_EN                    0u // ����TencentOS tiny�Ƿ�����Ϣ����ģ��

#define TOS_CFG_MAIL_QUEUE_EN                       0u // ����TencentOS tiny�Ƿ�����Ϣ����ģ��

#define TOS_CFG_PRIORITY_MESSAGE_QUEUE_EN           0u // ����TencentOS tiny�Ƿ������ȼ���Ϣ����ģ��

#define TOS_CFG_PRIORITY_MAIL_QUEUE_EN              0u // ����TencentOS tiny�Ƿ������ȼ���Ϣ����ģ��

#define TOS_CFG_TIMER_EN                            1u // ����TencentOS tiny�Ƿ��������ʱ��ģ��

#define TOS_CFG_PWR_MGR_EN                          0u // ����TencentOS tiny�Ƿ��������Դ����ģ��

#define TOS_CFG_TICKLESS_EN                         0u // ����Tickless �͹���ģ�鿪��

#define TOS_CFG_SEM_EN                              0u // ����TencentOS tiny�Ƿ����ź���ģ��

#define TOS_CFG_TASK_STACK_DRAUGHT_DEPTH_DETACT_EN  1u // ����TencentOS tiny�Ƿ�������ջ��ȼ��

#define TOS_CFG_FAULT_BACKTRACE_EN                  0u // ����TencentOS tiny�Ƿ����쳣ջ���ݹ���

#define TOS_CFG_IDLE_TASK_STK_SIZE                  128u // ����TencentOS tiny��������ջ��С

#define TOS_CFG_CPU_TICK_PER_SECOND                 1000u // ����TencentOS tiny��tickƵ��

#define TOS_CFG_CPU_CLOCK                           (SystemCoreClock) // ����TencentOS tiny CPUƵ��

#define TOS_CFG_TIMER_AS_PROC                       1u // �����Ƿ�TIMER���óɺ���ģʽ

#endif
