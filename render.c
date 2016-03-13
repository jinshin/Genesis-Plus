/*                                            `
        only update window clip on window change (?)
        fix leftmost window/nta render and window bug
        sprite masking isn't right in sonic/micromachines 2, but
        seems ok in galaxy force 2
*/

#include "shared.h"

//#define ALIGN_LONG

#ifdef ALIGN_LONG

/* Or change the names if you depend on these from elsewhere.. */

static __inline__ void WRITE_LONG(void *address, uint32 data)
{
            *((uint8 *)address++) = data; data=data>>8;
            *((uint8 *)address++) = data; data=data>>8;
            *((uint8 *)address++) = data; data=data>>8;
            *((uint8 *)address)   = data; return;
}

#endif  /* ALIGN_LONG */

#ifdef ALIGN_LONG

/* Draw a single 8-pixel column */
#define DRAW_COLUMN(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    WRITE_LONG(dst++, *src++ | atex); \
    WRITE_LONG(dst++, *src++ | atex); \
    ATTR >>= 16;  \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    WRITE_LONG(dst++, *src++ | atex); \
    WRITE_LONG(dst++, *src++ | atex); \

/* Draw a single 8-pixel column */
#define DRAW_COLUMN_A(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex); \
    ATTR >>= 16; \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex);

/* Draw a single 16-pixel column */
#define DRAW_COLUMN_IM2(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    WRITE_LONG(dst++, *src++ | atex); \
    WRITE_LONG(dst++, *src++ | atex); \
    ATTR >>= 16; \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    WRITE_LONG(dst++, *src++ | atex); \
    WRITE_LONG(dst++, *src++ | atex); \

/* Draw a single 16-pixel column */
#define DRAW_COLUMN_IM2_A(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex); \
    ATTR >>= 16; \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex);

#else

#define DRAW_COLUMN_A(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex); \
    ATTR >>= 16; \
    atex = atex_table[(ATTR >> 13) & 7]; \
    src = (uint32 *)&bg.bg_pattern_cache[(ATTR & 0x1FFF) << 6 | (LINE)]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex);

#define DRAW_COLUMN_IM2_A(ATTR, LINE) \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex); \
    ATTR >>= 16; \
    atex = atex_table[(ATTR >> 13) & 7]; \
    offs = (ATTR & 0x03FF) << 7 | (ATTR & 0x1800) << 6 | (LINE); \
    if(ATTR & 0x1000) offs ^= 0x40; \
    src = (uint32 *)&bg.bg_pattern_cache[offs]; \
    *dst++ = (*src++ | atex); \
    *dst++ = (*src++ | atex);

#endif /* ALIGN_LONG */

#define DRAW_SPRITE_TILE_SIMPLE \
	*lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src++ | palette)]; \
        *lb++ = table[(*lb << 8) |(*src   | palette)]; 


#define DST_ENTER register int colli = vdp.collision

#define DST_EXIT vdp.collision = colli

#define DRAW_SPRITE_TILE  \
	register int i=8; \
	do { \
	uint8 val = *src++; \
	  if (val!=0) { \
                *lb = table[(*lb << 8) |(val | palette)]; \
	  	if (!colli) { colli = *lbc; *lbc=1; }; \
		}; \
        lb++; lbc++; \
	} while (--i); 

/* Pixel creation macros, input is four bits each */

/* 8:8:8 RGB */
#define MAKE_PIXEL_32(r,g,b) ((r) << 20 | (g) << 12 | (b) << 4)
                     
/* 5:5:5 RGB */
#define MAKE_PIXEL_15(r,g,b) ((r) << 11 | (g) << 6 | (b) << 1)

/* 5:6:5 RGB */
#define MAKE_PIXEL_16(r,g,b) ((r) << 12 | (g) << 7 | (b) << 1)

/* 3:3:2 RGB */
#define MAKE_PIXEL_8(r,g,b)  ((r) <<  5 | (g) << 2 | ((b) >> 1))

/* Clip data */
static clip_t clip[2];

/* Attribute expansion table */
static const uint32 atex_table[] = {
    0x00000000, 0x10101010, 0x20202020, 0x30303030,
    0x40404040, 0x50505050, 0x60606060, 0x70707070
};

/* Sprite name look-up table */
uint8 name_lut[0x400];

/* Sprite line buffer data */
uint8 object_index_count;

struct {
    uint16 ypos;
    uint16 xpos;
    uint16 attr;
    uint8 size;
    uint8 index;
}object_info[20];

/* Pixel look-up tables and table base address */
uint8 *lut[5];
uint8 *lut_base;

uint32 _align_1;
uint16 pixel_16[0x100];
uint16 pixel_16_lut[3][0x200];

uint32 pixel_32[0x100];
uint32 pixel_32_lut[3][0x200];

/* Line buffers */
uint32 _align_11;
static uint8 lb[0x190];      /* Line Buffer */             /* Temporary buffer */
static uint8 bg_buf[0x190];                    /* Merged background buffer */
static uint8 nta_buf[0x190];                   /* Plane A / Window line buffer */
static uint8 ntb_buf[0x190];                   /* Plane B line buffer */
static uint8 obj_buf[0x190];                   /* Object layer line buffer */
static uint8 sc_buf[0x190];                    /* Sprite collision buffer */

extern uint8 rotateright;
extern uint8 landscape;
extern uint8 square_screen;
extern uint8 crop_screen;

void (*RenderLine) (uint8 *src, int line, int offset, uint32 *table, int length);

