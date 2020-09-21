/* License: COPYING.GPLv3 */
/* Copyright 2019 - present Lenovo */
#pragma once

// ITE TCON Error Code
const DWORD ITETCON_EC_SUCCESS = 0;
const DWORD ITETCON_EC_FAILED = -1;

#define SCSI_OPCODE_INQUIRY		0x12
#define SCSI_OPCODE_ITECUSTOM	0xFE

//IT8951 USB Op Code - Fill this code to CDB[6]
#define IT8951_USB_OP_GET_SYS      0x80
#define IT8951_USB_OP_READ_MEM     0x81
#define IT8951_USB_OP_WRITE_MEM    0x82
#define IT8951_USB_OP_READ_REG     0x83
#define IT8951_USB_OP_WRITE_REG    0x84

#define IT8951_USB_OP_DPY_AREA     0x94

#define IT8951_USB_OP_USB_SPI_ERASE 0x96 //Eric Added
#define IT8951_USB_OP_USB_SPI_READ  0x97
#define IT8951_USB_OP_USB_SPI_WRITE 0x98

#define IT8951_USB_OP_LD_IMG_AREA  0xA2
#define IT8951_USB_OP_PMIC_CTRL    0xA3
#define IT8951_USB_OP_IMG_CPY      0xA4 //Not used in Current Version(IT8961 Samp only)
#define IT8951_USB_OP_FSET_TEMP    0xA4
#define IT8951_USB_OP_FAST_WRITE_MEM 0xA5
#define IT8951_USB_OP_SCENARIO     0xA6
#define IT8951_USB_OP_LD_IMG_AREA2 0xA8
#define IT8951_USB_OP_SET_WAVEFORM 0xA9
#define IT8951_USB_OP_SET_HANDWR_REGION  0xAC
#define IT8951_USB_OP_SET_HANDWR_WIDTH   0xAE
#define IT8951_USB_OP_SET_TP_AREA    0xAF

#define IT8951_USB_OP_GET_DPY_STATUS 0xB1
#define IT8951_USB_OP_SET_LD_IMG_AREA2  0xB2
// zhuhl5
#define IT8951_USB_OP_DYNAMICSETTING 0xB3


#pragma pack(1)

//*****************************************************
//  Structure of Get Device Information
//*****************************************************
//typedef struct _TRSP_SYSTEM_INFO_DATA
//{
//	unsigned int uiStandardCmdNo; // Standard command number2T-con Communication Protocol
//	unsigned int uiExtendCmdNo; // Extend command number
//	unsigned int uiSignature; // 31 35 39 38h (8951)
//	unsigned int uiVersion; // command table version
//	unsigned int uiWidth; // Panel Width
//	unsigned int uiHeight; // Panel Height
//	unsigned int uiUpdateBufBase; // Update Buffer Address
//	unsigned int uiImageBufBase; // Image Buffer Address
//	unsigned int uiTemperatureNo; // Temperature segment number
//	unsigned int uiModeNo; // Display mode number
//	unsigned int uiFrameCount[8]; // Frame count for each mode(8).
//	unsigned int uiNumImgBuf;
//	unsigned int uiWbfSFIAddr; 
//	unsigned int uiReserved[8]; 
//	
////	void* lpCmdInfoDatas[1]; // Command table pointer
//} TRSP_SYSTEM_INFO_DATA;

//--------------------------------------------------------
//    Display Area
//--------------------------------------------------------

typedef struct  _TDRAW_UPD_ARG_DATA
{
	int     iMemAddr;
	int     iWavMode;
	//int     iAlpha;
	int     iPosX;
	int     iPosY;
	int     iWidth;
	int     iHeight;
	int     iEngineIndex;
} TDRAW_UPD_ARG_DATA;
typedef TDRAW_UPD_ARG_DATA  TDrawUPDArgData;
typedef TDrawUPDArgData*    LPDrawUPDArgData;

//--------------------------------------------------------
//    Load Image Area
//--------------------------------------------------------
typedef struct _LOAD_IMG_AREA_
{
	int     iAddress;
	int     iX;
	int     iY;
	int     iW;
	int     iH;
} LOAD_IMG_AREA;
//--------------------------------------------------------
//    PMIC Control
//--------------------------------------------------------
typedef struct _PMIC_CTRL_
{
	unsigned short usSetVComVal;
	unsigned char  ucDoSetVCom;
	unsigned char  ucDoPowerSeqSW;
	unsigned char  ucPowerOnOff;
	unsigned char  ucReserved[3];


}T_PMIC_CTRL;
//--------------------------------------------------------
//    Forece Set/Get Temperature Control
//--------------------------------------------------------
typedef struct
{
	unsigned char ucSetTemp;
	unsigned char ucTempVal;

}T_F_SET_TEMP;
//--------------------------------------------------------
//    Image DMA Copy 
//--------------------------------------------------------
typedef struct _DMA_IMG_COPY_
{
	int     iSrcAddr;
	int     iDestAddr;
	int     iX;    //Src X
	int     iY;    //Src Y
	int     iW;
	int     iH;
	int     iX2;   //Target X For different (x,y)
	int     iY2;   //Target Y
} TDMA_IMG_COPY;

//---------------------------------------------------------------------------
// SPI Struct
//---------------------------------------------------------------------------
typedef struct  _TSPI_CMD_ARG_DATA
{
	int     iSPIAddress;
	int     iDRAMAddress;
	int     iLength;            /*Byte count*/
} TSPI_CMD_ARG_DATA;
typedef TSPI_CMD_ARG_DATA  TSPICmdArgData;
typedef TSPICmdArgData*     LPSPICmdArgData;
//---------------------------------------------------------------------------
typedef struct  _TSPI_CMD_ARG_ERASE_DATA
{
	int     iSPIAddress;
	int     iLength;            /*Byte count*/
} TSPI_CMD_ARG_ERASE_DATA;
typedef TSPI_CMD_ARG_ERASE_DATA  TSPICmdArgEraseData;
typedef TSPICmdArgEraseData*     LPSPICmdArgEraseData;

//----------------------------------------------------
//   For USB Command 0xA5
//----------------------------------------------------
#define TABLE_TYPE_LUT                      0  //See Tableinfo.h
#define TABLE_TYPE_VCOM                     1
#define TABLE_TYPE_TCON                     2

typedef struct
{
	BYTE ucType;        //Table type
	BYTE ucMode;        //Display Mode
	BYTE ucTempSeg;     //Temperature segments
	BYTE ucSizeL;       //Table Size L
	BYTE ucSizeH;       //Table Size H
	BYTE ucFrameCntL;   //Frame counts L
	BYTE ucFrameCntH;   //Frame Counts H
	BYTE ucReserved;    //Reserved for padding to 8-bytes

}TSetLUTInfo;

typedef struct
{
	WORD usX;
	WORD usY;
	WORD usW;
	WORD usH;

}TWrRegionInfo;

#define SPTD_BUF_SIZE  (60*1024)

#define SWAP_32(l) (((l) >> 24) | (((l) & 0x00ff0000) >> 8) | (((l) & 0x0000ff00) << 8) | ((l) << 24))
#define SWAP_16(s) ((((s) & 0xff) << 8) | (((s) >> 8) & 0xff))

enum _USB_UPDATE_CMD {
	// for KB_BLOB1
	kb_blob_cmd_valid = 0x01,
	kb_blob_cmd_get_status,
	kb_blob_cmd_get_info,

	// for FW_P1
	fw_cmd_valid,
	fw_cmd_get_status,
};

#pragma pack()