
/* 本代码是CRC校验，油门发送的数据中需要用到CRC校验，通过串口读写
 * 2016.10.16
 */
#define uchar unsigned char
unsigned short CRC16(uchar *puchMsg, unsigned short usDataLen, uchar *puchResult);