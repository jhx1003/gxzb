#pragma once
//GenOil �ͻ��˵���Ϣ����
#define WM_GENOIL_MSG           WM_USER+200

//WPARAM����
//�ϱ��ٶ�
#define WP_GENOIL_SPEED		    1
//�ϱ�DAG����
#define WP_GENOIL_DAG	        2
//share�ύ���
#define WP_GENOIL_SHARE         3
//���ӿ�ν��
#define WP_GENOIL_CONNECT_POOL	4


//����
//�����Զ��˳�
#define WP_GENOIL_AUTOEXIT	    5
//������������Ϣ
#define WP_GENOIL_ERROR_INFO    6
#define WP_GENOIL_ERROR_OPENCL  7


//Zcash N�� �ͻ��˵���Ϣ����
#define WM_ZCASH_N_MSG           WM_USER+300

//WPARAM����
//�ϱ��ٶ�
#define WP_ZCASH_N_SPEED		 1
//GPU�¶�
#define WP_ZCASH_N_GPUTEMP       2 
//share�ύ���
#define WP_ZCASH_N_SHARE         3
//���ӿ�ν��
#define WP_ZCASH_N_CONNECT_POOL	 4


//����
//�����Զ��˳�
#define WP_ZCASH_N_AUTOEXIT	    5
//������������Ϣ
#define WP_ZCASH_N_ERROR_INFO    9



//Zcash A�� �ͻ��˵���Ϣ����
#define WM_ZCASH_A_MSG           WM_USER+400

//WPARAM����
//�ϱ��ٶ�
#define WP_ZCASH_A_SPEED		 1
//GPU�¶�
#define WP_ZCASH_A_GPUTEMP       2 
//share�ύ���
#define WP_ZCASH_A_SHARE         3
//���ӿ�ν��
#define WP_ZCASH_A_CONNECT_POOL	 4


//����
//�����Զ��˳�
#define WP_ZCASH_A_AUTOEXIT	    5
//������������Ϣ
#define WP_ZCASH_A_ERROR_INFO    9


//������Ϣ
#define WM_ERROR_INFO      WM_USER+100
#define MAX_ERROR_LEN      200