void RenderLine_X1 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X1_Buffer (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X1S1_Buffer (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X1S2_Buffer (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X1S3_Buffer (uint8 *src, int line, int offset, uint32 *table, int length);

void RenderLine_X1SGD_Buffer (uint8 *src, int line, int offset, uint32 *table, int length);

void RenderLine_X2 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X2S1 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X2S2 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X2S3 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X2S4 (uint8 *src, int line, int offset, uint32 *table, int length);

void RenderLine_X3 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X3S1 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X3S2 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X3S3 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X3S4 (uint8 *src, int line, int offset, uint32 *table, int length);

void RenderLine_X4 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X4S1 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X4S2 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X4S3 (uint8 *src, int line, int offset, uint32 *table, int length);
void RenderLine_X4S4 (uint8 *src, int line, int offset, uint32 *table, int length);


/*--------------------------------------------------------------------------*/
/* Init, reset, shutdown routines                                           */
/*--------------------------------------------------------------------------*/

extern uint8   renderer;
extern uint8   xbrz;
extern int   scale;
uint8	phos = 0;
uint8   lcd = 0;

uint32 Buffer[642*480];

uint32 Cache[642*480];


void set_renderer()
{
memset(&Cache[0],0,642*480*4);
memset(&Buffer[0],0,642*480*4);

if (scale <= 2) {
   switch (renderer) {
	case 0: xbrz=0; RenderLine = RenderLine_X2; break;
	case 1: xbrz=0; RenderLine = RenderLine_X2S1; break;
	case 2: xbrz=0; RenderLine = RenderLine_X2S2; break;
	case 3: xbrz=0; RenderLine = RenderLine_X2S3; break;
	case 4: xbrz=0; RenderLine = RenderLine_X2S4; break;

	case 5: xbrz=1; RenderLine = RenderLine_X1S4_Buffer; break;
	case 6: xbrz=1; RenderLine = RenderLine_X1S1_Buffer; break;
	case 7: xbrz=1; RenderLine = RenderLine_X1S2_Buffer; break;
	case 8: xbrz=1; RenderLine = RenderLine_X1S3_Buffer; break;

        }
}

if (scale == 3) {
   switch (renderer) {
	case 0: xbrz=0; RenderLine = RenderLine_X3; break;
	case 1: xbrz=0; RenderLine = RenderLine_X3S1; break;
	case 2: xbrz=0; RenderLine = RenderLine_X3S2; break;
	case 3: xbrz=0; RenderLine = RenderLine_X3S3; break;
	case 4: xbrz=0; RenderLine = RenderLine_X3S4; break;

	case 5: xbrz=1; RenderLine = RenderLine_X1S4_Buffer; break;
	case 6: xbrz=1; RenderLine = RenderLine_X1S1_Buffer; break;
	case 7: xbrz=1; RenderLine = RenderLine_X1S2_Buffer; break;
	case 8: xbrz=1; RenderLine = RenderLine_X1S4_Buffer; break;
	}
}

if (scale == 4) {
   switch (renderer) {
	case 0: xbrz=0; RenderLine = RenderLine_X4; break;
	case 1: xbrz=0; RenderLine = RenderLine_X4S1; break;
	case 2: xbrz=0; RenderLine = RenderLine_X4S2; break;
	case 3: xbrz=0; RenderLine = RenderLine_X4S3; break;
	case 4: xbrz=0; RenderLine = RenderLine_X4S4; break;

	case 5: xbrz=1; RenderLine = RenderLine_X1S4_Buffer; break;
	case 6: xbrz=1; RenderLine = RenderLine_X1S1_Buffer; break;
	case 7: xbrz=1; RenderLine = RenderLine_X1S2_Buffer; break;
	case 8: xbrz=1; RenderLine = RenderLine_X1S4_Buffer; break;
	}
}
}

int render_init(void)
{
    int bx, ax, i;

    /* Allocate and align pixel look-up tables */
    lut_base = malloc((LUT_MAX * LUT_SIZE) + LUT_SIZE);
    lut[0] = (uint8 *)(((uint32)lut_base + LUT_SIZE) & ~(LUT_SIZE - 1));
    for(i = 1; i < LUT_MAX; i += 1)
    {
        lut[i] = lut[0] + (i * LUT_SIZE);
    }

    /* Make pixel look-up table data */
    for(bx = 0; bx < 0x100; bx += 1)
    for(ax = 0; ax < 0x100; ax += 1)
    {
        uint16 index = (bx << 8) | (ax);
        lut[0][index] = make_lut_bg(bx, ax);
        lut[1][index] = make_lut_obj(bx, ax);
        lut[2][index] = make_lut_bg_ste(bx, ax);
        lut[3][index] = make_lut_obj_ste(bx, ax);
        lut[4][index] = make_lut_bgobj_ste(bx, ax);
    }

    /* Make pixel data tables */

//16 Bit
    for(i = 0; i < 0x200; i += 1)
    {
        int r, g, b;
        
        r = (i >> 6) & 7;
        g = (i >> 3) & 7;
        b = (i >> 0) & 7;

        pixel_16_lut[0][i] = MAKE_PIXEL_16(r,g,b);
        pixel_16_lut[1][i] = MAKE_PIXEL_16(r<<1,g<<1,b<<1);
        pixel_16_lut[2][i] = MAKE_PIXEL_16(r|8,g|8,b|8);
    }


//32 Bit
    for(i = 0; i < 0x200; i += 1)
    {
        int r, g, b;
        
        r = (i >> 6) & 7;
        g = (i >> 3) & 7;
        b = (i >> 0) & 7;

        pixel_32_lut[0][i] = MAKE_PIXEL_32(r,g,b);
        pixel_32_lut[1][i] = MAKE_PIXEL_32(r<<1,g<<1,b<<1);
        pixel_32_lut[2][i] = MAKE_PIXEL_32(r|8,g|8,b|8);
    }


    /* Set up color update function */

#define color_update color_update_32;

    /* Make sprite name look-up table */
    make_name_lut();

    return (1);
}

void make_name_lut(void)
{
    int col, row;
    int vcol, vrow;
    int width, height;
    int flipx, flipy;
    int i, name;

    memset(name_lut, 0, sizeof(name_lut));

    for(i = 0; i < 0x400; i += 1)
    {
        vcol = col = i & 3;
        vrow = row = (i >> 2) & 3;
        height = (i >> 4) & 3;
        width = (i >> 6) & 3;
        flipx = (i >> 8) & 1;
        flipy = (i >> 9) & 1;

        if(flipx)
            vcol = (width - col);
        if(flipy)
            vrow = (height - row);

        name = vrow + (vcol * (height + 1));

        if((row > height) || col > width)
            name = -1;

        name_lut[i] = name;        
    }
}



void render_reset(void)
{
    memset(&clip, 0, sizeof(clip));

    memset(bg_buf, 0, sizeof(bg_buf));
    memset(lb, 0, sizeof(lb));
    memset(nta_buf, 0, sizeof(nta_buf));
    memset(ntb_buf, 0, sizeof(ntb_buf));
    memset(obj_buf, 0, sizeof(obj_buf));
    memset(&pixel_16, 0, sizeof(pixel_16));
    memset(&pixel_32, 0, sizeof(pixel_32));
}


void render_shutdown(void)
{
    if(lut_base) free(lut_base);
}

/*--------------------------------------------------------------------------*/
/* Line render function                                                     */
/*--------------------------------------------------------------------------*/

void render_line(int line)
{

    int mergewidth = (vdp.reg[12] & 1) ? 320 : 256;

    update_bg_pattern_cache();


    if((vdp.reg[1] & 0x40) == 0x00)
    {
        // Use the overscan color to clear the screen
        memset(&lb[0x20], 0x40 | vdp.border , bitmap.viewport.w);
    }
    else
    {
        window_clip(line);

        if(vdp.im2_flag)
        {
            render_ntx_im2(0, line, nta_buf);
            render_ntx_im2(1, line, ntb_buf);
        }
        else
        {
            if(vdp.reg[0x0B] & 4)
            {
                render_ntx_vs(0, line, nta_buf);
                render_ntx_vs(1, line, ntb_buf);
            }
            else
            {
                render_ntx(0, line, nta_buf);
                render_ntx(1, line, ntb_buf);
            }
        }

        if(vdp.im2_flag)
              render_ntw_im2(line, nta_buf);
        else
              render_ntw(line, nta_buf);

        if(vdp.reg[12] & 8)
        {
            merge(&nta_buf[0x20], &ntb_buf[0x20], &bg_buf[0x20], lut[2], mergewidth);
            memset(&obj_buf[0x20], 0, mergewidth);

            if(vdp.im2_flag)
                render_obj_im2(line, obj_buf, lut[3]);
            else
                render_obj(line, obj_buf, lut[3]);

          merge(&obj_buf[0x20], &bg_buf[0x20], &lb[0x20], lut[4], mergewidth);
      }
      else
      {
            merge(&nta_buf[0x20], &ntb_buf[0x20], &lb[0x20], lut[0], mergewidth);

            if(vdp.im2_flag)
                render_obj_im2(line, lb, lut[1]);
            else
                render_obj(line, lb, lut[1]);
      }
    }

    if(vdp.reg[0] & 0x20)
    {
        memset(&lb[0x20], 0x40 | vdp.border, 0x08);
    }

//Render line
    RenderLine (lb+0x20, line, bitmap.viewport.x, pixel_32, mergewidth);

}


void RenderLine_X1 (uint8 *src, int line, int offset, uint32 *table, int length)
{
uint32* tmp=(uint8*)bitmap.data + ((offset+line*320)<<2);
uint32* outWrite = (uint32*)tmp;
	do {
		*outWrite++ = table[*src++];
	} while (--length);
}

#define PRE_X1_32_BUF  \
	src[length]=src[length-1]; \
	uint32 *outWrite = &Buffer[0] + offset+line*320; \
	uint32 *inCache = &Cache[0] + offset+line*320;	

void RenderLine_X1_Buffer (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X1_32_BUF;
	do {
		*outWrite++ = table[*src++];
	} while (--length);
}

void RenderLine_X1S1_Buffer (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X1_32_BUF;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		p1 = (p1+p2)>>1;
		*outWrite++ = p1;
	} while (--length);
}


void RenderLine_X1S2_Buffer (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X1_32_BUF;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = *inCache;
		*inCache++ = p1;
		p1 = (p1+p2)>>1;
		*outWrite++ = p1;
	} while (--length);
}

void RenderLine_X1S3_Buffer (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X1_32_BUF;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		uint32 p3 = *inCache;		
		p1 = (p1+p2)>>1;
		*inCache++ = p1;
		p1 = (p1+p3)>>1;
		*outWrite++ = p1;
	} while (--length);
}

