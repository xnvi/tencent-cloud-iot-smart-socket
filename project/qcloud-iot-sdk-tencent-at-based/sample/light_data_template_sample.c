/*
 * Tencent is pleased to support the open source community by making IoT Hub available.
 * Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.

 * Licensed under the MIT License (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://opensource.org/licenses/MIT

 * Unless required by applicable law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include "qcloud_iot_api_export.h"
#include "lite-utils.h"
#include "at_client.h"
#include "string.h"
#include "utils_timer.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"


static bool sg_control_msg_arrived = false;
static char sg_data_report_buffer[AT_CMD_MAX_LEN];
static size_t sg_data_report_buffersize = sizeof(sg_data_report_buffer) / sizeof(sg_data_report_buffer[0]);



/*data_config.c can be generated by tools/codegen.py -c xx/product.json*/
/*-----------------data config start  -------------------*/ 

#define TOTAL_PROPERTY_COUNT 4
#define MAX_STR_NAME_LEN	(64)

static sDataPoint    sg_DataTemplate[TOTAL_PROPERTY_COUNT];

typedef enum{
	eCOLOR_RED = 0,
	eCOLOR_GREEN = 1,
	eCOLOR_BLUE = 2,
}eColor;

typedef struct _ProductDataDefine {
    TYPE_DEF_TEMPLATE_BOOL m_light_switch; 
    TYPE_DEF_TEMPLATE_ENUM m_color;
    TYPE_DEF_TEMPLATE_FLOAT m_brightness;
    TYPE_DEF_TEMPLATE_STRING m_name[MAX_STR_NAME_LEN+1];
} ProductDataDefine;

static   ProductDataDefine     sg_ProductData;

static void _init_data_template(void)
{
    memset((void *) & sg_ProductData, 0, sizeof(ProductDataDefine));
	
	sg_ProductData.m_light_switch = 0;
    sg_DataTemplate[0].data_property.key  = "power_switch";
    sg_DataTemplate[0].data_property.data = &sg_ProductData.m_light_switch;
    sg_DataTemplate[0].data_property.type = TYPE_TEMPLATE_BOOL;

	sg_ProductData.m_color = eCOLOR_RED;
    sg_DataTemplate[1].data_property.key  = "color";
    sg_DataTemplate[1].data_property.data = &sg_ProductData.m_color;
    sg_DataTemplate[1].data_property.type = TYPE_TEMPLATE_ENUM;

	sg_ProductData.m_brightness = 0;
    sg_DataTemplate[2].data_property.key  = "brightness";
    sg_DataTemplate[2].data_property.data = &sg_ProductData.m_brightness;
    sg_DataTemplate[2].data_property.type = TYPE_TEMPLATE_INT;

	DeviceInfo* pInfo = iot_device_info_get();
	strncpy(sg_ProductData.m_name, pInfo->device_name, MAX_STR_NAME_LEN);
    sg_DataTemplate[3].data_property.key  = "name";
    sg_DataTemplate[3].data_property.data = sg_ProductData.m_name;
    sg_DataTemplate[3].data_property.type = TYPE_TEMPLATE_STRING;

};

/*-----------------data config end  -------------------*/ 


/*event_config.c can be generated by tools/codegen.py -c xx/product.json*/
/*-----------------event config start  -------------------*/ 
#ifdef EVENT_POST_ENABLED
#define EVENT_COUNTS     (3)
#define MAX_EVENT_STR_MESSAGE_LEN (64)
#define MAX_EVENT_STR_NAME_LEN (64)


static TYPE_DEF_TEMPLATE_BOOL sg_status;
static TYPE_DEF_TEMPLATE_STRING sg_message[MAX_EVENT_STR_MESSAGE_LEN+1];
static DeviceProperty g_propertyEvent_status_report[] = {

   {.key = "status", .data = &sg_status, .type = TYPE_TEMPLATE_BOOL},
   {.key = "message", .data = sg_message, .type = TYPE_TEMPLATE_STRING},
};

static TYPE_DEF_TEMPLATE_FLOAT sg_voltage;
static DeviceProperty g_propertyEvent_low_voltage[] = {

   {.key = "voltage", .data = &sg_voltage, .type = TYPE_TEMPLATE_FLOAT},
};

