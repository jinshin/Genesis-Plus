
#define DM_DISPLAYORIENTATION 0x00800000;
#define DM_DISPLAYQUERYORIENTATION 0x01000000;
uint CDS_RESET = 0x40000000;
uint CDS_TEST = 0x00000002;
uint DMDO_0 = 0;
uint DMDO_90 = 1;
uint DMDO_180 = 2;
uint DMDO_270 = 4;


typedef struct _devicemodeW4 { 
  WCHAR  dmDeviceName[CCHDEVICENAME]; 
  WORD   dmSpecVersion; 
  WORD   dmDriverVersion; 
  WORD   dmSize; 
  WORD   dmDriverExtra; 
  DWORD  dmFields; 
  short  dmOrientation;
  short  dmPaperSize;
  short  dmPaperLength;
  short  dmPaperWidth;
  short  dmScale; 
  short  dmCopies; 
  short  dmDefaultSource; 
  short  dmPrintQuality; 
  short  dmColor; 
  short  dmDuplex; 
  short  dmYResolution; 
  short  dmTTOption; 
  short  dmCollate; 
  WCHAR  dmFormName[CCHFORMNAME]; 
  WORD   dmLogPixels; 
  DWORD  dmBitsPerPel; 
  DWORD  dmPelsWidth; 
  DWORD  dmPelsHeight; 
  DWORD  dmDisplayFlags; 
  DWORD  dmDisplayFrequency; 
  DWORD  dmDisplayOrientation;
} DEVMODEW4;

#define DEVMODE4 DEVMODEW4