//GDAPT
/*
   Genesis Dithering and Pseudo Transparency Shader v1.3
   by Sp00kyFox, 2014

   Fast and simpified version by n0p, 2016	

*/

uint32 Line[320];
uint32 PreLine[322];
int  tag[321];

inline
float invsqrt( float number )
{
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                       // evil floating point bit level hacking
	i  = 0x5f3759df - ( i >> 1 );               // what the fuck? 
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) );   // 1st iteration

	return y;
}

inline
float ColorDistEq(const unsigned int pix1, const unsigned int pix2)
{
    if (pix1 == pix2) return 1;	
    unsigned int tmp1 = pix1;
    unsigned int tmp2 = pix2;
    int b_diff = (tmp1&0xFF)-(tmp2&0xFF);
    tmp1>>=8; tmp2>>=8; 
    int g_diff = (tmp1&0xFF)-(tmp2&0xFF);	
    tmp1>>=8; tmp2>>=8;
    int r_diff = tmp1 - tmp2;
    long rmean = ( (long)tmp1 + (long)tmp2 ) >> 1;
    float cdist = sqrt((((512+rmean)*r_diff*r_diff)>>8) + ((g_diff*g_diff)<<2) + (((767-rmean)*b_diff*b_diff)>>8));
    if (cdist==0) return  1;	
    if (cdist>=3) return  0;
    return 1-cdist/3;
}

inline
float DotNormals(const unsigned int C, const unsigned int L, const unsigned int R)
{
    unsigned int tmpC = C;
    unsigned int tmpL = L;
    unsigned int tmpR = R;

    float CLb = (tmpC&0xFF)-(tmpL&0xFF);
    float CRb = (tmpC&0xFF)-(tmpR&0xFF);
        tmpC>>=8; tmpL>>=8; tmpR>>=8;
    float CLg = (tmpC&0xFF)-(tmpL&0xFF);
    float CRg = (tmpC&0xFF)-(tmpR&0xFF);
        tmpC>>=8; tmpL>>=8; tmpR>>=8;
    float CLr = (tmpC&0xFF)-(tmpL&0xFF);
    float CRr = (tmpC&0xFF)-(tmpR&0xFF);
      
    float CLl = invsqrt(CLr*CLr + CLg*CLg + CLb*CLb);
    float CRl = invsqrt(CRr*CRr + CRg*CRg + CRb*CRb);
 
      //if ( d == 1.0  || d == 0.0 ) return;

      CLr = CLr * CLl;
      CLg = CLg * CLl;
      CLb = CLb * CLl;
	
      CRr = CRr * CRl;
      CRg = CRg * CRl;
      CRb = CRb * CRl;
   
      return CLr*CRr+CLg*CRg+CLb*CRb; 

}

inline
int Lerp (int A, int B, int W)
{
	//if (W<0.1) return A;
        return (A + W*(B-A));
}