static TYPE_DEF_TEMPLATE_STRING sg_name[MAX_EVENT_STR_NAME_LEN+1];
static TYPE_DEF_TEMPLATE_INT sg_error_code;
static DeviceProperty g_propertyEvent_hardware_fault[] = {

   {.key = "name", .data = sg_name, .type = TYPE_TEMPLATE_STRING},
   {.key = "error_code", .data = &sg_error_code, .type = TYPE_TEMPLATE_INT},
};


static sEvent g_events[]={

    {
     .event_name = "status_report",
     .type = "info",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_status_report)/sizeof(g_propertyEvent_status_report[0]),
     .pEventData = g_propertyEvent_status_report,
    },
    {
     .event_name = "low_voltage",
     .type = "alert",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_low_voltage)/sizeof(g_propertyEvent_low_voltage[0]),
     .pEventData = g_propertyEvent_low_voltage,
    },
    {
     .event_name = "hardware_fault",
     .type = "fault",
     .timestamp = 0,
     .eventDataNum = sizeof(g_propertyEvent_hardware_fault)/sizeof(g_propertyEvent_hardware_fault[0]),
     .pEventData = g_propertyEvent_hardware_fault,
    },
};
	 
/*-----------------event config end	-------------------*/ 
#ifdef	EVENT_TIMESTAMP_USED
static void update_events_timestamp(sEvent *pEvents, int count)
{
	int i;
	
	for(i = 0; i < count; i++){
        if (NULL == (&pEvents[i])) { 
	        Log_e("null event pointer"); 
	        return; 
        }			
		pEvents[i].timestamp = 0; //utc time accurate or 0
	}
}
#endif

static void event_post_cb(char *msg, void *context)
{
	Log_d("Reply:%s", msg);
	IOT_Event_clearFlag(context, FLAG_EVENT0|FLAG_EVENT1|FLAG_EVENT2);
}

#endif

#ifdef ACTION_ENABLED

#define TOTAL_ACTION_COUNTS     (1)

static TYPE_DEF_TEMPLATE_INT sg_blink_in_period = 5;
static DeviceProperty g_actionInput_blink[] = {
   {.key = "period", .data = &sg_blink_in_period, .type = TYPE_TEMPLATE_INT}
};
static TYPE_DEF_TEMPLATE_BOOL sg_blink_out_result = 0;
static DeviceProperty g_actionOutput_blink[] = {

   {.key = "result", .data = &sg_blink_out_result, .type = TYPE_TEMPLATE_BOOL},
};

static DeviceAction g_actions[]={

    {
     .pActionId = "blink",
     .timestamp = 0,
     .input_num = sizeof(g_actionInput_blink)/sizeof(g_actionInput_blink[0]),
     .output_num = sizeof(g_actionOutput_blink)/sizeof(g_actionOutput_blink[0]),
     .pInput = g_actionInput_blink,
     .pOutput = g_actionOutput_blink,
    },
};
	 
/*-----------------action config end	-------------------*/ 
static void OnActionCallback(void *pClient, const char *pClientToken, DeviceAction *pAction) 
{	
	int i;
	sReplyPara replyPara;

	//control light blink
	int period = 0;
	DeviceProperty *pActionInput = pAction->pInput;
	for (i = 0; i < pAction->input_num; i++) {		
		if(!strcmp(pActionInput[i].key, "period")) {
			period = *((int*)pActionInput[i].data);
		} else {
			Log_e("no such input[%s]!", pActionInput[i].key);
		}
	}	

	//do blink
	HAL_Printf( "%s[lighting blink][****]" ANSI_COLOR_RESET, ANSI_COLOR_RED);
	HAL_SleepMs(period*1000);
	HAL_Printf( "\r%s[lighting blink][****]" ANSI_COLOR_RESET, ANSI_COLOR_GREEN);
	HAL_SleepMs(period*1000);
	HAL_Printf( "\r%s[lighting blink][****]\n" ANSI_COLOR_RESET, ANSI_COLOR_RED);

	// construct output 
	memset((char *)&replyPara, 0, sizeof(sReplyPara));
	replyPara.code = eDEAL_SUCCESS;
	replyPara.timeout_ms = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;						
	strcpy(replyPara.status_msg, "action execute success!"); //add the message about the action resault 
	
	DeviceProperty *pActionOutnput = pAction->pOutput;	
	*(int*)(pActionOutnput[0].data) = 0; //set result 

	IOT_ACTION_REPLY(pClient, pClientToken, sg_data_report_buffer, sg_data_report_buffersize, pAction, &replyPara); 	
}

