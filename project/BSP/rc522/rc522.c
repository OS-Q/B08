#include <time.h>
#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"
#include "rc522.h"
#include "app_cfg.h"
#include "delay.h"
#include "debug.h"
#include "timer2.h"
#include "rc522_task.h"
#include "led_task.h"
#include "rtc_config.h"

#define MAXRLEN 18

extern unsigned char device_id[4];

u8 device_id_count = 0;


/////////////////////////////////////////////////////////////////////
//功    能：寻卡
//参数说明: req_code[IN]:寻卡方式
//                0x52 = 寻感应区内所有符合14443A标准的卡
//                0x26 = 寻未进入休眠状态的卡
//          pTagType[OUT]：卡片类型代码
//                0x4400 = Mifare_UltraLight
//                0x0400 = Mifare_One(S50)
//                0x0200 = Mifare_One(S70)
//                0x0800 = Mifare_Pro(X)
//                0x4403 = Mifare_DESFire
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdRequest(unsigned char req_code,unsigned char *pTagType)
{
   char status;  
   unsigned int  unLen;
   unsigned char ucComMF522Buf[MAXRLEN]; 
   //unsigned char xTest ;
   ClearBitMask(Status2Reg,0x08);
   WriteRawRC(BitFramingReg,0x07);

//  xTest = ReadRawRC(BitFramingReg);
//  if(xTest == 0x07 )
 //   { LED_GREEN  =0 ;}
 // else {LED_GREEN =1 ;while(1){}}
   SetBitMask(TxControlReg,0x03);
 
   ucComMF522Buf[0] = req_code;

   status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,1,ucComMF522Buf,&unLen);
//     if(status  == MI_OK )
//   { LED_GREEN  =0 ;}
//   else {LED_GREEN =1 ;}
   if ((status == MI_OK) && (unLen == 0x10))
   {    
       *pTagType     = ucComMF522Buf[0];
       *(pTagType+1) = ucComMF522Buf[1];
   }
   else
   {   status = MI_ERR;   }
   
   return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：防冲撞
//参数说明: pSnr[OUT]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////  
char PcdAnticoll(unsigned char *pSnr)
{
    char status;
    unsigned char i,snr_check=0;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    

    ClearBitMask(Status2Reg,0x08);
    WriteRawRC(BitFramingReg,0x00);
    ClearBitMask(CollReg,0x80);
 
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x20;

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,2,ucComMF522Buf,&unLen);

    if (status == MI_OK)
    {
    	 for (i=0; i<4; i++)
         {   
             *(pSnr+i)  = ucComMF522Buf[i];
             snr_check ^= ucComMF522Buf[i];
         }
         if (snr_check != ucComMF522Buf[i])
         {   status = MI_ERR;    }
    }
    
    SetBitMask(CollReg,0x80);
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：选定卡片
//参数说明: pSnr[IN]:卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdSelect(unsigned char *pSnr)
{
    char status;
    unsigned char i;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_ANTICOLL1;
    ucComMF522Buf[1] = 0x70;
    ucComMF522Buf[6] = 0;
    for (i=0; i<4; i++)
    {
    	ucComMF522Buf[i+2] = *(pSnr+i);
    	ucComMF522Buf[6]  ^= *(pSnr+i);
    }
    CalulateCRC(ucComMF522Buf,7,&ucComMF522Buf[7]);
  
    ClearBitMask(Status2Reg,0x08);

    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,9,ucComMF522Buf,&unLen);
    
    if ((status == MI_OK) && (unLen == 0x18))
    {   status = MI_OK;  }
    else
    {   status = MI_ERR;    }

    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：验证卡片密码
