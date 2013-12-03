/* 
 * utt.c
 * Uart Test Tool for DOS 
 *
 * shinewave<shinewave-c@163.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <conio.h>

#define OFF_RBR 0
#define OFF_THR 0
#define OFF_IER 1
#define OFF_IIR 2
#define OFF_FCR 2
#define OFF_LCR 3
#define OFF_MCR 4
#define OFF_LSR 5
#define OFF_MSR 6
#define OFF_SCR 7
#define OFF_DLL 0
#define OFF_DLH 1

#define DEVIDER_BAUD_1200 96
#define DEVIDER_BAUD_2400 48
#define DEVIDER_BAUD_4800 24
#define DEVIDER_BAUD_9600 12
#define DEVIDER_BAUD_19200 6
#define DEVIDER_BAUD_38400 3
#define DEVIDER_BAUD_115200 1

#define IER_BIT_PTIME 0x80
#define IER_BIT_EDSSI 0x08
#define IER_BIT_ELSI 0x04
#define IER_BIT_ETBEI 0x02
#define IER_BIT_ERBFI 0x01

#define IIR_FIFO_ENABLED 0xC0
#define IIR_IDT_MSC 0x00
#define IIR_IDT_THRE 0x01
#define IIR_IDT_RDA 0x02
#define IIR_IDT_RS 0x03
#define IIR_IDT_CTO 0x06
#define IIR_IDT_OFF 0x01

#define FCR_RFIFO_TL_1 0x00
#define FCR_RFIFO_TL_4 0x01
#define FCR_RFIFO_TL_8 0x02
#define FCR_RFIFO_TL_14 0x03
#define FCR_RFIFO_TL_OFF 0x06
#define FCR_TFIFO_TL_0 0x00
#define FCR_TFIFO_TL_2 0x01
#define FCR_TFIFO_TL_QUAD 0x02
#define FCR_TFIFO_TL_HALF 0x03
#define FCR_TFIFO_TL_OFF 0x04
#define FCR_TFIFO_CLS 0x04
#define FCR_RFIFO_CLS 0x02
#define FCR_BIT_FIFO_ENABLE 0x01

#define LCR_BIT_DLAB 0x80
#define LCR_BIT_BREAK_CRTL 0x40
#define LCR_BIT_PARITY_EVEN 0x10
#define LCR_BIT_PEN 0x08
#define LCR_BIT_STOP_1P5 0x04
#define LCR_BITS_5 0
#define LCR_BITS_6 1
#define LCR_BITS_7 2
#define LCR_BITS_8 3

#define MCR_BIT_LOOPBACK 0x10
#define MCR_BIT_OUT2 0x08
#define MCR_BIT_OUT1 0x04
#define MCR_BIT_RTS 0x02
#define MCR_BIT_DTS 0x01

#define LSR_RFIFO_ERR 0x80
#define LSR_TRANS_EMPTY 0x40
#define LSR_TRANSH_EMPTY 0x20
#define LSR_BIT_BI 0x10
#define LSR_BIT_FE 0x08
#define LSR_BIT_PE 0x04
#define LSR_BIT_OE 0x02
#define LSR_BIT_DR 0x01

#define MSR_OUT2 0x80
#define MSR_OUT1 0x40
#define MSR_DTS 0x20
#define MSR_RTS 0x10
#define MSR_DDCD 0x08
#define MSR_TERI 0x04
#define MSRDDSR 0x02
#define MSR_DCTS 0x01

#define PIC_ICR 0x20
#define PIC_IMR 0x21

#define BUFF_SIZE 1024

unsigned char buff[BUFF_SIZE];
int in_int_count = 0;
int in_int_count1 = 0;
int out_int_count = 0;
int out_int_count1 = 0;
int poll_read;
int in_count = 0;
int data_idx = 0;

unsigned char irq;
unsigned char intnum;
int base;
int tmp;
unsigned char conf;
unsigned char divider;

unsigned char last_iir;
unsigned char last_lsr;

void interrupt (*old_isr)();

int tdelay(int c)
{
    int i,j,k;

    for(i=0;i<c;i++)
        for(j=0;j<c;j++)
            for(k=0;k<c;k++)
                tmp=i+j+k;
    return 0;
}

void interrupt far uart_isr()
{
    unsigned char type = 0xFF;

    last_iir = inp(base + OFF_IIR);
    last_lsr = inp(base + OFF_LSR);
    type = (last_iir>>IIR_IDT_OFF) & 0x07;

    if(type == IIR_IDT_RDA){
        in_int_count++;
        if(!poll_read){
            while(inp(base + OFF_LSR) & LSR_BIT_DR){
                buff[in_count++] = inp(base + OFF_RBR);
                in_count = in_count % BUFF_SIZE;
            };
        }
    }
    else if (type == IIR_IDT_THRE){
        out_int_count++;
    }
    else if (type == IIR_IDT_CTO){
        inp(base + OFF_RBR);
    }
    else if (type == IIR_IDT_MSC){
        inp(base + OFF_MSR);
    }

    outportb(PIC_ICR, 0x20);
}

void help()
{
    printf("UART Test Tool for DOS.\n");
    printf("shinewave-c@163.com\n");
    printf("usage: utt <I/O base> <irq> <poll_mode>\n");
    printf("        I/O base: Hex value starting with \"0x\"");
    printf("        irq: irq line");
    printf("        poll_mode: 1 to enable");
}

int main(int argc, char* argv[])
{
    int i;
    unsigned char c = 0;
    unsigned char ch = 0;

    if(argc != 4){
        help();
        return 1;
    }
    else {
        irq = atoi(argv[2]);
        base = strtoul(argv[1], 0 , 16);
        poll_read = atoi(argv[3]);
    }
    conf = LCR_BITS_8;
    intnum = irq + 8;


    outp(base + OFF_LCR, LCR_BIT_DLAB);
    outp(base + OFF_DLL, DEVIDER_BAUD_115200);
    outp(base + OFF_DLH, 0);

    outp(base + OFF_LCR, conf);
    outp(base + OFF_FCR, FCR_TFIFO_CLS | FCR_RFIFO_CLS | FCR_BIT_FIFO_ENABLE);
    outp(base + OFF_MCR, MCR_BIT_OUT2 | MCR_BIT_RTS | MCR_BIT_DTS);

    outp(base + OFF_IER, 0);

    printf("\nUART Test Tool for DOS. Press ESC to quit\n");

    if(!poll_read){
        old_isr = getvect(intnum);
        setvect(intnum, uart_isr);
        outp(PIC_IMR, inp(PIC_IMR) & ~(1 << irq));
        outp(base + OFF_IER, IER_BIT_ERBFI | IER_BIT_ETBEI);
        printf("Working in Interrupt Mode\n");
    }
    else
        printf("Working in Poll Mode\n");

    do{
        if(poll_read){
            while(inp(base + OFF_LSR) & LSR_BIT_DR){
                buff[in_count++] = inp(base + OFF_RBR);
                in_count = in_count % BUFF_SIZE;
            };
        }

        while(data_idx != in_count){
            ch = buff[data_idx++];
            data_idx = data_idx % BUFF_SIZE;
            printf("0x%02x ", ch);
        }

        c = 0;
        if (kbhit()){
            c = getch();
            outportb(base, c);
        }

        if((in_int_count1 != in_int_count) || (out_int_count1 != out_int_count)){
            printf("\nint count: %d in, %d out, iir: 0x%02x, lsr: 0x%2x\n", in_int_count, out_int_count, last_iir, last_lsr);
            in_int_count1 = in_int_count;
            out_int_count1 = out_int_count;
        }
    }while(c != 27);

    outp(base + OFF_MCR, 0);
    outp(base + OFF_IER, 0);

    if(!poll_read){
        outp(PIC_IMR, inportb(PIC_IMR) | (1<<irq));
        setvect(intnum, old_isr);
    }

    return 0;
}
