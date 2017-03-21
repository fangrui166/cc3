#ifndef __GLOBAL_H__
#define __GLOBAL_H__



typedef enum{
    MP3_INDEX_WELCOME_TO_USE,                // 开机提示音
    MP3_INDEX_SECURITY_CODE_USED,            // 验证码已经使用过了
    MP3_INDEX_SECURITY_CODE_RETRY_OVERRUN,   // 验证码重试超限
    MP3_INDEX_SECURITY_CODE_RETRY_TIPS,      // 验证码输入次数超过 5次， 请等 10分钟后再试
    MP3_INDEX_SECURITY_CODE_DISABLE,         // 离线密码功能不开放
    MP3_INDEX_INPUT_ERROR,                   // 输入有误
    MP3_INDEX_INPUT_CHARGER_PORT,            // 请输入充电端口号
    MP3_INDEX_CHARGER_PORT_BUSY,             // 充电端口已经被占用
    MP3_INDEX_INPUT_TIPS,                    // 请先刷卡或输入验证码，然后再输入端口号
    MP3_INDEX_CHARGE_PORT,                   // 端口(XX, XX)
    MP3_INDEX_NUM0,
    MP3_INDEX_NUM1,
    MP3_INDEX_NUM2,
    MP3_INDEX_NUM3,
    MP3_INDEX_NUM4,
    MP3_INDEX_NUM5,
    MP3_INDEX_NUM6,
    MP3_INDEX_NUM7,
    MP3_INDEX_NUM8,
    MP3_INDEX_NUM9,
    MP3_INDEX_CHARGE_FULL,                   // (端口xx)已充满
    MP3_INDEX_CHARGE_OVERLOAD,               // (端口xx)电流过大
    MP3_INDEX_CHARGE_STOP,                   // (端口xx)停止充电
    MP3_INDEX_BRUSHCARD,                     // 刷卡声 "哔哔"
    MP3_INDEX_RFID_CARD_UNKOWN,              // rfid 验证未知错误
    MP3_INDEX_RFID_CARD_NO_BALANCE,          // rfid 验证余额不足
    MP3_INDEX_RFID_CARD_NO_REGISTER,         // rfid 验证卡未注册
    MP3_INDEX_RFID_WAITE_RESPONSE_TIMEOUT,   // rfid 验证等待网络超时，请重新刷卡

}GD5820_MP3_INDEX;

typedef enum{
    RFID_CARD_UNKOWN = 1,          // 未知错误
    RFID_CARD_OK,
    RFID_CARD_NO_BALANCE,      // 余额不足
    RFID_CARD_NO_REGISTER,     // 卡没注册
}rfid_response;

/*
*
*   定义 CC_I , CC_C , CC_O 之间数据传输命令
*
*/
#define  SET_CC_O_ADDR  0x01
typedef enum{
    I2C_SET_CC_O_ADDR = 0x01,   // I2C --> CC_I  Send cmd to  CC_C
    I2C_CHARGE_OFFLINE,
    I2C_CHARGE_ONLINE,
    I2C_SELECT_CHARGE_PORT,
    I2C_GET_DEVICE_ID,
    I2C_SEND_RF_CARD_ID,        // 发送 RFID 卡号

    C2I_PLAY_MP3,
    C2I_SET_SECURITY_CODE_USED,  // 将验证码设为已使用
    C2I_HW_INSPECTION,
    C2I_SYNC_DEVICE_ID,
    C2I_RF_CARD_ID_RESP,         // 回复 RFID 验证结果

    C2O_SEND_CHARGER_DATA,       // 发送充电参数到 CC_O
    C2O_SET_CHARGER_PORT_STOP,   // 关闭充电端口
    C2O_HW_INSPECTION,

    O2C_PLAY_MP3,
    O2C_SEND_CHARGER_PORT_DATA,  // 发送端口充电数据到 CC_C
    O2C_SET_PORT_CHARGE_STATE,   // 设置端口充电状态， busy 、free
    O2C_SYNC_PORT_STATE,

    C2C_HW_INSPECTION,
}CC_C_CMD_TYPE;

#endif