//参数说明: auth_mode[IN]: 密码验证模式
//                 0x60 = 验证A密钥
//                 0x61 = 验证B密钥 
//          addr[IN]：块地址
//          pKey[IN]：密码
//          pSnr[IN]：卡片序列号，4字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////               
char PcdAuthState(unsigned char auth_mode,unsigned char addr,unsigned char *pKey,unsigned char *pSnr)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = auth_mode;
    ucComMF522Buf[1] = addr;
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+2] = *(pKey+i);   }
    for (i=0; i<6; i++)
    {    ucComMF522Buf[i+8] = *(pSnr+i);   }
 //   memcpy(&ucComMF522Buf[2], pKey, 6); 
 //   memcpy(&ucComMF522Buf[8], pSnr, 4); 
    
    status = PcdComMF522(PCD_AUTHENT,ucComMF522Buf,12,ucComMF522Buf,&unLen);
    if ((status != MI_OK) || (!(ReadRawRC(Status2Reg) & 0x08)))
    {   status = MI_ERR;   }
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：读取M1卡一块数据
//参数说明: addr[IN]：块地址
//          pData[OUT]：读出的数据，16字节
//返    回: 成功返回MI_OK
///////////////////////////////////////////////////////////////////// 
char PcdRead(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_READ;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
   
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);
    if ((status == MI_OK) )//&& (unLen == 0x90))
 //   {   memcpy(pData, ucComMF522Buf, 16);   }
    {
        for (i=0; i<16; i++)
        {    *(pData+i) = ucComMF522Buf[i];   }
    }
    else
    {   
			printf("func:%s, status=%d, unLen=0x%x\n",__func__,status,unLen);
			status = MI_ERR;
		}
    
    return status;
}

/////////////////////////////////////////////////////////////////////
//功    能：写数据到M1卡一块
//参数说明: addr[IN]：块地址
//          pData[IN]：写入的数据，16字节
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////                  
char PcdWrite(unsigned char addr,unsigned char *pData)
{
    char status;
    unsigned int  unLen;
    unsigned char i,ucComMF522Buf[MAXRLEN]; 
    
    ucComMF522Buf[0] = PICC_WRITE;
    ucComMF522Buf[1] = addr;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
    {   status = MI_ERR;   }
        
    if (status == MI_OK)
    {
        //memcpy(ucComMF522Buf, pData, 16);
        for (i=0; i<16; i++)
        {    ucComMF522Buf[i] = *(pData+i);   }
        CalulateCRC(ucComMF522Buf,16,&ucComMF522Buf[16]);

        status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,18,ucComMF522Buf,&unLen);
        if ((status != MI_OK) || (unLen != 4) || ((ucComMF522Buf[0] & 0x0F) != 0x0A))
        {   status = MI_ERR;   }
    }
    
    return status;
}



/////////////////////////////////////////////////////////////////////
//功    能：命令卡片进入休眠状态
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdHalt(void)
{
    char status;
    unsigned int  unLen;
    unsigned char ucComMF522Buf[MAXRLEN]; 

    ucComMF522Buf[0] = PICC_HALT;
    ucComMF522Buf[1] = 0;
    CalulateCRC(ucComMF522Buf,2,&ucComMF522Buf[2]);
 
    status = PcdComMF522(PCD_TRANSCEIVE,ucComMF522Buf,4,ucComMF522Buf,&unLen);

    return status;
}

/////////////////////////////////////////////////////////////////////
//用MF522计算CRC16函数
/////////////////////////////////////////////////////////////////////
void CalulateCRC(unsigned char *pIndata,unsigned char len,unsigned char *pOutData)
{
    unsigned char i,n;
    ClearBitMask(DivIrqReg,0x04);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    for (i=0; i<len; i++)
    {   WriteRawRC(FIFODataReg, *(pIndata+i));   }
    WriteRawRC(CommandReg, PCD_CALCCRC);
    i = 0xFF;
    do 
    {
        n = ReadRawRC(DivIrqReg);
        i--;
    }
    while ((i!=0) && !(n&0x04));
    pOutData[0] = ReadRawRC(CRCResultRegL);
    pOutData[1] = ReadRawRC(CRCResultRegM);
}