static int _register_data_template_action(void *pTemplate_client)
{
	int i,rc;
	
    for (i = 0; i < TOTAL_ACTION_COUNTS; i++) {
	    rc = IOT_Template_Register_Action(pTemplate_client, &g_actions[i], OnActionCallback);
	    if (rc != QCLOUD_RET_SUCCESS) {
	        rc = IOT_Template_Destroy(pTemplate_client);
	        Log_e("register device data template action failed, err: %d", rc);
	        return rc;
	    } else {
	        Log_i("data template action=%s registered.", g_actions[i].pActionId);
	    }
    }

	return QCLOUD_RET_SUCCESS;
}
#endif

static void OnControlMsgCallback(void *pClient, const char *pJsonValueBuffer, uint32_t valueLength, DeviceProperty *pProperty) 
{
    int i = 0;

    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {
		/*???????????????????????????_handle_delta????????????????????????????????????json????????????????????????????????????????????????????????????string/json??????????????????*/
        if (strcmp(sg_DataTemplate[i].data_property.key, pProperty->key) == 0) {
            sg_DataTemplate[i].state = eCHANGED;		
            Log_i("Property=%s changed", pProperty->key);
            sg_control_msg_arrived = true;
            return;
        }
    }

    Log_e("Property=%s changed no match", pProperty->key);
}

static void OnReportReplyCallback(void *pClient, Method method, ReplyAck replyAck, const char *pJsonDocument, void *pUserdata) {
	Log_i("recv report_reply(ack=%d): %s", replyAck,pJsonDocument);
}

/**
 * ????????????????????????
 */
static int _register_data_template_property(void *ptemplate_client)
{
	int i,rc;
	
    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {
	    rc = IOT_Template_Register_Property(ptemplate_client, &sg_DataTemplate[i].data_property, OnControlMsgCallback);
	    if (rc != QCLOUD_RET_SUCCESS) {
	        rc = IOT_Template_Destroy(ptemplate_client);
	        Log_e("register device data template property failed, err: %d", rc);
	        return rc;
	    } else {
	        Log_i("data template property=%s registered.", sg_DataTemplate[i].data_property.key);
	    }
    }

	return QCLOUD_RET_SUCCESS;
}


/*??????????????????????????????*/
static void deal_down_stream_user_logic(void *client, ProductDataDefine *light)
{
	int i;
    const char * ansi_color = NULL;
    const char * ansi_color_name = NULL;
    char brightness_bar[]      = "||||||||||||||||||||";
    int brightness_bar_len = strlen(brightness_bar);

	/*????????????*/
	switch(light->m_color) {
	    case eCOLOR_RED:
	        ansi_color = ANSI_COLOR_RED;
	        ansi_color_name = " RED ";
	        break;
	    case eCOLOR_GREEN:
	        ansi_color = ANSI_COLOR_GREEN;
	        ansi_color_name = "GREEN";
	        break;
	    case eCOLOR_BLUE:
	        ansi_color = ANSI_COLOR_BLUE;
	        ansi_color_name = " BLUE";
	        break;
	    default:
	        ansi_color = ANSI_COLOR_YELLOW;
	        ansi_color_name = "UNKNOWN";
	        break;
	}


	/* ????????????????????? */		    
    brightness_bar_len = (light->m_brightness >= 100)?brightness_bar_len:(int)((light->m_brightness * brightness_bar_len)/100);
    for (i = brightness_bar_len; i < strlen(brightness_bar); i++) {
        brightness_bar[i] = '-';
    }

	if(light->m_light_switch){
        /* ?????????????????????????????????????????? */
		HAL_Printf( "%s[  lighting  ]|[color:%s]|[brightness:%s]|[%s][%d]\n" ANSI_COLOR_RESET,\
					ansi_color,ansi_color_name,brightness_bar,light->m_name);
	}else{
		/* ?????????????????? */
		HAL_Printf( ANSI_COLOR_YELLOW"[  light is off ]|[color:%s]|[brightness:%s]|[%s][%d]\n" ANSI_COLOR_RESET,\
					ansi_color_name,brightness_bar,light->m_name);	
	}
	
#ifdef EVENT_POST_ENABLED
	if(eCHANGED == sg_DataTemplate[0].state){
		if(light->m_light_switch){	
			memset(sg_message, 0, MAX_EVENT_STR_MESSAGE_LEN);
			strcpy(sg_message,"light on");
			sg_status = 1;
		}else{
			memset(sg_message, 0, MAX_EVENT_STR_MESSAGE_LEN);
			strcpy(sg_message,"light off");
			sg_status = 0;			
		}
		IOT_Event_setFlag(client, FLAG_EVENT0);
	}
#endif

	
}