void RenderLine_X1SGD_Buffer (uint8 *src, int line, int offset, uint32 *table, int length)
{
//Line
long savelen = length;
uint32 *outWrite = &PreLine[1]; 
	do {
		*outWrite++ = table[*src++];
	} while (--length);
//Pass 1
        for (int x=1; x<=savelen; x++)
	{
	    uint32 C = PreLine[x];
	    uint32 L = PreLine[x-1];
            uint32 R = PreLine[x+1];		

	//	float DN = ColorDistEq(L,R);
        //    if ((DN>0)&&(DN<0.99)) fprintf(stderr," %f",DN);

            //tag[x] = ((DotNormals(C,L,R) * ColorDistEq(L,R))<0.6) ? 0 : 1;
            //tag[x] = ((ColorDistEq(L,R))<0.6) ? 0 : 1;
	
	    //tag[x] = (L!=R) ? 0 : 1;
	    tag [x] = 0;	

	    if ((R==L)&(C!=R)) {
                tag [x] = 256;
	    }   //else if (tag [x-1] == 256) tag [x] = 32;

	    //if ((tag[x]>0)&&(tag[x]<0.95)) fprintf(stderr," %f",tag[x]);
	}	
//Pass 2	
	outWrite = &Buffer[0] + offset+line*320; \
	uint32 *inCache = &Cache[0] + offset+line*320;	
        for (int x=1; x<=savelen; x++)
	{

	    int Cw = tag[x];
	    int Lw = tag[x-1];
	    int Rw = tag[x+1];

	    int str = Lw;
	    if (Lw<Rw) str = Rw;
	    if (Cw<str) str = Cw;

	    uint32 C = PreLine[x];

	    if (str == 0) {
	        *outWrite++ = C;
	    } else {

	    uint32 L = PreLine[x-1];
	    uint32 R = PreLine[x+1];

	    //if ((str>0)&&(str<0.95)) fprintf(stderr," %f",str);

	    unsigned char nB,nG,nR;	

   	    int sum  = Lw + Rw;
	    int wght = Lw;
	    if (Rw>Lw) wght = Rw;	
	    wght = (wght == 0) ? 256 : (sum*256)/wght;
		

	    //sum=512, wght=256, 

		nB = (long)(wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(long)(wght + sum);
		C>>=8; L>>=8; R>>=8;
		nG = (long)(wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(long)(wght + sum);
		C>>=8; L>>=8; R>>=8;
		nR = (long)(wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(long)(wght + sum);
		
		*outWrite++ = (nR<<16) + (nG<<8) + nB;

	    }
/*
            unsigned char nB = Lerp(C&0xFF, (wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(wght + sum), str);	
            C>>=8; L>>=8; R>>=8;
            unsigned char nG = Lerp(C&0xFF, (wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(wght + sum), str);
            C>>=8; L>>=8; R>>=8;
            unsigned char nR = Lerp(C&0xFF, (wght*(C&0xFF) + Lw*(L&0xFF) + Rw*(R&0xFF))/(wght + sum), str);		
		
	    *outWrite++ = (nR<<16) + (nG<<8) + nB;
*/	   
	}	
}

extern void xBRZScale_2X (uint32* input, uint8* output, int pitch);
extern void xBRZScale_3X (uint32* input, uint8* output, int pitch);
extern void xBRZScale_4X (uint32* input, uint8* output, int pitch);

extern void xBRZScale_2X_MT (uint32* input, uint8* output, int pitch);
extern void xBRZScale_3X_MT (uint32* input, uint8* output, int pitch);
extern void xBRZScale_4X_MT (uint32* input, uint8* output, int pitch);

extern void xBRZScale_2X_MT2 (uint32* input, uint8* output, int pitch);
extern void xBRZScale_3X_MT2 (uint32* input, uint8* output, int pitch);
extern void xBRZScale_4X_MT2 (uint32* input, uint8* output, int pitch);


void xBRZ_2X (uint32* input, uint32* output) {
	xBRZScale_2X(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_3X (uint32* input, uint32* output) {
	xBRZScale_3X(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_4X (uint32* input, uint32* output) {
	xBRZScale_4X(&Buffer[0],bitmap.data,bitmap.pitch);
}

void xBRZ_2X_MT (uint32* input, uint32* output) {
	xBRZScale_2X_MT(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_3X_MT (uint32* input, uint32* output) {
	xBRZScale_3X_MT(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_4X_MT (uint32* input, uint32* output) {
	xBRZScale_4X_MT(&Buffer[0],bitmap.data,bitmap.pitch);
}


void xBRZ_2X_MT2 (uint32* input, uint32* output) {
	xBRZScale_2X_MT2(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_3X_MT2 (uint32* input, uint32* output) {
	xBRZScale_3X_MT2(&Buffer[0],bitmap.data,bitmap.pitch);
} 

void xBRZ_4X_MT2 (uint32* input, uint32* output) {
	xBRZScale_4X_MT2(&Buffer[0],bitmap.data,bitmap.pitch);
}  
  

extern int scale;

#define PRE_X2_32  \
	src[length]=src[length-1]; \
	uint32 *outWrite = (uint32*)bitmap.data + ((offset+line*640)<<1); \
	uint32 *inCache = &Cache[0] + (offset+line*320);	


void RenderLine_X2 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
	} while (--length);
}

void RenderLine_X2S1 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];

		//LINE
		//p1 = ((p1&0xFEFEFE)>>1)+((p2&0xFEFEFE)>>1);
		p1 = (p1+p2)>>1;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
	} while (--length);
}

void RenderLine_X2S2 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p3 = *inCache;
	
		*inCache++=p1;	
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite = p1;

                *(outWrite+640) = p1;
                *outWrite++;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
	} while (--length);
}

void RenderLine_X2S3 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		uint32 p3 = *inCache;
		uint32 p4 = p1;
	
		//LINE
		p1 = (p1+p2)>>1;

		*inCache=p1;	
		*inCache++;
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite = p1;

                *(outWrite+640) = p1;
                *outWrite++;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
	} while (--length);
}

//DOTs
void RenderLine_X2S4 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 pa = (p1&0xFEFEFE)>>1;
		uint32 p2 = (p1&0x00FF00)|(pa&0xFF00FF);
		uint32 p3 = (p1&0xFF0000)|(pa&0x00FFFF);
		uint32 p4 = (p1&0x0000FF)|(pa&0xFFFF00);
		*outWrite = p1;
                *(outWrite+640) = p2;
                *outWrite++;
		*outWrite = p3;
                *(outWrite+640) = p4;
                *outWrite++;
	} while (--length);
}


//------------------------------------- X3 -------------------------------------------------------

#define PRE_X3_32  \
	src[length]=src[length-1]; \
	int soffset = (bitmap.pitch-(320*3))/2; \
	uint8 *tmp2=(uint8*)&Cache[0] + ((offset+line*320)<<2);	\
	uint32 *outWrite = (uint32*)bitmap.data + offset*3 + soffset + line*bitmap.pitch*3; \
	uint32 *inCache = (uint32*)tmp2;	


void RenderLine_X3 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X3_32;
	do {
		//if (line<2) fprintf(stderr,"%d %d %d %d\n",soffset,offset,outWrite,bitmap.data); 
	        //uint32 * savePtr = outWrite++;
		uint32 p1 = table[*src++];
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
		outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
                outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*2);
		
	} while (--length);
}

void RenderLine_X3S1 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X3_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];

		//LINE
		//p1 = ((p1&0xFEFEFE)>>1)+((p2&0xFEFEFE)>>1);
		p1 = (p1+p2)>>1;

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
		outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
                outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*2);

	} while (--length);
}




void RenderLine_X3S2 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X3_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p3 = *inCache;
	
		*inCache++=p1;	
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
		outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
                outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*2);

	} while (--length);
}

void RenderLine_X3S3 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X3_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		uint32 p3 = *inCache;
		uint32 p4 = p1;
	
		//LINE
		p1 = (p1+p2)>>1;

		*inCache=p1;	
		*inCache++;
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
		outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p1;
                outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*2);

	} while (--length);
}

//DOTs
void RenderLine_X3S4 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X3_32;
	do {
		uint32 p1 = table[*src++];
		uint32 pa = (p1&0xFEFEFE)>>1;
		uint32 p2 = (p1&0x00FF00)|(pa&0xFF00FF);
		uint32 p3 = (p1&0xFF0000)|(pa&0x00FFFF);
		uint32 p4 = (p1&0x0000FF)|(pa&0xFFFF00);

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite = p2;
		outWrite+=(bitmap.pitch-2);
		*outWrite++ = p1;
		*outWrite++ = p3;
		*outWrite = p1;
                outWrite+=(bitmap.pitch-2);
		*outWrite++ = p4;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*2);

	} while (--length);
}

//-------------------------------------------------------------------------------------------

#define PRE_X4_32  \
	src[length]=src[length-1]; \
	int soffset = (bitmap.pitch-(320*4))/2; \
	uint8 *tmp2=(uint8*)&Cache[0] + ((offset+line*320)<<2);	\
	uint32 *outWrite = (uint32*)bitmap.data + offset*4 + soffset + line*bitmap.pitch*4; \
	uint32 *inCache = (uint32*)tmp2;	
	
void RenderLine_X4 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X4_32;
//fprintf(stderr," %d",line);
	do {
		uint32 p1 = table[*src++];
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
		outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*3);

	} while (--length);
}

void RenderLine_X4S1 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X4_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];

		//LINE
		//p1 = ((p1&0xFEFEFE)>>1)+((p2&0xFEFEFE)>>1);
		p1 = (p1+p2)>>1;

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
		outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*3);

	} while (--length);
}

void RenderLine_X4S2 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X4_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p3 = *inCache;
	
		*inCache++=p1;	
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
		outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*3);

	} while (--length);
}

void RenderLine_X4S3 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X4_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		uint32 p3 = *inCache;
		uint32 p4 = p1;
	
		//LINE
		p1 = (p1+p2)>>1;

		*inCache=p1;	
		*inCache++;
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
		outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		outWrite-=(bitmap.pitch*3);

	} while (--length);
}

//DOTs
void RenderLine_X4S4 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X4_32;
	do {
		uint32 p1 = table[*src++];
		uint32 pa = (p1&0xFEFEFE)>>1;
		uint32 p2 = (p1&0x00FF00)|(pa&0xFF00FF);
		uint32 p3 = (p1&0xFF0000)|(pa&0x00FFFF);
		uint32 p4 = (p1&0x0000FF)|(pa&0xFFFF00);

		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite++ = p1;
		*outWrite =   p1;
		outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p2;
		*outWrite++ = p2;
		*outWrite =   p2;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p3;
		*outWrite++ = p3;
		*outWrite =   p4;
                outWrite+=(bitmap.pitch-3);
		*outWrite++ = p1;
		*outWrite++ = p4;
		*outWrite++ = p4;
		*outWrite++ = p4;
		outWrite-=(bitmap.pitch*3);

	} while (--length);
}



