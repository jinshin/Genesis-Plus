typedef struct {
char str[80];
int  ttl;
} mymessage;

void AddMessage (char * string, int ttl);
void ShiftMessages (int From);
void ProcessMessages ();
void draw_text (uint8 * buffer, int x, int y, char * string, int bwidth);
void PutBuffer();
void UpdateFromCache();
void UpdateCache();

#ifndef _WFIX
void DrawLine_RL (uint16 *src, int line, int offset, uint16 *dst, int length);
void DrawLine_RR (uint16 *src, int line, int offset, uint16 *dst, int length);
void DrawLine_P  (uint16 *src, int line, int offset, uint16 *dst, int length);
#endif
void DrawLine_X  (uint16 *src, int line, int offset, uint16 *dst, int length);