/*????????????????????????????????????????????????,??????????????????*/
static int deal_up_stream_user_logic(void *client, DeviceProperty *pReportDataList[], int *pCount)
{
	int i, j;
	//check local property state
	//_refresh_local_property. if property changed, set sg_DataTemplate[i].state = eCHANGED;
     for (i = 0, j = 0; i < TOTAL_PROPERTY_COUNT; i++) {       
        if(eCHANGED == sg_DataTemplate[i].state) {
            pReportDataList[j++] = &(sg_DataTemplate[i].data_property);
			sg_DataTemplate[i].state = eNOCHANGE;
        }
    }
	*pCount = j;

	return (*pCount > 0)?QCLOUD_RET_SUCCESS:QCLOUD_ERR_FAILURE;
}


static eAtResault net_prepare(void)
{
	eAtResault Ret;
	DeviceInfo sDevInfo;
	at_client_t pclient = at_client_get();	

	memset((char *)&sDevInfo, '\0', sizeof(DeviceInfo));
	Ret = (eAtResault)HAL_GetDevInfo(&sDevInfo);
	
	if(QCLOUD_RET_SUCCESS != Ret){
		Log_e("Get device info err");
		return QCLOUD_ERR_FAILURE;
	}
	
	if(QCLOUD_RET_SUCCESS != module_init(eMODULE_L206D)) 
	{
		Log_e("module init failed");
		goto exit;
	}
	else
	{
		Log_d("module init success");	
	}
	
	/*at_parse thread should work first*/
	while(AT_STATUS_INITIALIZED != pclient->status)
	{	
		HAL_SleepMs(1000);
	}
	
	Log_d("Start shakehands with module...");
	Ret = module_handshake(CMD_TIMEOUT_MS);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("module connect fail,Ret:%d", Ret);
		goto exit;
	}
	else
	{
		Log_d("module connect success");
	}
	
	Ret = iot_device_info_init(sDevInfo.product_id, sDevInfo.device_name, sDevInfo.devSerc);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("dev info init fail,Ret:%d", Ret);
		goto exit;
	}

	Ret = module_info_set(iot_device_info_get(), ePSK_TLS);
	if(QCLOUD_RET_SUCCESS != Ret)
	{
		Log_e("module info set fail,Ret:%d", Ret);
	}

exit:

	return Ret;
}

static void eventPostCheck(void *client)
{
#ifdef EVENT_POST_ENABLED	
	int rc, i;
	uint32_t eflag;
	sEvent *pEventList[EVENT_COUNTS];
	uint8_t EventCont;

	//????????????
	eflag = IOT_Event_getFlag(client);
	if((EVENT_COUNTS > 0 )&& (eflag > 0))
	{	
		EventCont = 0;
		for(i = 0; i < EVENT_COUNTS; i++)
		{

			if((eflag&(1<<i))&ALL_EVENTS_MASK)
			{
				 pEventList[EventCont++] = &(g_events[i]);
#ifdef	EVENT_TIMESTAMP_USED				 
				 update_events_timestamp(&g_events[i], 1);
#endif
			}
		}
		
		rc = IOT_Post_Event(client, sg_data_report_buffer, sg_data_report_buffersize, \
									EventCont, pEventList, event_post_cb);
		if(rc < 0)
		{
			Log_e("event post failed: %d", rc);
		}
		else
		{
			Log_d("event post success");
		}
	}	
#endif

}


