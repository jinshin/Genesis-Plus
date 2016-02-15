/*
    io.c
    I/O controller chip emulation
*/

#include "shared.h"

port_t port[3];
uint8 io_reg[0x10];

extern uint8 sixbuttonpad;

void io_reset(void)
{
    /* I/O register default settings */
    uint8 io_def[0x10] =
    {
        0xA0,
        0x7F, 0x7F, 0x7F,
        0x00, 0x00, 0x00, 
        0xFF, 0x00, 0x00,
        0xFF, 0x00, 0x00,  
        0xFB, 0x00, 0x00,  
    };

    /* Initialize I/O registers */
    memcpy(io_reg, io_def, 0x10);

    /*
        Port A : 3B pad
        Port B : Unused
        Port C : Unused
    */

if(!sixbuttonpad) {
    port[0].data_w = NULL;   
    port[0].data_r = device_3b_r;
} else {
    port[0].data_w = device_6b_w;
    port[0].data_r = device_6b_r;
}
    port[1].data_w = NULL;
    port[1].data_r = NULL;
    port[2].data_w = NULL;
    port[2].data_r = NULL;
}

/*--------------------------------------------------------------------------*/
/* I/O chip functions                                                       */
/*--------------------------------------------------------------------------*/

void gen_io_w(int offset, int value)
{
    switch(offset)
    {
        case 0x01: /* Port A Data */
            //value = ((value & 0x80) | (value & io_reg[offset+3]));
            if(port[0].data_w) port[0].data_w(value);
            io_reg[offset]=value;
            return;

        case 0x02: /* Port B Data */
            //value = ((value & 0x80) | (value & io_reg[offset+3]));
            if(port[1].data_w) port[1].data_w(value);
            io_reg[offset]=value;
            return;

        case 0x03: /* Port C Data */
            //value = ((value & 0x80) | (value & io_reg[offset+3]));
            if(port[2].data_w) port[2].data_w(value);
            io_reg[offset] = value;
            return;  

        case 0x04: /* Port A Ctrl */
        case 0x05: /* Port B Ctrl */
        case 0x06: /* Port C Ctrl */
            io_reg[offset] = value & 0xFF;
            break;

        case 0x07: /* Port A TxData */
        case 0x0A: /* Port B TxData */
        case 0x0D: /* Port C TxData */
            io_reg[offset] = value;
            break;

        case 0x09: /* Port A S-Ctrl */
        case 0x0C: /* Port B S-Ctrl */
        case 0x0F: /* Port C S-Ctrl */
            io_reg[offset] = (value & 0xF8);
            break;
    }
}

int gen_io_r(int offset)
{
    extern uint8 hw;
    uint8 has_scd = 0x20; /* No Sega CD unit attached */
    uint8 gen_ver = 0x00; /* Version 0 hardware */

    switch(offset)
    {
        case 0x00: /* Version */                    
            return (hw | has_scd | gen_ver);
            break;

        case 0x01: /* Port A Data */
            if(port[0].data_r) return ((io_reg[offset] & 0x80) | port[0].data_r());
            return (io_reg[offset] | ((~io_reg[offset+3]) & 0x7F));

        case 0x02: /* Port B Data */
            if(port[1].data_r) return ((io_reg[offset] & 0x80) | port[1].data_r());
            return (io_reg[offset] | ((~io_reg[offset+3]) & 0x7F));

        case 0x03: /* Port C Data */
            if(port[2].data_r) return ((io_reg[offset] & 0x80) | port[2].data_r());
            return (io_reg[offset] | ((~io_reg[offset+3]) & 0x7F));
    }

    return (io_reg[offset]);
}

/*--------------------------------------------------------------------------*/
/* Input callbacks                                                          */
/*--------------------------------------------------------------------------*/

uint8 pad_2b_r(void)
{
    uint8 temp = 0x3F;
    if(input.pad[0].data & INPUT_UP)    temp &= ~0x01;
    if(input.pad[0].data & INPUT_DOWN)  temp &= ~0x02;
    if(input.pad[0].data & INPUT_LEFT)  temp &= ~0x04;
    if(input.pad[0].data & INPUT_RIGHT) temp &= ~0x08;
    if(input.pad[0].data & INPUT_B)     temp &= ~0x10;
    if(input.pad[0].data & INPUT_C)     temp &= ~0x20;
    return (temp);
}

uint8 device_3b_r(void)
{
    uint8 th=io_reg[1]&0x40;
    uint8 temp = 0x3F;
    if(th)
    {
        temp = 0x3f;
        if(input.pad[0].data & INPUT_UP)    temp &= ~0x01;
        if(input.pad[0].data & INPUT_DOWN)  temp &= ~0x02;
        if(input.pad[0].data & INPUT_LEFT)  temp &= ~0x04;
        if(input.pad[0].data & INPUT_RIGHT) temp &= ~0x08;
        if(input.pad[0].data & INPUT_B)     temp &= ~0x10;
        if(input.pad[0].data & INPUT_C)     temp &= ~0x20;
        return (temp | 0x40);
    }
    else
    {
        temp = 0x33;
        if(input.pad[0].data & INPUT_UP)    temp &= ~0x01;
        if(input.pad[0].data & INPUT_DOWN)  temp &= ~0x02;
        if(input.pad[0].data & INPUT_A)     temp &= ~0x10;
        if(input.pad[0].data & INPUT_START) temp &= ~0x20;
        return (temp);
    }
}

uint8 device_6b_r(void)
{
    uint8 th=io_reg[1]&0x40;
    uint32 phase = input.pad[0].phase;
    uint8 temp;

    if(th)
    {

        if (phase==3) {
        temp = 0x3f;
        if(input.pad[0].data & INPUT_Z)    	temp &= ~0x01;
        if(input.pad[0].data & INPUT_Y)  	temp &= ~0x02;
        if(input.pad[0].data & INPUT_X)  	temp &= ~0x04;
        if(input.pad[0].data & INPUT_MODE) 	temp &= ~0x08;
        if(input.pad[0].data & INPUT_B)     	temp &= ~0x10;
        if(input.pad[0].data & INPUT_C)     	temp &= ~0x20;
        return (temp | 0x40); }

    } else {

        if (phase==2) {
        temp = 0x30;
        if(input.pad[0].data & INPUT_A)     temp &= ~0x10;
        if(input.pad[0].data & INPUT_START) temp &= ~0x20;
        return (temp); }

        if (phase==3) {
        temp = 0x3f;
        if(input.pad[0].data & INPUT_A)     temp &= ~0x10;
        if(input.pad[0].data & INPUT_START) temp &= ~0x20;
        return (temp); }
    }

    return device_3b_r();

}

void device_6b_w (uint8 data)
{
      input.pad[0].delay=0;
      if (!(io_reg[1]&0x40)&&(data&0x40)) input.pad[0].phase++;
}
