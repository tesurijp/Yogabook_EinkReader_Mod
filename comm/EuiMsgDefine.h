/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */


#pragma once
//定义程序所需要的消息

// 自定义消息
#define EMSG_HB_TYPE_BASE	0xAE
#define EMSG_TS_REQUEST(_MJ_NUM,_MN_NUM) EMSG_DEFINE(EMSG_HB_TYPE_BASE,_MJ_NUM,_MN_NUM)

#define EI_POWER_ENTER_CONNECTED_STANDBY 0
#define EI_POWER_RETURN_CONNECTED_STANDBY 1
#define EI_POWER_ENTER_SHUTDOWN 2
// 电源事件
#define EEVT_HB_POWER_CHANGE		EMSG_TS_REQUEST(1,1)			
// input ULONG  事件ID，如上
// output none
// return none

// 机器形态变化
#define EEVT_HB_MODE_CHANGE		EMSG_TS_REQUEST(1,2)			
// input ULONG  形态ID
// output none
// return none

// 关机行为
#define EEVT_HB_SHUT_DOWN	EMSG_TS_REQUEST(1,3)			
// NONE
// output none
// return none

// 用户session事件
#define EEVT_HB_SESSION_CHANGE		EMSG_TS_REQUEST(1,4)			
// input ULONG  wparam
// output none
// return none

// 前台/后台
#define EEVT_HB_ACTIVITE		EMSG_TS_REQUEST(1,5)			
// input ULONG  wparam 0后台 1前台
// output none
// return none

// 服务通知事件
#define EEVT_HB_SERVICE_NOTIFY		EMSG_TS_REQUEST(1,6)			
// input ULONG  wparam 消息ID
// output none
// return none

// 服务通知机器形态发生变化
#define EEVT_HB_LAPTOP_CHANGED		EMSG_TS_REQUEST(1,7)			
// input ULONG  形态定义 GIR_MODE_LAPTOP/GIR_MODE_TENT/GIR_MODE_TABLET
// output none
// return none


// 选择的秒数
#define EEVT_TS_SELECT		EMSG_TS_REQUEST(1,8)			
// input ULONG  多少秒
// output none
// return none

// 选择的键盘样式
#define EEVT_KEYBOARD_SELECT		EMSG_TS_REQUEST(1,9)			
// input ULONG  选择的第几个
// output none
// return none


// 封面选择变化
#define EEVT_COVER_SELECT		EMSG_TS_REQUEST(1,10)			
// input ULONG  1表示图片 2表示议程
// output none
// return none


// 要求预览图片
#define EEVT_PICTURE_PREVIEW		EMSG_TS_REQUEST(1,11)			
// input wchar_t*  图片的相对路径
// output none
// return none

// 预览图片界面OK按钮
#define EEVT_PICTURE_PREVIEW_EXIT		EMSG_TS_REQUEST(1,12)			
// input ULONG 1表示确定 0表示取消
// output none
// return none

// 获取配置文件指针
#define EEVT_GET_SETTING_FILE		EMSG_TS_REQUEST(1,13)			
// input none
// output XmlDocument*
// return none