/*
void RenderLine_X2S5 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];

		//LINE
		uint32 p3 = (p1+p2)>>1;
		
		*outWrite = p1;
                *(outWrite+640) = p3;
                *outWrite++;
		*outWrite = p3;
                *(outWrite+640) = p3;
                *outWrite++;
	} while (--length);
}


void RenderLine_X2S6 (uint8 *src, int line, int offset, uint32 *table, int length)
{
PRE_X2_32;
	do {
		uint32 p1 = table[*src++];
		uint32 p2 = table[*src];
		uint32 p3 = *inCache;
		uint32 p4 = p1;
	
		//LINE
		p1 = (p1+p2)>>1;

		*inCache=p1;	
		*inCache++;
 
		//PHOSPHOR
 		p1 = (p1+p3)>>1;		

		*outWrite = p4;

                *(outWrite+640) = p1;
                *outWrite++;
		*outWrite = p1;
                *(outWrite+640) = p1;
                *outWrite++;
	} while (--length);
}
*/

/*--------------------------------------------------------------------------*/
/* Window rendering                                                         */
/*--------------------------------------------------------------------------*/

void render_ntw(int line, uint8 *buf)
{
    int column, v_line, width;
    uint32 *nt, *src, *dst, atex, atbuf;

    v_line = (line & 7) << 3;
    width = (vdp.reg[12] & 1) ? 7 : 6;

    nt = (uint32 *)&vdp.vram[vdp.ntwb | ((line >> 3) << width)];
    dst = (uint32 *)&buf[0x20 + (clip[1].left << 4)];

    column = clip[1].left;
    int i=clip[1].right-column;
    if (i>0) do
    {
        atbuf = nt[column++];
        DRAW_COLUMN_A(atbuf, v_line)
    } while (--i);
}

void render_ntw_im2(int line, uint8 *buf)
{
    int column, v_line, width;
    uint32 *nt, *src, *dst, atex, atbuf, offs;

    v_line = ((line & 7) << 1 | ((vdp.status >> 4) & 1)) << 3;
    width = (vdp.reg[12] & 1) ? 7 : 6;

    nt = (uint32 *)&vdp.vram[vdp.ntwb | ((line >> 3) << width)];
    dst = (uint32 *)&buf[0x20 + (clip[1].left << 4)];

    column = clip[1].left;
    int i=clip[1].right-column;
    if (i>0) do
    {
        atbuf = nt[column++];
        DRAW_COLUMN_IM2_A(atbuf, v_line)
    } while (--i);
}

/*--------------------------------------------------------------------------*/
/* Background plane rendering                                               */
/*--------------------------------------------------------------------------*/

void render_ntx(int which, int line, uint8 *buf)
{
    int column;
    int start, end;
    int index;
    int shift;
    int nametable_row_mask = (vdp.playfield_col_mask >> 1);
    int v_line;
    uint32 atex, atbuf, *src, *dst;
    uint16 xascroll, xbscroll, xscroll;
    int y_scroll;
    uint32 *nt;
    int vsr_shift;
    uint32 *vs;
    uint16 table;


    table = (which) ? vdp.ntbb : vdp.ntab;

    get_hscroll(line, &xascroll, &xbscroll);
    xscroll = (which) ? xbscroll : xascroll;

    shift = (xscroll & 0x0F);
    index = ((vdp.playfield_col_mask + 1) >> 1) - ((xscroll >> 4) & nametable_row_mask);

    if(which)
    {
        start = 0;
        end = (vdp.reg[0x0C] & 1) ? 20 : 16;
    }
    else
    {
// Looks correct if clip[0].left has 1 subtracted
// Otherwise window has gap between endpoint and where the first normal
// nta column starts

        if(clip[0].enable == 0) return;
        start = clip[0].left;
        end = clip[0].right;
        index = (index + clip[0].left) & nametable_row_mask;
    }

    vsr_shift = (which) ? 16 : 0;
    vs = (uint32 *)&vdp.vsram[0];

    y_scroll = (vs[0] >> vsr_shift) & 0xFFFF;
    y_scroll = (line + (y_scroll & 0x3FF)) & vdp.playfield_row_mask;
    v_line = (y_scroll & 7) << 3;
    nt = (uint32 *)&vdp.vram[table + (((y_scroll >> 3) << vdp.playfield_shift) & vdp.y_mask)];

    if(shift)
    {
        dst = (uint32 *)&buf[0x20-(0x10-shift)];
        atbuf = nt[(index-1) & nametable_row_mask];
#ifdef ALIGN_LONG
      if (shift&3) {
        DRAW_COLUMN(atbuf, v_line);
	} else 
#endif
        {
        DRAW_COLUMN_A(atbuf, v_line);
	};
    }
    buf = (buf + 0x20 + shift);
    dst = (uint32 *)&buf[start<<4];

    column=start;
    int i=end-start;

if (i>0)
#ifdef ALIGN_LONG
 if (shift&3) {
    do
    {
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN(atbuf, v_line)
    } while (--i);
 } else 
#endif
   {
    do
    {
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN_A(atbuf, v_line)
    } while (--i);
 }
}


void render_ntx_im2(int which, int line, uint8 *buf)
{
    int column;
    int start, end;
    int index;
    int shift;
    int nametable_row_mask = (vdp.playfield_col_mask >> 1);
    int v_line;
    uint32 atex, atbuf, *src, *dst;
    uint16 xascroll, xbscroll, xscroll;
    int y_scroll;
    uint32 *nt;
    int vsr_shift;
    uint32 *vs;
    uint16 table;
    uint32 offs;

    table = (which) ? vdp.ntbb : vdp.ntab;

    get_hscroll(line, &xascroll, &xbscroll);
    xscroll = (which) ? xbscroll : xascroll;

    shift = (xscroll & 0x0F);
    index = ((vdp.playfield_col_mask + 1) >> 1) - ((xscroll >> 4) & nametable_row_mask);

    if(which)
    {
        start = 0;
        end = (vdp.reg[0x0C] & 1) ? 20 : 16;
    }
    else
    {
        if(clip[0].enable == 0) return;
        start = clip[0].left;
        end = clip[0].right;
        index = (index + clip[0].left) & nametable_row_mask;
    }

    vsr_shift = (which) ? 16 : 0;
    vs = (uint32 *)&vdp.vsram[0];

    y_scroll = (vs[0] >> vsr_shift) & 0xFFFF;
    y_scroll = (line + ((y_scroll >> 1) & 0x3FF)) & vdp.playfield_row_mask;
    v_line = (((y_scroll & 7) << 1) | ((vdp.status >> 4) & 1)) << 3;
    nt = (uint32 *)&vdp.vram[table + (((y_scroll >> 3) << vdp.playfield_shift) & vdp.y_mask)];

    if(shift)
    {
        dst = (uint32 *)&buf[0x20-(0x10-shift)];
        atbuf = nt[(index-1) & nametable_row_mask];
#ifdef ALIGN_LONG
      if (shift&3) {
        DRAW_COLUMN_IM2(atbuf, v_line);
	} else
#endif
        {
        DRAW_COLUMN_IM2_A(atbuf, v_line);
	};
    }
    buf = (buf + 0x20 + shift);
    dst = (uint32 *)&buf[start<<4];

    column=start;
    int i=end-start;

if (i>0)
#ifdef ALIGN_LONG
 if (shift&3) {
    do
    {
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN_IM2(atbuf, v_line)
    } while (--i);
 } else
#endif
 {
    do
    {
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN_IM2_A(atbuf, v_line)
    } while (--i);
 }
}


