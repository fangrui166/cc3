
STM32F103RCT6:
Flash: 256K  Ram:48K

CC_C : Cloud Charger Contorler
CC_I : Cloud Charger Input
CC_O : Cloud Charger Output

资源定义： CC_C
UART1 --> show log and shell
UART2 --> communication with SIM900A
UART3 --> communication with CC_I
UART4 --> communication with CC-O1
UART5 --> communication with CC-O2

服务器：
121.41.105.74
administrator
密码：abnHMgj2NNHaybnHzMgVj2NYNH
充电

 tcpdump -i eth1 icmp

cmd：
开关机
FOTA升级
开始充电
结束充电
心跳
硬件异常
充电度数
#充电时长
充电端口
设备ID

