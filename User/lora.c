#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include "arm_comm.h"

#include "radio/radio.h"


uint16_t	LoRa_Channel [20] = {
9173, 			// 0
9179,			// 1
9185,			// 2
9191,			// 3
9197,			// 4
9203,			// 5
9207,			// 6
9209,			// 7
9211,			// 8
9213,			// 9
9215,			// 10
9217,			// 11
9219,			// 12
9221,			// 13
9223,			// 14
9225,			// 15
9227,			// 16
9229,			// 17
9231,			// 18
9233			// 19
};


tRadioDriver *Radio = NULL;
double SYS_PacketRssiValue;
uint8_t		SYS_RTx_State = _Rx_;


//------------------------------------------------------< LoRa >
struct Lora_Tx_Table	Tx_Table [MAX_Tx_TABLE_EA];
struct Lora_Rx_Table	Rx_Table [MAX_Rx_TABLE_EA];
struct Lora_Packet 		Lpkt;
uint8_t		Lora_Rx_Buff [100];


//------------------------------------------------------------------------------
// CID(2)+Brg(1)+Cmd(1)+Len(1)+Payload
//                   		   SID(3)+Speep(1)+Opt(1)+Bridge(1)+Reg(2)+Payload

void LoRa_Proc(void)
{
uint16_t crc;
uint8_t len, ret;
int8_t	Brg_Ea, Brg_No;
int32_t no;

	len = Get_LoRa ();
	if (len < LORA_HEADER_LEN) return;


	Lora_Rx_Buff[4] &= 0x3f;										// Seq 번호 제거, ACT 인 경우 코디가 3번 보내는것을 별개로 인식하여, 깜박거림
	len = Lora_Rx_Buff[4];

	if (len > 0 && len < 16) len = 16;						// AES 최소 크기
	if (len+LORA_HEADER_LEN != Rx_Len) return;

	Lora_Recv_Timer = Time_1sec_Count + SB_LORA_TIMEOUT;		// 60 분

	crc = Get_CRC16 (&Lora_Rx_Buff[3], Rx_Len-3);			// cmd ~ PAyload 만 CRC
	//-----------------------------------------------------------<저장된 패킷 중에 같은것이 있으면 무시한다
	for (no=0; no<MAX_Rx_TABLE_EA; no++)
		{
		if (Rx_Tbl.Len == 0) continue;
		if (Rx_Tbl.Crc == crc && Rx_Tbl.Len == Rx_Len) 		// 이미 수신한 패킷
			{
			_DBG_ ADN_Printf ("Same packet");
			return;
			}
		}

	//----------------------------------------------------------<테이블에 저장한다
	Add_Rx_Table (crc, Rx_Len);

	Brg_Ea  = (Lora_Rx_Buff[2] & 0x70) >> 4;				// 전체 브릿지 갯수에서 해당 노드/코디가 전송시 브릿징을 요청한 브릿지 갯수
	Brg_No  = (Lora_Rx_Buff[2] & 0x07);						// 내가 패킷을 처음 받은 브리지 번호 (응답시 남은 브리지 이후에 전송), 0이면 코디/노드에서 바로 받음
	if (Brg_Ea < Brg_No) Brg_No = Brg_Ea;					// 만약을 대비 - 값이 안나오도록
	Rx_Rssi_Val = (uint8_t)0xff - (uint8_t)SYS_PacketRssiValue;

	//-----------------------------------------------------// 노드가 보낸 패킷
	if ((Lora_Rx_Buff[3] & 0x80) == 0x80)
		{
		if (Lora_Rx_Buff[3] == NODE_EVENT)
			{
			Delete_Response_Packet ();						// 노드로 부터 이벤트 수신시 전송패킷내 폴 응답 제거
			}

		if (My_Bridge_No)
			{
			no = Check_Bridge (Brg_Ea, Brg_No, 'N');
			if (no != 100) Add_Tx_Table_Brg (no, Rx_Len, Lora_Rx_Buff, Rx_Len);
			}
		return;
		}

#if 1
	memcpy (Lpkt.Cid, Lora_Rx_Buff, LORA_HEADER_LEN);
	AES128_CBC_Decrypt ((uint8_t *)&Lpkt.Payload, &Lora_Rx_Buff[LORA_HEADER_LEN], len);
#else
	memcpy (Lpkt.Cid, Lora_Rx_Buff, Rx_Len);
#endif

	if (My_Bridge_No)										// 코디가 보내고, 목적지가 브로드케스트 또는 다른노드이고, 내가 브리지이면
		{
		no = Check_Bridge (Brg_Ea, Brg_No, 'C');
		if (no != 100) Add_Tx_Table_Brg (no, Rx_Len, Lora_Rx_Buff, Rx_Len);
		}

//	Lpkt.Len &= 0x3f;										// Seq 번호 제거

	_DBG_ ADN_Hexprint ("LRx", Lpkt.Cid, Lpkt.Len+LORA_HEADER_LEN);
//	_DBG_ ADN_Printf ("Req(Ea=%d, Rx=%d), My(Tot=%d, My_B=%d), Rx_B_No=%d, Rssi=%d", Brg_Ea, Brg_No, Total_Bridge_Ea, My_Bridge_No, Rx_Bridge_No, Rx_Rssi_Val);

	switch (Lpkt.Cmd)
		{
		case CODY_POLL :	//	Config
			if (memcmp (Lpkt.Payload, CFG.Id, 3) != 0) break;	// 내가 아니면 리턴,  브로드케스트는 안됨
			Rx_Bridge_No = Brg_Ea;									// 코디가 보낸 브리지 요청 갯수
			Check_Config (Lpkt.Payload, Lpkt.Len);				// 센서는 있는 겟수만큼 보내므로
			Poll_Flag = 1;
			Lora_Send_Status (Rx_Len, NODE_POLL_RES, Brg_Ea-Brg_No, 0);
			break;

		case CODY_RESET :	//  Reset
			if (Find_My_Id (Lpkt.Payload, Lpkt.Len) == 0) break;
			Restart ();
			break;

		case CODY_FIND :	//	RC Find
			if (Poll_Flag)	break;
			Lora_Send_Response (0xff, Rx_Len, Brg_Ea-Brg_No, NODE_FIND_RES);		// 폴을 받은적이 없으면
			break;

		case CODY_SET_LORA_NODE :		// 특정 노드나 전채 노드드중 브리지를 제외한 노드만 변경
			if (Lpkt.Len != 7) break;
			if (memcmp (Lpkt.Payload, Broadcast_Addr, 3) == 0)
				{
				if (My_Bridge_No > 0) break;  						// 브로드케스트 이면서 내가 브리지이면 무시
				}
			else
				{
				if (memcmp (Lpkt.Payload, CFG.Id, 3) != 0) break;	// 내가 아니면 끝
				}
			Lora_Change_Value (&Lpkt.Payload[3], 3);  // 3 초 후 리셋하여 설정되도록
			break;

		case CODY_SET_LORA_BRG :				// 브리지 노드만 변경
			if (Lpkt.Len != 4) break;
			if (My_Bridge_No == 0) break; 		// 내가 브리지가 아니면 무시
			Lora_Change_Value (&Lpkt.Payload[0], 10);  // 10 초 후 리셋하여 설정되도록
			break;

		case CODY_SET_NODE_ID :
			if (memcmp (Lpkt.Payload, CFG.Id, 3) != 0) break;	// 내가 아니면 리턴,  브로드케스트는 안됨
			if (Lpkt.Len != 6) break;
			if (memcmp (&Lpkt.Payload[3], Broadcast_Addr, 3) == 0) break;
			memcpy (CFG.Id, &Lpkt.Payload[3], 3);
			Write_CFG ();
			Restart_Timer = Time_1sec_Count + 3;	// 3 초 후 리셋
			break;

		case CODY_ACTION :	// 	Action		// CID(2)+Bridge+Cmd+len+Payload
			if (Lpkt.Len % 5) break;
			len = Lpkt.Len / 5;
			Action_Flag  = _ON_;
			Action_Timer = TimerGetCurrentTick();		// 받은 시간을 기억함,  이벶트 보낼대 3초 이상 없을때 전송을 위함
			{
			uint8_t *p = Lpkt.Payload;
			for (no=0; no<len; no++)
				{
				if (memcmp(p, Broadcast_Addr, 3) == 0) break;
				if (memcmp(p, CFG.Id, 3) == 0) break;
				p += 5;
				}
			if (no >= len) break;
#if 0
			if (!memcmp(p+3, Broadcast_Addr, 2))		// 자체 다으그노스틱 수행
				{
				Test_Flag  = 0;
				Test_Timer = 0;
				break;
				}
#endif
			Delete_Response_Packet ();					// 테이블에 있는 폴 응답 패킷 제거
			memcpy (Cfg_Act, p+3, 2);					// ACTION 으로 오는 것은 Flash 저장안함
			Action (Cfg_Act);
			}
			break;

		case CODY_SET_BRIDGE :	// 	Set Bridge (패킷하나에 7개 브리지 정보 전달)
			Find_Bridge_No (Lpkt.Payload, Lpkt.Len);
			break;

		case CODY_FW_UPDATE :	//	Update
			{
			uint8_t flag=0;
			if (memcmp (Lpkt.Payload, CFG.Id, 3) != 0) break;
			if (Lpkt.Len != FW_PKT_SZ+5) return;				// 데이타 48 + ID(3) + Seq(2)

			if (Lpkt.Payload[4] & 0x80)				// 상위 비트가 1이면 패킷의 끝
				{
				Lpkt.Payload[4] &= 0x7f;			// 알림비트 제거
				flag = 1;
				}

			no = Lpkt.Payload[3] + (Lpkt.Payload[4]*256);

			if (flag)
				{
				if (no < 630 || no > 960) return;		// 31K ~ 45K 아니면 에러
				}
			else
				{
				if (no > 960) return;					// 45K 아니면 에러
				}

			ret = Save_SFlash_Data (no, &Lpkt.Payload[5]);					// erase 600 msec 걸림
			switch (ret)
				{
				case 'A' :
					_DBG_ ADN_Printf ("Ack %d\n", FW_Save_Bno);
					Push_FW_Ack_Resopnse (Brg_Ea-Brg_No, FW_Save_Bno); 			// ACK	다음 받은 패킷순번을 알려준다.
					if (flag == 0) break;
					if (Restart_Timer)
						Restart_Timer = Time_1sec_Count + 30;		// 30초 후에 리부트 (코디에서 Ack 받을시간을 준다)
					else
						{
						Restart_Timer = Time_1sec_Count + 30;		// 30초 후에 리부트 (코디에서 Ack 받을시간을 준다)
						no = (FW_Save_Bno+2) * FW_PKT_SZ;
						no /= 100;
						sprintf (Work, "%c%c%c%c", 0x02, no%256, no/256, 0x03);
						sFLASH_WriteBuffer (Work, SB_SFLASH_FW, 6);
						ADN_Printf ("FW 100 bytes Block = %d", no);
						}
					break;
				case 99 : break;

				default:
					_DBG_ ADN_Printf ("Nak %d\n", FW_Save_Bno+ret);
					Push_FW_Ack_Resopnse (Brg_Ea-Brg_No, FW_Save_Bno+ret); // NAK  블럭의 못받은 패킷부터 보내도록
					break;
				}
			}
			break;

		case CODY_ALIVE :	//	Test Comm  코디에서 보내는 시험데이타
			ret = Find_My_Id (Lpkt.Payload, Lpkt.Len);
			if (ret == 0) break;
			Lora_Send_Response (ret, Rx_Len, Brg_Ea-Brg_No, NODE_ALIVE_RES);
			_DBG_ ADN_Printf ("Rx=-%d dBm", Rx_Rssi_Val);
			break;

		case NODE_EVENT :	//	event
			break;	//  릴레이는 다른 노드의 이벤트 감지시 동작안함


		case CMD_CN_RSSI_RES :
			if (memcmp(Lpkt.Payload, CFG.Id, 3) != 0) break;
			ADN_Printf ("RSSI  Tx: -%d, Rx: -%d", Lpkt.Payload[3], Rx_Rssi_Val);
			if (Test_Rssi_Count > 0)
				{
				Test_Rssi_Count--;
				Lora_Send_Response (0, 0, 0, NODE_RSSI);
				}
			break;
		}
}