void render_ntx_vs(int which, int line, uint8 *buf)
{
    int column;
    int start, end;
    int index;
    int shift;
    int nametable_row_mask = (vdp.playfield_col_mask >> 1);
    int v_line;
    uint32 atex, atbuf, *src, *dst;
    uint16 xascroll, xbscroll, xscroll;
    int y_scroll;
    uint32 *nt;
    int vsr_shift;
    uint32 *vs;
    uint16 table;

    table = (which) ? vdp.ntbb : vdp.ntab;

    get_hscroll(line, &xascroll, &xbscroll);
    xscroll = (which) ? xbscroll : xascroll;
    shift = (xscroll & 0x0F);
    index = ((vdp.playfield_col_mask + 1) >> 1) - ((xscroll >> 4) & nametable_row_mask);

    if(which)
    {
        start = 0;
        end = (vdp.reg[0x0C] & 1) ? 20 : 16;
    }
    else
    {
        if(clip[0].enable == 0) return;
        start = clip[0].left;
        end = clip[0].right;
        index = (index + clip[0].left) & nametable_row_mask;
    }

    vsr_shift = (which) ? 16 : 0;
    vs = (uint32 *)&vdp.vsram[0];
    end = (vdp.reg[0x0C] & 1) ? 20 : 16;

    if(shift)
    {
        dst = (uint32 *)&buf[0x20-(0x10-shift)];
        y_scroll = (line & vdp.playfield_row_mask);
        v_line = (y_scroll & 7) << 3;
        nt = (uint32 *)&vdp.vram[table + (((y_scroll >> 3) << vdp.playfield_shift) & vdp.y_mask)];
        atbuf = nt[(index-1) & nametable_row_mask];
#ifdef ALIGN_LONG
      if (shift&3) {
        DRAW_COLUMN(atbuf, v_line);
	} else
#endif
	{
        DRAW_COLUMN_A(atbuf, v_line);
	};
    }

    buf = (buf + 0x20 + shift);
    dst = (uint32 *)&buf[start << 4];

    column=start;
    int i=end-start;

if (i>0)
#ifdef ALIGN_LONG
 if (shift&3) {
    do
    {
        y_scroll = (vs[column++] >> vsr_shift) & 0xFFFF;
        y_scroll = (line + (y_scroll & 0x3FF)) & vdp.playfield_row_mask;
        v_line = (y_scroll & 7) << 3;
        nt = (uint32 *)&vdp.vram[table + (((y_scroll >> 3) << vdp.playfield_shift) & vdp.y_mask)];
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN(atbuf, v_line)
    } while (--i);
 } else
#endif
 {
    do
    {
        y_scroll = (vs[column++] >> vsr_shift) & 0xFFFF;
        y_scroll = (line + (y_scroll & 0x3FF)) & vdp.playfield_row_mask;
        v_line = (y_scroll & 7) << 3;
        nt = (uint32 *)&vdp.vram[table + (((y_scroll >> 3) << vdp.playfield_shift) & vdp.y_mask)];
        atbuf = nt[index++ & nametable_row_mask];
        DRAW_COLUMN_A(atbuf, v_line)
    } while (--i);
 }
}
/*--------------------------------------------------------------------------*/
/* Helper functions (cache update, hscroll, window clip)                    */
/*--------------------------------------------------------------------------*/

void update_bg_pattern_cache(void)
{
    //int i;
    uint32 x,y;
    uint8 c;
    uint32 name;

    if(!bg.bg_list_index) return;
    //i=bg.bg_list_index;

    do
    {
        name = bg.bg_name_list[--bg.bg_list_index];
        bg.bg_name_list[bg.bg_list_index] = 0;

        y=8;
	do
        {
            if(bg.bg_name_dirty[name] & (1 << (--y)))
            {
                uint8 *dst = &bg.bg_pattern_cache[name << 6];
                uint32 bp = *(uint32 *)&vdp.vram[(name << 5) | (y << 2)];
		uint8 yt = y<<3;
                x=8;
		do
                {
                    c = (bp >> (((--x) ^ 3) << 2)) & 0x0F; 
                    dst[0x00000 | (yt) | (x)] = (c);
                    dst[0x2<<16 | (yt) | (x ^ 7)] = (c);
                    dst[0x4<<16 | ((y ^ 7) << 3) | (x)] = (c);
                    dst[0x6<<16 | ((y ^ 7) << 3) | (x ^ 7)] = (c);
                } while (x);
            }
        } while (y);
        bg.bg_name_dirty[name] = 0;
    } while (bg.bg_list_index);
    //bg.bg_list_index = 0;
}

void get_hscroll(int line, uint16 *scrolla, uint16 *scrollb)
{
    switch(vdp.reg[11] & 3)
    {
        case 0: /* Full-screen */
            *scrolla = *(uint16 *)&vdp.vram[vdp.hscb + 0];
            *scrollb = *(uint16 *)&vdp.vram[vdp.hscb + 2];
            break;

        case 1: /* First 8 lines */
            *scrolla = *(uint16 *)&vdp.vram[vdp.hscb + ((line & 7) << 2) + 0];
            *scrollb = *(uint16 *)&vdp.vram[vdp.hscb + ((line & 7) << 2) + 2];
            break;

        case 2: /* Every 8 lines */
            *scrolla = *(uint16 *)&vdp.vram[vdp.hscb + ((line & ~7) << 2) + 0];
            *scrollb = *(uint16 *)&vdp.vram[vdp.hscb + ((line & ~7) << 2) + 2];
            break;

        case 3: /* Every line */
            *scrolla = *(uint16 *)&vdp.vram[vdp.hscb + (line << 2) + 0];
            *scrollb = *(uint16 *)&vdp.vram[vdp.hscb + (line << 2) + 2];
            break;
    }

    *scrolla &= 0x03FF;
    *scrollb &= 0x03FF;
}

void window_clip(int line)
{
    /* Window size and invert flags */
    int hp = (vdp.reg[17] & 0x1F);
    int hf = (vdp.reg[17] >> 7) & 1;
    int vp = (vdp.reg[18] & 0x1F) << 3;
    int vf = (vdp.reg[18] >> 7) & 1;

    /* Display size  */
    int sw = (vdp.reg[12] & 1) ? 20 : 16;

    /* Clear clipping data */
    memset(&clip, 0, sizeof(clip));

    /* Check if line falls within window range */
    if(vf == (line >= vp))
    {
        /* Window takes up entire line */
        clip[1].right = sw;
        clip[1].enable = 1;
    }
    else
    {
        /* Perform horizontal clipping; the results are applied in reverse
           if the horizontal inversion flag is set */
        int a = hf;
        int w = hf ^ 1;

        if(hp)
        {
            if(hp > sw)
            {
                /* Plane W takes up entire line */
                clip[w].right = sw;
                clip[w].enable = 1;
            }
            else
            {
                /* Window takes left side, Plane A takes right side */
                clip[w].right = hp;
                clip[a].left = hp;
                clip[a].right = sw;
                clip[0].enable = clip[1].enable = 1;
            }
        }
        else
        {
            /* Plane A takes up entire line */
            clip[a].right = sw;
            clip[a].enable = 1;
        }
    }
}