/*????????????????????????????????????,??????????????????*/
static int _get_sys_info(void *handle, char *pJsonDoc, size_t sizeOfBuffer)
{
	/*????????????????????????????????????????????????*/
    DeviceProperty plat_info[] = {
     	{.key = "module_hardinfo", .type = TYPE_TEMPLATE_STRING, .data = "ESP8266"},
     	{.key = "module_softinfo", .type = TYPE_TEMPLATE_STRING, .data = "V1.0"},
     	{.key = "fw_ver", 		   .type = TYPE_TEMPLATE_STRING, .data = QCLOUD_IOT_AT_SDK_VERSION},
     	{.key = "imei", 		   .type = TYPE_TEMPLATE_STRING, .data = "11-22-33-44"},
     	{.key = "lat", 			   .type = TYPE_TEMPLATE_STRING, .data = "22.546015"},
     	{.key = "lon", 			   .type = TYPE_TEMPLATE_STRING, .data = "113.941125"},
        {NULL, NULL, JINT32}  //??????
	};
		
	/*?????????????????????*/
	DeviceProperty self_info[] = {
        {.key = "append_info", .type = TYPE_TEMPLATE_STRING, .data = "your self define ifno"},
        {NULL, NULL, JINT32}  //??????
	};

	return IOT_Template_JSON_ConstructSysInfo(handle, pJsonDoc, sizeOfBuffer, plat_info, self_info); 	
}

/*5s????????????????????????,??????????????????????????????????????????*/
int cycle_report(DeviceProperty *pReportDataList[], Timer *reportTimer)
{
	int i;

	if(expired(reportTimer)){
	    for (i = 0; i < TOTAL_PROPERTY_COUNT; i++) {       
			pReportDataList[i] = &(sg_DataTemplate[i].data_property);
			countdown_ms(reportTimer, 5000);
	    }
		return QCLOUD_RET_SUCCESS;
	}else{

		return QCLOUD_ERR_INVAL;
	}		
}