/////////////////////////////////////////////////////////////////////
//功    能：复位RC522
//返    回: 成功返回MI_OK
/////////////////////////////////////////////////////////////////////
char PcdReset(void)
{
	CLR_RC522_NSS;

	SET_RC522_RST;
	delay_10ms(1);
	CLR_RC522_RST;
	delay_10ms(1);
	SET_RC522_RST;
	delay_10ms(1);

	WriteRawRC(CommandReg,PCD_RESETPHASE);
	delay_10ms(1);

	WriteRawRC(ModeReg,0x3D);            //和Mifare卡通讯，CRC初始值0x6363
	WriteRawRC(TReloadRegL,30);           
	WriteRawRC(TReloadRegH,0);
	WriteRawRC(TModeReg,0x8D);
	WriteRawRC(TPrescalerReg,0x3E);
	WriteRawRC(TxAutoReg,0x40);     
	return MI_OK;
}
//////////////////////////////////////////////////////////////////////
//设置RC632的工作方式 
//////////////////////////////////////////////////////////////////////
char M500PcdConfigISOType(unsigned char type)
{
   if (type == 'A')                     //ISO14443_A
   { 
       ClearBitMask(Status2Reg,0x08);

 /*     WriteRawRC(CommandReg,0x20);    //as default   
       WriteRawRC(ComIEnReg,0x80);     //as default
       WriteRawRC(DivlEnReg,0x0);      //as default
	   WriteRawRC(ComIrqReg,0x04);     //as default
	   WriteRawRC(DivIrqReg,0x0);      //as default
	   WriteRawRC(Status2Reg,0x0);//80    //trun off temperature sensor
	   WriteRawRC(WaterLevelReg,0x08); //as default
       WriteRawRC(ControlReg,0x20);    //as default
	   WriteRawRC(CollReg,0x80);    //as default
*/
       WriteRawRC(ModeReg,0x3D);//3F
/*	   WriteRawRC(TxModeReg,0x0);      //as default???
	   WriteRawRC(RxModeReg,0x0);      //as default???
	   WriteRawRC(TxControlReg,0x80);  //as default???

	   WriteRawRC(TxSelReg,0x10);      //as default???
   */
       WriteRawRC(RxSelReg,0x86);//84
 //      WriteRawRC(RxThresholdReg,0x84);//as default
 //      WriteRawRC(DemodReg,0x4D);      //as default

 //      WriteRawRC(ModWidthReg,0x13);//26
       WriteRawRC(RFCfgReg,0x7F);   //4F
	/*   WriteRawRC(GsNReg,0x88);        //as default???
	   WriteRawRC(CWGsCfgReg,0x20);    //as default???
       WriteRawRC(ModGsCfgReg,0x20);   //as default???
*/
   	   WriteRawRC(TReloadRegL,30);//tmoLength);// TReloadVal = 'h6a =tmoLength(dec) 
	   WriteRawRC(TReloadRegH,0);
       WriteRawRC(TModeReg,0x8D);
	   WriteRawRC(TPrescalerReg,0x3E);
	   

  //     PcdSetTmo(106);
	   delay_10ms(1);
       PcdAntennaOn();
   }
   else{ return MI_NOTAGERR; }
   
   return MI_OK;
}

/////////////////////////////////////////////////////////////////////
//功    能：读RC632寄存器
//参数说明：Address[IN]:寄存器地址
//返    回：读出的值
/////////////////////////////////////////////////////////////////////
unsigned char ReadRawRC(unsigned char Address)
{
     unsigned char i, ucAddr;
     unsigned char ucResult=0;

     CLR_RC522_NSS;
     ucAddr = ((Address<<1)&0x7E)|0x80;

     for(i=8;i>0;i--)
     {
         CLR_RC522_SCK;
	 	 if(ucAddr&0x80)
		 {
         	SET_RC522_MOSI;
		 }
		 else
		 {
		 	CLR_RC522_MOSI;
		 }
         SET_RC522_SCK;
         ucAddr <<= 1;
     }

     for(i=8;i>0;i--)
     {
         CLR_RC522_SCK;
         ucResult <<= 1;
         SET_RC522_SCK;
			 
		 if(GET_RC522_MISO)
         	ucResult |= 1;
     }

     SET_RC522_NSS;
     SET_RC522_SCK;
     return ucResult;
}

/////////////////////////////////////////////////////////////////////
//功    能：写RC632寄存器
//参数说明：Address[IN]:寄存器地址
//          value[IN]:写入的值
/////////////////////////////////////////////////////////////////////
void WriteRawRC(unsigned char Address, unsigned char value)
{  
    unsigned char i, ucAddr;

    CLR_RC522_SCK;
    CLR_RC522_NSS;
    ucAddr = ((Address<<1)&0x7E);

    for(i=8;i>0;i--)
    {
		if(ucAddr&0x80)
		{
        	SET_RC522_MOSI;
		}
		else
		{
			CLR_RC522_MOSI;
		}
        SET_RC522_SCK;
        ucAddr <<= 1;
        CLR_RC522_SCK;
    }

    for(i=8;i>0;i--)
    {
		if(value&0x80)
		{
        	SET_RC522_MOSI;
		}
		else
		{
			CLR_RC522_MOSI;
		}
        SET_RC522_SCK;
        value <<= 1;
        CLR_RC522_SCK;
    }
    SET_RC522_NSS;
    SET_RC522_SCK;
}