//------------------------------------------------------------------------------
int Get_LoRa ()
{
uint8_t	len;

	switch(Radio->Process())
		{
		case RF_RX_TIMEOUT:
	    	return 0;

		case RF_RX_DONE:
			len = Radio->GetRxPacket (Lora_Rx_Buff, 96);
			if (Lora_Rx_Buff[0] != 0x20 || Lora_Rx_Buff[1] != 0x57) return 0;
			if (len > 80) return 0;
			break;

		case RF_TX_DONE:
		    SYS_RTx_State  = _Rx_;
		    GPIO_ResetBits(GPIOD, GPIO_Pin_6);
		    Radio->StartRx ();
		    Tx_Wait_50ms_Time = TimerGetCurrentTick();	// 노이즈 화인시간 초기화
		    return 0;

		default :
			return 0;
		}

	return (len);
}
//----------------------------------------------------------------------------------
void Set_LoRa (uint8_t flag, uint8_t Channel, uint8_t Band, uint8_t Sfactor, uint8_t Pwr)
{
uint32_t Ch;

	Ch = (uint32_t)LoRa_Channel[Channel] * 100000;
	if (flag == 0)
		{
		Radio = RadioDriverInit( );
		Radio->Init(Ch, Band, Sfactor, Pwr);
		}
	else
		{
		SX1276LoRaSetOpMode( 0x01 );					// re init mode
		SX1276LoRaSetRFFrequency (Ch);
		SX1276LoRaSetSignalBandwidth (Band);
		SX1276LoRaSetSpreadingFactor (Sfactor);
		SX1276LoRaSetRFPower (Pwr);
		}

	Set_125khz (Band);

	Radio->StartRx( );
	GPIO_ResetBits(GPIOD, GPIO_Pin_6);
	_DBG_ ADN_Printf ("%d Set Lora %d:%d:%d:%d",Time_1sec_Count, Ch, Band, Sfactor, Pwr);
}

//----------------------------------------------------------
void Lora_Change_Value (uint8_t *p, uint8_t sec)
{
uint8_t c, b, s;

	c = *p++;
	b = *p++;
	s = *p++;
	if (c > 19 && c != 0xff) return;
	if ((b < 7 || b > 8)  && b != 0xff) return;
	if ((s < 7 || s > 12) && s != 0xff) return;
	if (c != 0xff) CFG.Channel = c;					// 0xff 는 이전값 그대로
	if (b != 0xff) CFG.Band    = b;
	if (s != 0xff) CFG.Sfactor = s;

	Write_CFG ();
	Restart_Timer = Time_1sec_Count + sec;	// sec 후 리셋하여 설정되도록
}
//------------------------------------------------------------------------------
void Send_Lora (uint8_t *p, uint16_t len)
{
	GPIO_SetBits(GPIOD, GPIO_Pin_6);
	SYS_RTx_State  = _Tx_;									// 재 전송 중지
	Radio->SetTxPacket(p, len);
	_DBG_ ADN_Hexprint ("LTx", p, len);
}