/*--------------------------------------------------------------------------*/
/* Look-up table functions                                                  */
/*--------------------------------------------------------------------------*/

/* Input (bx):  d5-d0=color, d6=priority, d7=unused */
/* Input (ax):  d5-d0=color, d6=priority, d7=unused */
/* Output:      d5-d0=color, d6=priority, d7=unused */
int make_lut_bg(int bx, int ax)
{
    int bf, bp, b;
    int af, ap, a;
    int x = 0;
    int c;

    bf = (bx & 0x7F);
    bp = (bx >> 6) & 1;
    b  = (bx & 0x0F);
    
    af = (ax & 0x7F);   
    ap = (ax >> 6) & 1;
    a  = (ax & 0x0F);

    c = (ap ? (a ? af : (b ? bf : x)) : \
        (bp ? (b ? bf : (a ? af : x)) : \
        (     (a ? af : (b ? bf : x)) )));

    /* Strip palette bits from transparent pixels */
    if((c & 0x0F) == 0x00) c = (c & 0xC0);

    return (c);
}


/* Input (bx):  d5-d0=color, d6=priority, d7=sprite pixel marker */
/* Input (sx):  d5-d0=color, d6=priority, d7=unused */
/* Output:      d5-d0=color, d6=zero, d7=sprite pixel marker */
int make_lut_obj(int bx, int sx)
{
    int bf, bp, bs, b;
    int sf, sp, s;
    int c;

    bf = (bx & 0x3F);
    bs = (bx >> 7) & 1;
    bp = (bx >> 6) & 1;
    b  = (bx & 0x0F);
    
    sf = (sx & 0x3F);
    sp = (sx >> 6) & 1;
    s  = (sx & 0x0F);

    if(s == 0) return bx;

    if(bs)
    {
        c = bf;
    }
    else
    {
        c = (sp ? (s ? sf : bf)  : \
            (bp ? (b ? bf : (s ? sf : bf)) : \
                  (s ? sf : bf) ));
    }

    /* Strip palette bits from transparent pixels */
    if((c & 0x0F) == 0x00) c = (c & 0xC0);

    return (c | 0x80);
}


/* Input (bx):  d5-d0=color, d6=priority, d7=unused */
/* Input (sx):  d5-d0=color, d6=priority, d7=unused */
/* Output:      d5-d0=color, d6=priority, d7=intensity select (half/normal) */
int make_lut_bg_ste(int bx, int ax)
{
    int bf, bp, b;
    int af, ap, a;
    int gi;
    int x = 0;
    int c;

    bf = (bx & 0x7F);
    bp = (bx >> 6) & 1;
    b  = (bx & 0x0F);
    
    af = (ax & 0x7F);   
    ap = (ax >> 6) & 1;
    a  = (ax & 0x0F);

    gi = (ap | bp) ? 0x80 : 0x00;

    c = (ap ? (a ? af  : (b ? bf  : x  )) : \
        (bp ? (b ? bf  : (a ? af  : x  )) : \
        (     (a ? af : (b ? bf : x)) )));

    c |= gi;

    /* Strip palette bits from transparent pixels */
    if((c & 0x0F) == 0x00) c = (c & 0xC0);

    return (c);
}


/* Input (bx):  d5-d0=color, d6=priority, d7=sprite pixel marker */
/* Input (sx):  d5-d0=color, d6=priority, d7=unused */
/* Output:      d5-d0=color, d6=priority, d7=sprite pixel marker */
int make_lut_obj_ste(int bx, int sx)
{
    int bf, bs;
    int sf;
    int c;

    bf = (bx & 0x7F);   
    bs = (bx >> 7) & 1; 
    sf = (sx & 0x7F);

    if((sx & 0x0F) == 0) return bx;

    c = (bs) ? bf : sf;

    /* Strip palette bits from transparent pixels */
    if((c & 0x0F) == 0x00) c = (c & 0xC0);

    return (c | 0x80);
}


/* Input (bx):  d5-d0=color, d6=priority, d7=intensity (half/normal) */
/* Input (sx):  d5-d0=color, d6=priority, d7=sprite marker */
/* Output:      d5-d0=color, d6=intensity (half/normal), d7=(double/invalid) */
int make_lut_bgobj_ste(int bx, int sx)
{
    int c;

    int bf = (bx & 0x3F);
    int bp = (bx >> 6) & 1;
    int bi = (bx & 0x80) ? 0x40 : 0x00;
    int b  = (bx & 0x0F);

    int sf = (sx & 0x3F);
    int sp = (sx >> 6) & 1;
    int si = (sx & 0x40);
    int s  = (sx & 0x0F);

    if(bi & 0x40) si |= 0x40;

    if(sp)
    {
        if(s)
        {            
            if((sf & 0x3E) == 0x3E)
            {
                if(sf & 1)
                {
                    c = (bf | 0x00);
                }
                else
                {
                    c = (bx & 0x80) ? (bf | 0x80) : (bf | 0x40);
                }
            }
            else
            {
                if(sf == 0x0E || sf == 0x1E || sf == 0x2E)
                {
                    c = (sf | 0x40);
                }
                else
                {
                    c = (sf | si);
                }
            }
        }
        else
        {
            c = (bf | bi);
        }
    }
    else
    {
        if(bp)
        {
            if(b)
            {
                c = (bf | bi);
            }
            else
            {
                if(s)
                {
                    if((sf & 0x3E) == 0x3E)
                    {
                        if(sf & 1)
                        {
                            c = (bf | 0x00);
                        }
                        else
                        {
                            c = (bx & 0x80) ? (bf | 0x80) : (bf | 0x40);
                        }
                    }
                    else
                    {
                        if(sf == 0x0E || sf == 0x1E || sf == 0x2E)
                        {
                            c = (sf | 0x40);
                        }
                        else
                        {
                            c = (sf | si);
                        }
                    }
                }
                else
                {
                    c = (bf | bi);
                }
            }
        }
        else
        {
            if(s)
            {
                if((sf & 0x3E) == 0x3E)
                {
                    if(sf & 1)
                    {
                        c = (bf | 0x00);
                    }
                    else
                    {
                        c = (bx & 0x80) ? (bf | 0x80) : (bf | 0x40);
                    }
                }
                else
                {
                    if(sf == 0x0E || sf == 0x1E || sf == 0x2E)
                    {
                        c = (sf | 0x40);
                    }
                    else
                    {
                        c = (sf | si);
                    }
                }
            }
            else
            {                    
                c = (bf | bi);
            }
        }
    }

    if((c & 0x0f) == 0x00) c = (c & 0xC0);

    return (c);
}

/*--------------------------------------------------------------------------*/
/* Merge functions                                                          */
/*--------------------------------------------------------------------------*/

void merge(uint8 *srca, uint8 *srcb, uint8 *dst, uint8 *table, int width)
{
    int i=width;
    do	
    {
        *dst++ = table[((*srcb++) << 8) | (*srca++)];
    } while (--i);
}

/*--------------------------------------------------------------------------*/
/* Color update functions                                                   */
/*--------------------------------------------------------------------------*/

