
#ifndef _VDP_H_
#define _VDP_H_

/* Global variables */
extern uint8 is_color_dirty;           
extern uint8 color_dirty[0x40];        
extern uint8 is_border_dirty;          

typedef struct
{
	uint8 bg_name_dirty[0x800];     /* 1= This pattern is dirty */
	uint32 bg_name_list[0x800];     /* List of modified pattern indices */
	uint8 bg_pattern_cache[0x80000]; /* Cached and flipped patterns */
	uint32 bg_list_index;           /* # of modified patterns in list */
}t_bg;

extern t_bg bg;

typedef struct
{
uint8  sat[0x400];               /* Internal copy of sprite attribute table */
uint8  vram[0x10000];            /* Video RAM (64Kx8) */
uint8  cram[0x80];               /* On-chip color RAM (64x9) */
uint8  vsram[0x80];              /* On-chip vertical scroll RAM (40x11) */
uint8  reg[0x20];                /* Internal VDP registers (23x8) */
uint16 addr;                    /* Address register */
uint16 addr_latch;              /* Latched A15, A14 of address */
uint8  code;                     /* vdp.code register */
uint8  pending;                  /* vdp.pending write flag */
uint16 buffer;                  /* Read vdp.buffer */
uint16 status;                  /* VDP vdp.status flags */
uint16 ntab;                    /* Name table A base address */
uint16 ntbb;                    /* Name table B base address */
uint16 ntwb;                    /* Name table W base address */
uint16 satb;                    /* Sprite attribute table base address */
uint16 hscb;                    /* Horizontal scroll table base address */
uint16 sat_base_mask;           /* Base bits of vdp.sat */
uint16 sat_addr_mask;           /* Index bits of vdp.sat */
uint8  border;                   /* Border color index */
uint8  playfield_shift;          /* Width of planes A, B (in bits) */
uint16 playfield_row_mask;      /* Horizontal scroll mask */
uint32 y_mask;                  /* Name table Y-index bits mask */
uint8  playfield_col_mask;       /* Vertical scroll mask */
int    hint_pending;               /* 0= Line interrupt is vdp.pending */
int    vint_pending;               /* 1= Frame interrupt is vdp.pending */
int    dma_fill;                   /* 1= DMA fill has been requested */
int    im2_flag;                   /* 1= Interlace mode 2 is being used */
int    frame_end;                  /* End-of-frame (IRQ line) */
int    v_counter;                  /* VDP scan line vdp.counter */
int    v_update;                   /* 1= VC was updated by a ctrl or HV read */
int    collision;         		/* VDP Sprite collision flag */
uint8  pal;
uint8  dma;
}t_vdp;

extern t_vdp vdp;

/* Function prototypes */
void vdp_init(void);
void vdp_reset(void);
void vdp_shutdown(void);
void vdp_ctrl_w(uint16 data);
uint16 vdp_ctrl_r(void);
void vdp_data_w(uint16 data);
uint16 vdp_data_r(void);
void vdp_reg_w(uint8 r, uint8 d);
uint16 vdp_hvc_r(void);
void dma_copy(void);
void dma_vbus(void);
void vdp_test_w(uint16 value);

#endif /* _VDP_H_ */