/////////////////////////////////////////////////////////////////////
//功    能：置RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:置位值
/////////////////////////////////////////////////////////////////////
void SetBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg,tmp | mask);  // set bit mask
}

/////////////////////////////////////////////////////////////////////
//功    能：清RC522寄存器位
//参数说明：reg[IN]:寄存器地址
//          mask[IN]:清位值
/////////////////////////////////////////////////////////////////////
void ClearBitMask(unsigned char reg,unsigned char mask)  
{
    char tmp = 0x0;
    tmp = ReadRawRC(reg);
    WriteRawRC(reg, tmp & ~mask);  // clear bit mask
} 

/////////////////////////////////////////////////////////////////////
//功    能：通过RC522和ISO14443卡通讯
//参数说明：Command[IN]:RC522命令字
//          pInData[IN]:通过RC522发送到卡片的数据
//          InLenByte[IN]:发送数据的字节长度
//          pOutData[OUT]:接收到的卡片返回数据
//          *pOutLenBit[OUT]:返回数据的位长度
/////////////////////////////////////////////////////////////////////
char PcdComMF522(unsigned char Command, 
                 unsigned char *pInData, 
                 unsigned char InLenByte,
                 unsigned char *pOutData, 
                 unsigned int  *pOutLenBit)
{
    char status = MI_ERR;
    unsigned char irqEn   = 0x00;
    unsigned char waitFor = 0x00;
    unsigned char lastBits;
    unsigned char n;
    unsigned int i;
    switch (Command)
    {
       case PCD_AUTHENT:
          irqEn   = 0x12;
          waitFor = 0x10;
          break;
       case PCD_TRANSCEIVE:
          irqEn   = 0x77;
          waitFor = 0x30;
          break;
       default:
         break;
    }
   
    WriteRawRC(ComIEnReg,irqEn|0x80);
    ClearBitMask(ComIrqReg,0x80);
    WriteRawRC(CommandReg,PCD_IDLE);
    SetBitMask(FIFOLevelReg,0x80);
    
    for (i=0; i<InLenByte; i++)
    {   WriteRawRC(FIFODataReg, pInData[i]);    }
    WriteRawRC(CommandReg, Command);
   
    
    if (Command == PCD_TRANSCEIVE)
    {    SetBitMask(BitFramingReg,0x80);  }
    
//    i = 600;//根据时钟频率调整，操作M1卡最大等待时间25ms
    i = 2000;
    do 
    {
         n = ReadRawRC(ComIrqReg);
         i--;
    }
    while ((i!=0) && !(n&0x01) && !(n&waitFor));
    ClearBitMask(BitFramingReg,0x80);
	      
    if (i!=0)
    {    
         if(!(ReadRawRC(ErrorReg)&0x1B))
         {
             status = MI_OK;
             if (n & irqEn & 0x01)
             {   status = MI_NOTAGERR;   }
             if (Command == PCD_TRANSCEIVE)
             {
               	n = ReadRawRC(FIFOLevelReg);
              	lastBits = ReadRawRC(ControlReg) & 0x07;
                if (lastBits)
                {   *pOutLenBit = (n-1)*8 + lastBits;   }
                else
                {   *pOutLenBit = n*8;   }
                if (n == 0)
                {   n = 1;    }
                if (n > MAXRLEN)
                {   n = MAXRLEN;   }
                for (i=0; i<n; i++)
                {   pOutData[i] = ReadRawRC(FIFODataReg);    }
            }
         }
         else
         {   status = MI_ERR;   }
        
   }
   

   SetBitMask(ControlReg,0x80);           // stop timer now
   WriteRawRC(CommandReg,PCD_IDLE); 
   return status;
}


/////////////////////////////////////////////////////////////////////
//开启天线  
//每次启动或关闭天险发射之间应至少有1ms的间隔
/////////////////////////////////////////////////////////////////////
void PcdAntennaOn()
{
    unsigned char i;
    i = ReadRawRC(TxControlReg);
    if (!(i & 0x03))
    {
        SetBitMask(TxControlReg, 0x03);
    }
}


