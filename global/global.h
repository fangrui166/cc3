#ifndef __GLOBAL_H__
#define __GLOBAL_H__



typedef enum{
    MP3_INDEX_WELCOME_TO_USE,                // ������ʾ��
    MP3_INDEX_SECURITY_CODE_USED,            // ��֤���Ѿ�ʹ�ù���
    MP3_INDEX_SECURITY_CODE_RETRY_OVERRUN,   // ��֤�����Գ���
    MP3_INDEX_SECURITY_CODE_RETRY_TIPS,      // ��֤������������� 5�Σ� ��� 10���Ӻ�����
    MP3_INDEX_SECURITY_CODE_DISABLE,         // �������빦�ܲ�����
    MP3_INDEX_INPUT_ERROR,                   // ��������
    MP3_INDEX_INPUT_CHARGER_PORT,            // ��������˿ں�
    MP3_INDEX_CHARGER_PORT_BUSY,             // ���˿��Ѿ���ռ��
    MP3_INDEX_INPUT_TIPS,                    // ����ˢ����������֤�룬Ȼ��������˿ں�
    MP3_INDEX_CHARGE_PORT,                   // �˿�(XX, XX)
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
    MP3_INDEX_CHARGE_FULL,                   // (�˿�xx)�ѳ���
    MP3_INDEX_CHARGE_OVERLOAD,               // (�˿�xx)��������
    MP3_INDEX_CHARGE_STOP,                   // (�˿�xx)ֹͣ���
    MP3_INDEX_BRUSHCARD,                     // ˢ���� "����"
    MP3_INDEX_RFID_CARD_UNKOWN,              // rfid ��֤δ֪����
    MP3_INDEX_RFID_CARD_NO_BALANCE,          // rfid ��֤����
    MP3_INDEX_RFID_CARD_NO_REGISTER,         // rfid ��֤��δע��
    MP3_INDEX_RFID_WAITE_RESPONSE_TIMEOUT,   // rfid ��֤�ȴ����糬ʱ��������ˢ��

}GD5820_MP3_INDEX;

typedef enum{
    RFID_CARD_UNKOWN = 1,          // δ֪����
    RFID_CARD_OK,
    RFID_CARD_NO_BALANCE,      // ����
    RFID_CARD_NO_REGISTER,     // ��ûע��
}rfid_response;

/*
*
*   ���� CC_I , CC_C , CC_O ֮�����ݴ�������
*
*/
#define  SET_CC_O_ADDR  0x01
typedef enum{
    I2C_SET_CC_O_ADDR = 0x01,   // I2C --> CC_I  Send cmd to  CC_C
    I2C_CHARGE_OFFLINE,
    I2C_CHARGE_ONLINE,
    I2C_SELECT_CHARGE_PORT,
    I2C_GET_DEVICE_ID,
    I2C_SEND_RF_CARD_ID,        // ���� RFID ����

    C2I_PLAY_MP3,
    C2I_SET_SECURITY_CODE_USED,  // ����֤����Ϊ��ʹ��
    C2I_HW_INSPECTION,
    C2I_SYNC_DEVICE_ID,
    C2I_RF_CARD_ID_RESP,         // �ظ� RFID ��֤���

    C2O_SEND_CHARGER_DATA,       // ���ͳ������� CC_O
    C2O_SET_CHARGER_PORT_STOP,   // �رճ��˿�
    C2O_HW_INSPECTION,

    O2C_PLAY_MP3,
    O2C_SEND_CHARGER_PORT_DATA,  // ���Ͷ˿ڳ�����ݵ� CC_C
    O2C_SET_PORT_CHARGE_STATE,   // ���ö˿ڳ��״̬�� busy ��free
    O2C_SYNC_PORT_STATE,

    C2C_HW_INSPECTION,
}CC_C_CMD_TYPE;

#endif
