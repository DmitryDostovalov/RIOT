/*
 * Copyright (C) 2016 cr0s
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file
 * @brief
 * @author      cr0s
 */

#ifdef __cplusplus
extern "C" {
#endif

/* define is autogenerated, do not change */
#undef _UMDK_MID_
#define _UMDK_MID_ UNWDS_CONFIG_MODULE_ID

/* define is autogenerated, do not change */
#undef _UMDK_NAME_
#define _UMDK_NAME_ "config"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "board.h"
#include "unwds-common.h"
#include "ls-settings.h"

#include "umdk-config.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

static rtctimer_t timer;

static void umdk_config_reset_system(void *arg) {
    NVIC_SystemReset();
}

void umdk_config_init(uint32_t *non_gpio_pin_map, uwnds_cb_t *event_callback)
{
    (void) non_gpio_pin_map;
    (void) event_callback;
    
    /* Create handler thread */
    /*
	char *stack = (char *) allocate_stack(1024);
	if (!stack) {
		puts("[umdk-" _UMDK_NAME_ "] unable to allocate memory. Are too many modules enabled?");
		return;
	}
    timer_pid = thread_create(stack, UNWDS_STACK_SIZE_BYTES, THREAD_PRIORITY_MAIN - 1, THREAD_CREATE_STACKTEST, timer_thread, NULL, "config thread");
    */
}

static inline void do_reply(module_data_t *reply, umdk_config_reply_t reply_code)
{
    reply->length = 2;
    reply->data[0] = _UMDK_MID_;
    reply->data[1] = reply_code;
}

static bool config_cmd(module_data_t *cmd, module_data_t *reply, bool with_reply)
{
    uint8_t command = cmd->data[0];

    switch (command) {
        case UMDK_CONFIG_MODULES: {
            uint8_t id = cmd->data[1];
            uint8_t onoff = cmd->data[2];
            if (!unwds_is_module_exists(id)) {
                do_reply(reply, UMDK_CONFIG_REPLY_ERR);
                return false;
            }
            
            unwds_set_module(id, onoff);
            unwds_config_save();
            
            char status[10];
            if (onoff) {
                strcpy(status, "enabled");
            } else {
                strcpy(status, "disabled");
            }
            printf("[umdk-" _UMDK_NAME_ "] Module %d %s, configuration saved\n", (int) id, status);
            
            do_reply(reply, UMDK_CONFIG_REPLY_OK);
            break;
        }
        case UMDK_REBOOT_DEVICE: {
            do_reply(reply, UMDK_CONFIG_REPLY_OK);
            /* reboot in 30 seconds */
            timer.callback = umdk_config_reset_system;
            rtctimers_set(&timer, 30);
            break;
        }
        case UMDK_SET_CLASS: {
            char class = cmd->data[1];
            switch (class) {
                case 'a':
                    puts("[umdk-" _UMDK_NAME_ "] set node class A");
                    unwds_set_class(LS_ED_CLASS_A);
                    unwds_config_save();
                    do_reply(reply, UMDK_CONFIG_REPLY_OK);
                    break;
                case 'b':
                case 'c':
                    puts("[umdk-" _UMDK_NAME_ "] set node class C");
                    unwds_set_class(LS_ED_CLASS_C);
                    unwds_config_save();
                    do_reply(reply, UMDK_CONFIG_REPLY_OK);
                    break;
                default:
                    do_reply(reply, UMDK_CONFIG_REPLY_ERR);
                    break;
            }
            break;
        }
        
        default:
            do_reply(reply, UMDK_CONFIG_REPLY_ERR);
            break;
    }

    return true;
}

bool umdk_config_cmd(module_data_t *cmd, module_data_t *reply)
{
    return config_cmd(cmd, reply, true);
}

#ifdef __cplusplus
}
#endif