//16 bit
void color_update_16(int index, uint16 data)
{

if (index==0) { //n0p - well, this should do it ;)
data = *(uint16 *)&vdp.cram[(vdp.border << 1)];
}
    if(vdp.reg[12] & 8)
    {
        pixel_16[0x00 | index] = pixel_16_lut[0][data];
        pixel_16[0x40 | index] = pixel_16_lut[1][data];
        pixel_16[0x80 | index] = pixel_16_lut[2][data];
    }
    else
    {
        uint16 temp = pixel_16_lut[1][data];
        pixel_16[0x00 | index] = temp;
        pixel_16[0x40 | index] = temp;
        pixel_16[0x80 | index] = temp;
    }
}

//32
void color_update_32(int index, uint16 data)
{

if (index==0) { //n0p - well, this should do it ;)
data = *(uint16 *)&vdp.cram[(vdp.border << 1)];
}
    if(vdp.reg[12] & 8)
    {
        pixel_32[0x00 | index] = pixel_32_lut[0][data];
        pixel_32[0x40 | index] = pixel_32_lut[1][data];
        pixel_32[0x80 | index] = pixel_32_lut[2][data];
    }
    else
    {
        uint32 temp = pixel_32_lut[1][data];
        pixel_32[0x00 | index] = temp;
        pixel_32[0x40 | index] = temp;
        pixel_32[0x80 | index] = temp;
    }
}


/*--------------------------------------------------------------------------*/
/* Object render functions                                                  */
/*--------------------------------------------------------------------------*/

void parse_satb(int line)
{
    uint8 *p, *q, link = 0;
    uint16 ypos;

    int count;
    int height;

    int limit = (vdp.reg[12] & 1) ? 20 : 16;
    int total = (vdp.reg[12] & 1) ? 80 : 64;

    object_index_count = 0;

    if (total>0) do
    //for(count = 0; count < total; count += 1)
    {
        q = &vdp.sat[link << 3];
        p = &vdp.vram[vdp.satb + (link << 3)];

        ypos = *(uint16 *)&q[0];

        if(vdp.im2_flag)
            ypos = (ypos >> 1) & 0x1FF;
        else
            ypos &= 0x1FF;

        height =  q[3] & 3; height++; height<<=3;

        if((line >= ypos) && (line < (ypos + height)))
        {
            //object_info[object_index_count].ypos = *(uint16 *)&q[0];
	    object_info[object_index_count].ypos = ypos;
            object_info[object_index_count].xpos = *(uint16 *)&p[6];

            // using xpos from internal satb stops sprite x
            // scrolling in bloodlin.bin,
            // but this seems to go against the test prog
//          object_info[object_index_count].xpos = *(uint16 *)&q[6];
            object_info[object_index_count].attr = *(uint16 *)&p[4];
            object_info[object_index_count].size = q[3];
            object_info[object_index_count].index = count++;

            object_index_count += 1;

            if(object_index_count == limit)
            {
                if(vdp.vint_pending == 0)
                    vdp.status |= 0x40;
                return;
            }
        } 

        link = q[2] & 0x7F;
        if(link == 0) break;
    } while (--total);
}

void render_obj(int line, uint8 *buf, uint8 *table)
{
    uint16 ypos;
    uint16 attr;
    uint16 xpos;
    uint8 size;
    uint8 *src;

    int count;
    int pixellimit = (vdp.reg[12] & 1) ? 320 : 256;
    int pixelcount = 0;
    int width;
    int height;
    int v_line;
    int column;
    int sol_flag = 0;
    int left = 0x80;
    int right = 0x80 + pixellimit;

    uint8 *s, *lb, *lbc;
    uint16 name, index;
    uint8 palette;

    int attr_mask, nt_row;

    if(object_index_count == 0) return;

    for(count = 0; count < object_index_count; count += 1) 
    {
        size = object_info[count].size & 0x0f;
        xpos = object_info[count].xpos;
        xpos &= 0x1ff;

        width = (size>>2) & 3; width++; width<<=3;

        if(xpos != 0) sol_flag = 1;
        else
        if(xpos == 0 && sol_flag) return;

        if(pixelcount > pixellimit) return;
        pixelcount += width;

        if(((xpos + width) >= left) && (xpos < right))
        {
            ypos = object_info[count].ypos;
            //ypos &= 0x1ff; //n0p - this was precalced in parse_satb

            attr = object_info[count].attr;
            attr_mask = (attr & 0x1800);

            height = (size&3); height++; height<<=3;

            palette = (attr >> 9) & 0x70;

            v_line = (line - ypos);
            nt_row = (v_line >> 3) & 3;
            v_line = (v_line & 7) << 3;

            name = (attr & 0x07FF);
            s = &name_lut[((attr >> 3) & 0x300) | (size << 4) | (nt_row << 2)];

            lb = (uint8 *)&buf[0x20 + (xpos - 0x80)];
            lbc = (uint8 *)&sc_buf[0x20 + (xpos - 0x80)];

            width >>= 3;
            column=0;
            int i=width;
            DST_ENTER;
            do
            {
                index = attr_mask | ((name + s[column++]) & 0x07FF);
                src = &bg.bg_pattern_cache[(index << 6) | (v_line)];
                DRAW_SPRITE_TILE;
            } while (--i);
	    DST_EXIT;
    } //if
  } //for
}

void render_obj_im2(int line, uint8 *buf, uint8 *table)
{
    uint16 ypos;
    uint16 attr;
    uint16 xpos;
    uint8 size;
    uint8 *src;

    int count;
    int pixellimit = (vdp.reg[12] & 1) ? 320 : 256;
    int pixelcount = 0;
    int width;
    int height;
    int v_line;
    int column;
    int sol_flag = 0;
    int left = 0x80;
    int right = 0x80 + pixellimit;

    uint8 *s, *lb, *lbc;
    uint16 name, index;
    uint8 palette;
    uint32 offs;

    int attr_mask, nt_row;

    if(object_index_count == 0) return;

    for(count = 0; count < object_index_count; count += 1)
    {
        size = object_info[count].size & 0x0f;
        xpos = object_info[count].xpos;
        xpos &= 0x1ff;

        width = (size>>2) & 3; width++; width<<=3;

        if(xpos != 0) sol_flag = 1;
        else
        if(xpos == 0 && sol_flag) return;

        if(pixelcount > pixellimit) return;
        pixelcount += width;

        if(((xpos + width) >= left) && (xpos < right))
        {
            ypos = object_info[count].ypos;
            //ypos = (ypos >> 1) & 0x1ff; //n0p - this was precalced in parse_satb

            attr = object_info[count].attr;
            attr_mask = (attr & 0x1800);

            height = (size&3); height++; height<<=3;
            palette = (attr >> 9) & 0x70;

            v_line = (line - ypos);
            nt_row = (v_line >> 3) & 3;
            v_line = (((v_line & 7) << 1) | ((vdp.status >> 4) & 1)) << 3;            

            name = (attr & 0x03FF);
            s = &name_lut[((attr >> 3) & 0x300) | (size << 4) | (nt_row << 2)];

            lb = (uint8 *)&buf[0x20 + (xpos - 0x80)];
            lbc = (uint8 *)&sc_buf[0x20 + (xpos - 0x80)];	

            width >>= 3;
	    int i=width;
	    column=0;
            DST_ENTER;
            do
            {
                index = (name + s[column++]) & 0x3ff;
                offs = index << 7 | attr_mask << 6 | v_line;
                if(attr & 0x1000) offs ^= 0x40;
                src = &bg.bg_pattern_cache[offs];
                DRAW_SPRITE_TILE;
            } while (--i);
            DST_EXIT;
    } //if
  } //for
}