/////////////////////////////////////////////////////////////////////
//关闭天线
/////////////////////////////////////////////////////////////////////
void PcdAntennaOff()
{
    ClearBitMask(TxControlReg, 0x03);
}

//等待卡离开
void WaitCardOff(void)
{
	char status;
	unsigned char TagType[2];

	while(1)
	{
		status = PcdRequest(REQ_ALL, TagType);
		if(status)
		{
			status = PcdRequest(REQ_ALL, TagType);
			if(status)
			{
				status = PcdRequest(REQ_ALL, TagType);
				if(status)
				{
					return;
				}
			}
		}
		delay_10ms(1);
	}
}

///////////////////////////////////////////////////////////////////////
// Delay 10ms
///////////////////////////////////////////////////////////////////////
void delay_10ms(unsigned int _10ms)
{
	unsigned int i;

	for(i=0; i<_10ms; i++)
	{
    delay_ms(10);
	}
}

unsigned char CardNum[4];
unsigned char TagType[2];
void rc522_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);  //使能端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	SET_RC522_NSS;
	SET_RC522_MOSI;
	SET_RC522_SCK;
	SET_RC522_RST;
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}

u32 u8_to_u32(u8 *value)
{
	return (((value[0]&0xFF)<<24)+((value[1]&0xFF)<<16)+((value[2]&0xFF)<<8)+(value[3]&0xFF));
}

