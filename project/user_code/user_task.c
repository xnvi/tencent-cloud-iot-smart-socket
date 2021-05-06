#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_hal.h"

#include "tos_k.h"
#include "user_task.h"
#include "user_utils.h"
#include "hlw8032.h"
#include "key.h"
#include "ws2812_driver.h"
#include "at_client.h"

uint32_t g_do_upload_property = 0;
electric_info g_power_info = {0.0, 0.0, 0.0, 0.0, 0.0, 0};
uint32_t g_switch = 0;
uint32_t g_count_down = 0;
uint32_t g_count_down_update = 0;

// const uint8_t product_info[128]__attribute__((at(0x0801b000))) = "this is test product_info 0123456789";


// void user_task(void *arg);

// #define USER_TASK_STACK_SIZE 1024
// k_stack_t user_task_stack[USER_TASK_STACK_SIZE];
// k_task_t user_task_t;

// void start_user_task(void *arg)
// {
// 	tos_task_create(&user_task_t,
// 					"user_task",
// 					(k_task_entry_t)user_task,
// 					arg,
// 					4,
// 					user_task_stack,
// 					USER_TASK_STACK_SIZE,
// 					0);
// }





void wifi_config(void)
{
	int i;
	// int result = QCLOUD_RET_SUCCESS;
	at_response_t resp = NULL;
	// uint8_t key = 0; // TODO 读取按键退出配网

	g_do_upload_property = 0;

	tos_sleep_ms(3000);

	// 改成闪灯
	// CleanScreen();
	// DrawPic(0, 0, 128, 32, (uint8_t *)wifi_config_pic1);
	// RefreshFullScreen();

	resp = at_create_resp(64, 0, 2000);

	// TODO 配网的流程还不太合理，但是目前可以使用

	// if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCMQTTSTATE?"))
	// {
	// 	printf("cmd AT+TCMQTTSTATE exec err");
	// 	result = QCLOUD_ERR_FAILURE;
	// }
	// printf("TCMQTTSTATE ret lines %d\r\n", resp->line_counts);
	// printf("%s", resp->buf);
	// for (int i = 0; i < resp->line_counts - 1; i++)
	// {
	//     printf("%s\r\n", at_resp_get_line(resp, i + 1));
	// }
	// printf("TCMQTTSTATE end\r\n", resp->line_counts);

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCMQTTDISCONN"))
	{
		printf("cmd AT+TCMQTTDISCONN exec err");
		// result = QCLOUD_ERR_FAILURE;
	}

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+CWQAP"))
	{
		printf("cmd AT+CWQAP exec err");
		// result = QCLOUD_ERR_FAILURE;
	}
	
	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTARTSMART"))
	{
		printf("cmd AT+TCSTARTSMART exec err");
		// result = QCLOUD_ERR_FAILURE;
	}

	tos_sleep_ms(3000);

	// 等待串口收到下面这个，说明配网完成
	// Smartconfig connected Wi-Fi

	// 改成闪灯
	// DrawPic(0, 0, 128, 32, (uint8_t *)wifi_config_pic2);
	// RefreshFullScreen();
	// tos_sleep_ms(3000);

	// 等待60秒，结束配网
	// CleanScreen();
	for(i=0; i<600; i++)
	{
		// key = ReadEncoderKey();
		// if(key == ENCODER_KEY_RELEASE)
		// {
		// 	break;
		// }

		// 改成闪灯
		// sprintf(&menu_line1[0], "配网中，请稍等 %02d", 60 - i / 10);
		// PrintString12(0, 0, menu_line1);
		// PrintString12(0, 14, "单击旋钮返回");
		// RefreshFullScreen();
		tos_sleep_ms(70);
	}

	if(QCLOUD_RET_SUCCESS != at_exec_cmd(resp, "AT+TCSTOPSMART"))
	{
		// printf("cmd AT+TCSTARTSMART exec err");
		// result = QCLOUD_ERR_FAILURE;
	}

	if(resp)
	{
		at_delete_resp(resp);
	}

	// 配网无论成功与否都会系统复位
	HAL_NVIC_SystemReset();
}





void user_task(void *arg)
{
	uint32_t key = 0;
	uint32_t key_timer_old = 0;
	uint32_t key_timer_new = 0;
	uint32_t wh_timer_old = 0;
	uint32_t time_sec_old = 0; // 秒级更新时间
	uint32_t power_timer_old = 0;
	uint32_t total_wh = 0;
	uint32_t color = 0;
	int32_t i = 0;

	printf("user task start\r\n");

	relay_switch(0);
  	ws2812_reset();
  	esp8266_rst();
	
	total_wh = read_wh();
	hlw8032_init(total_wh);
	
	time_sec_old = HAL_GetTick() / 1000;
	wh_timer_old = HAL_GetTick();
	power_timer_old = HAL_GetTick();
	g_do_upload_property = 1;
	while (1)
	{
		color = color_ring(i);
		color = color & 0x00f0f0f0;
		color = color >> 4;
		ws2812_write_color_u32(color);
		
		i += 1;
		if (i >= 1536)
		{
			i = 0;
		}
		// HAL_Delay(5);


		if (HAL_GetTick() - power_timer_old > 2000)
		{
			power_timer_old = HAL_GetTick();
			hlw8032_get_info(&g_power_info);
		}

		
		// 保存累计电力，延长flash寿命，只有每分钟且用电量变化时才写入
		// if (HAL_GetTick() - wh_timer_old > 60 * 1000 && total_wh != g_power_info.total_wh)
		if (0)
		{
			wh_timer_old = HAL_GetTick();
			total_wh = g_power_info.total_wh;
			save_wh(total_wh);
		}


		key = key_read();
		// key = key_hw_read(0);
		if (key)
		{
			// printf("key 0x%x \r\n", key);
			key_timer_old = HAL_GetTick();
			while (1)
			{
				tos_task_delay(tos_millisec2tick(31));
				key = key_read();
				if (key == 0)
				{
					break;
				}
			}
			key_timer_new = HAL_GetTick();
			if (key_timer_new - key_timer_old < 1000)
			{
				// 开关
				printf("switch %d \r\n", g_switch);
				g_switch = !g_switch;
			}
			else if (key_timer_new - key_timer_old > 5000 && key_timer_new - key_timer_old < 10000)
			{
				// 配网
				printf("wifi config\r\n");
				wifi_config();
			}
			else if (key_timer_new - key_timer_old > 10000 && key_timer_new - key_timer_old < 15000)
			{
				// 恢复出厂设置
				printf("config reset\r\n");
			}
			else
			{
				// 无效按键
			}
			key_timer_old = 0;
			key_timer_new = 0;
		}
		// printf("key 0x%x \r\n", key);

		// 更新倒计时
		if (g_count_down > 0 && time_sec_old != HAL_GetTick() / 1000)
		{
			g_count_down -= 1;
			time_sec_old = HAL_GetTick() / 1000;
		}
		if (g_count_down == 0 && g_count_down_update)
		{
			g_count_down_update = 0;
			g_switch = !g_switch;
		}

		// 设置继电器状态
		relay_switch(g_switch);

		tos_task_delay(tos_millisec2tick(7));
	}
}