void light_data_template_demo_task(void *arg)
{
	eAtResault Ret;
	int rc;
	int ReportCont;
	Timer reportTimer;
	void *client = NULL;
	at_client_t pclient = at_client_get();	
	DeviceProperty *pReportDataList[TOTAL_PROPERTY_COUNT];
			
	Log_d("template_demo_task Entry...");
	do  
	{
		Ret = net_prepare();
		if(QCLOUD_RET_SUCCESS != Ret)
		{
			Log_e("net prepare fail,Ret:%d", Ret);
			break;
		}
		
		/*
		 *?????????module_register_network ????????????????????????????????????????????????
		*/
		Ret = module_register_network(eMODULE_L206D);
		if(QCLOUD_RET_SUCCESS != Ret)
		{			
			Log_e("network connect fail,Ret:%d", Ret);
			break;
		}
		
		MQTTInitParams init_params = DEFAULT_MQTTINIT_PARAMS;
		Ret = module_mqtt_conn(init_params);
		if(QCLOUD_RET_SUCCESS != Ret)
		{
			Log_e("module mqtt conn fail,Ret:%d", Ret);
			break;
		}
		else
		{
			Log_d("module mqtt conn success");
		}

		
		if(!IOT_MQTT_IsConnected())
		{
			Log_e("mqtt connect fail");
			break;
		}
		
		Ret = (eAtResault)IOT_Template_Construct(&client);
		if(QCLOUD_RET_SUCCESS != Ret)
		{
			Log_e("data template construct fail,Ret:%d", Ret);
			break;
		}
		else
		{
			Log_d("data template construct success");
		}

		//init data template
		_init_data_template();
				
		//register data template propertys here
		rc = _register_data_template_property(client);
		if (rc == QCLOUD_RET_SUCCESS) 
		{
			Log_i("Register data template propertys Success");
		} 
		else 
		{
			Log_e("Register data template propertys Failed: %d", rc);
			break;
		}

			
#ifdef ACTION_ENABLED
		//register data template actions
		rc = _register_data_template_action(client);
		if (rc == QCLOUD_RET_SUCCESS) {
			Log_i("Register data template actions Success");
		} else {
			Log_e("Register data template actions Failed: %d", rc);
			break;
		}
#endif

		//??????????????????,?????????????????????????????????????????????????????????,?????????????????????
		rc = _get_sys_info(client, sg_data_report_buffer, sg_data_report_buffersize);
		if(QCLOUD_RET_SUCCESS == rc)
		{
			rc = IOT_Template_Report_SysInfo_Sync(client, sg_data_report_buffer, sg_data_report_buffersize, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);	
			if (rc != QCLOUD_RET_SUCCESS) 
			{
				Log_e("Report system info fail, err: %d", rc);
				break;
			}
		}
		else
		{
			Log_e("Get system info fail, err: %d", rc);
			break;
		}
		
		//????????????????????????
		rc = IOT_Template_GetStatus_sync(client, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
		if (rc != QCLOUD_RET_SUCCESS) 
		{
			Log_e("Get data status fail, err: %d", rc);
			break;
		}
		else
		{
			Log_d("Get data status success");
		}

		
		//??????????????????timer????????????????????????????????????
		InitTimer(&reportTimer);
		
		while(1)
		{
			HAL_SleepMs(1000);
			IOT_Template_Yield(client, 2000);
			
			/*??????????????????????????????????????????1??????*/
			if (sg_control_msg_arrived) {	
				
				deal_down_stream_user_logic(client, &sg_ProductData);				
				//?????????????????????????????????????????????control msg ??????????????????????????????control msg???????????????????????????control msg(??????Get status??????????????????????????????Control??????)
				sReplyPara replyPara;
				memset((char *)&replyPara, 0, sizeof(sReplyPara));
				replyPara.code = eDEAL_SUCCESS;
				replyPara.timeout_ms = QCLOUD_IOT_MQTT_COMMAND_TIMEOUT;						
				replyPara.status_msg[0] = '\0';			//???????????? replyPara.status_msg ??????????????????????????????????????????????????????
				
				rc = IOT_Template_ControlReply(client, sg_data_report_buffer, sg_data_report_buffersize, &replyPara);
	            if (rc == QCLOUD_RET_SUCCESS) {
					Log_d("Contol msg reply success");
					sg_control_msg_arrived = false;   
	            } else {
	                Log_e("Contol msg reply failed, err: %d", rc);
					break;
	            }

				
			}	

			/*??????????????????,????????????2??????*/			
			if(QCLOUD_RET_SUCCESS == deal_up_stream_user_logic(client, pReportDataList, &ReportCont)){
				
				rc = IOT_Template_JSON_ConstructReportArray(client, sg_data_report_buffer, sg_data_report_buffersize, ReportCont, pReportDataList);
				if (rc == QCLOUD_RET_SUCCESS) {
					rc = IOT_Template_Report(client, sg_data_report_buffer, sg_data_report_buffersize, \
												OnReportReplyCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
					if (rc == QCLOUD_RET_SUCCESS) {
						Log_d("data template reporte success");
					} else {
						Log_e("data template reporte failed, err: %d", rc);
						break;
					}
				} else {
					Log_e("construct reporte data failed, err: %d", rc);
					break;
				}
			}

			/*??????????????????????????????????????????deal_up_stream_user_logic ?????????????????????????????? sg_DataTemplate[i].state = eCHANGED*/			
			if(QCLOUD_RET_SUCCESS == cycle_report(pReportDataList, &reportTimer)){				
				rc = IOT_Template_JSON_ConstructReportArray(client, sg_data_report_buffer, sg_data_report_buffersize, TOTAL_PROPERTY_COUNT, pReportDataList);
		        if (rc == QCLOUD_RET_SUCCESS) {
					Log_d("start cycle report");
		            rc = IOT_Template_Report(client, sg_data_report_buffer, sg_data_report_buffersize, \
		                    					OnReportReplyCallback, NULL, QCLOUD_IOT_MQTT_COMMAND_TIMEOUT);
		            if (rc == QCLOUD_RET_SUCCESS) {
		                Log_d("cycle reporte success");
		            } else {
		                Log_e("cycle reporte failed, err: %d", rc);
		            }

		        } else {
		            Log_e("construct reported failed, err: %d", rc);
					break;
		        }
			}

			eventPostCheck(client);
		}				
	}while (0);
	
	hal_thread_destroy(NULL);
}

void light_data_template_sample(void)
{
	osThreadId demo_threadId;
	
#ifdef OS_USED
	hal_thread_create(&demo_threadId, 512, osPriorityNormal, light_data_template_demo_task, NULL);
	hal_thread_destroy(NULL);
#else
	#error os should be used just now
#endif
}