//读取卡号
unsigned char ReadCardFuntion(void)
{
  //unsigned char KEY_A[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  unsigned char KEY_A[6]={0x4C,0x4F,0x50,0x45,0x47,0x4F};
  //unsigned char KEY_B[6]={0xff,0xff,0xff,0xff,0xff,0xff};
  unsigned char RFID[16];
  u8 sector_count;
  u8 device_id_keys;
  u8 m_device_id[4];
  u8 i,j;
  int k;
  uint32_t cardId,m_time,l_time;

	//寻卡请求
	if(MI_OK != PcdRequest(PICC_REQIDL,TagType))
	{
		return MI_ERR;
	}
	//防冲突
	if(MI_OK != PcdAnticoll(CardNum) )
	{
		return MI_ERR;
	}
	//选卡
	if(MI_OK != PcdSelect(CardNum) )
	{
		return MI_ERR;
	}
  
  cardId = u8_to_u32(CardNum);
	DBG_INFO("got ID=0x%08x\n",cardId);
  if(blackNameID[0] != 0)
  {
    uint8_t m;
    for(m = 1; m <= blackNameID[0]; m++)
    {
      if(cardId == blackNameID[m])
      {  
        DBG_INFO("match black id:0x%08x\n",blackNameID[m]);
        goto out;
      }  
    }
  } 
  
  //密码A验证
	if(MI_OK != PcdAuthState(PICC_AUTHENT1A,START_SECTOR_ONE,KEY_A,CardNum) )
	{
		DBG_ERROR("PICC_AUTHENT1A error\n");
    //return MI_ERR;
    goto out;
	}
/*  
  //密码B验证
	if(MI_OK != PcdAuthState(PICC_AUTHENT1B,START_SECTOR_ONE,KEY_B,CardNum) )
	{
		DBG_ERROR("PICC_AUTHENT1B error\n");
    //return MI_ERR;
	}
*/ 
  //读卡
	if(MI_OK != PcdRead(START_SECTOR_ONE+1,RFID) )
	{
		DBG_ERROR("read ikey plus control data error\n");
    //return MI_ERR;
    goto out;
	}else{
    DBG_INFO("%c%c%c%c\n",RFID[0],RFID[1],RFID[2],RFID[3]);
    if(RFID[0] != 'L' || RFID[1] != 'O' || RFID[2] != 'P' || RFID[3] != 'E')
    {
      DBG_ERROR(" warn!!!!  illegal card!!!!!\n");
      goto out;
    }  
    device_id_count = RFID[4];
    device_id_keys = RFID[5];
    //m_time = u8_to_u32(RFID+6);
    l_time = u8_to_u32(RFID+10);
    if(l_time != 0)
    {  
      get_rtc_time(&m_time);
      #if 0
      {
        struct tm *tm;
        tm=localtime(&l_time);
        tm->tm_year +=1900;
        tm->tm_mon++;
        DBG_INFO("card deadline: %04d-%02d-%02d %02d:%02d:%02d week=%d\n",tm->tm_year,tm->tm_mon,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec,tm->tm_wday);
      }  
      #endif
      if(l_time < m_time)
      {
        DBG_INFO("this card is over the deadline!\n");
        goto out;
      }  
    }  
    m_device_id[0] = device_id[0] ^ device_id_keys;
    m_device_id[1] = device_id[1] ^ device_id_keys;
    m_device_id[2] = device_id[2] ^ device_id_keys;
    m_device_id[3] = device_id[3] ^ device_id_keys;
    
    sector_count = device_id_count / 12;
    DBG_INFO("sector_count :%d RFIF[4] = %d\n",sector_count,device_id_count);
    for(i = 1; i <= sector_count; i++)
    {
      DBG_INFO("sector index: %d\n",i);
      //密码A验证
      if(MI_OK != PcdAuthState(PICC_AUTHENT1A,4*i,KEY_A,CardNum) )
      {
        DBG_ERROR("authenticate block:%d key A error\n",4*i);
        //return MI_ERR;
        goto out;
      }else{
        for(j = 0; j < 3; j++)
        {
          //读卡
          if(MI_OK != PcdRead(4*i+j,RFID) )
          {
            DBG_ERROR("read block:%d error\n",4*i+j);
            //return MI_ERR;
            goto out;
          }else{
            for(k = 0; k < 4; k++)
            {
              DBG_INFO("%02x %02x %02x %02x ",RFID[4*k],RFID[4*k+1],RFID[4*k+2],RFID[4*k+3]);
              if(m_device_id[0] == RFID[4*k] && m_device_id[1] == RFID[4*k+1] && 
                 m_device_id[2] == RFID[4*k+2] && m_device_id[3] == RFID[4*k+3])
              {
                DBG_INFO("\n");
                PcdHalt();
                return MI_OK;
              }
            }
            DBG_INFO("\n");
          }
        }
      }
    }
  }
    
  sector_count = device_id_count % 12;
  if(sector_count != 0)
  {
    DBG_INFO("sector index: %d\n",i);
    //密码A验证
    if(MI_OK != PcdAuthState(PICC_AUTHENT1A,4*i,KEY_A,CardNum) )
    {
      DBG_ERROR("authenticate block:%d key A error\n",4*i);
      //return MI_ERR;
      goto out;
    }else{
      for(j = 0; j < 3; j++)
      {
        //读卡
        if(MI_OK != PcdRead(4*i+j,RFID) )
        {
          DBG_ERROR("read block:%d error\n",4*i+j);
          //return MI_ERR;
          goto out;
        }else{
          for(k = 0; k < 4; k++)
          {
            DBG_INFO("%02x %02x %02x %02x ",RFID[4*k],RFID[4*k+1],RFID[4*k+2],RFID[4*k+3]);
            if(m_device_id[0] == RFID[4*k] && m_device_id[1] == RFID[4*k+1] && 
               m_device_id[2] == RFID[4*k+2] && m_device_id[3] == RFID[4*k+3])
            {
              DBG_INFO("\n");
              PcdHalt();
              return MI_OK;
            }
            sector_count--;
            if(sector_count <= 0)
            {  
              DBG_INFO("\n");
              //goto out;
            }  
          }
        }
        DBG_INFO("\n");
      }
    }  
  }
  PcdHalt();
  HalLedSet(HAL_LED_MODE_FLASH_FAST);
  beep_start(250);
  delay_ms(300);
  beep_start(250);
  delay_ms(300);
  beep_start(250);
  delay_ms(300);
  HalLedSet(HAL_LED_MODE_FLASH_SLOW);
  return MI_NOTAGERR;

	//bell_start(1,2);//读到卡片响一声
out:	
	PcdHalt();
  HalLedSet(HAL_LED_MODE_FLASH_FAST);
  beep_start(250);
  delay_ms(300);
  beep_start(250);
  delay_ms(300);
  beep_start(250);
  delay_ms(300);
  HalLedSet(HAL_LED_MODE_FLASH_SLOW);
  return MI_ERR;
}

