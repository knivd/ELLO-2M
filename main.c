/*
 * ELLO 2M keyboard controller
 * chip PIC16F1517, compiler XC8 v1.36 (Lite)
 * author KnivD (C) 11/2015 - 2016
 */

#include <xc.h>
#include <string.h>
#include <stdio.h>

// firmware version string
#define VERSION "1605.2"

#pragma config CLKOUTEN = OFF
#pragma config WDTE = SWDTEN
#pragma config PWRTE = OFF
#pragma config CP = ON
#pragma config BOREN = OFF
#pragma config FCMEN = OFF
#pragma config MCLRE = ON
#pragma config IESO = OFF
#pragma config FOSC = INTOSC
#pragma config STVREN = ON
#pragma config LPBOR = OFF
#pragma config BORV = LO
#pragma config LVP = OFF
#pragma config VCAPEN = OFF
#pragma config WRT = OFF    // ALL

#define _XTAL_FREQ  8000000

#define ARDLY_DEFAULT   40  // autorepeat initial delay
#define ARCYC_DEFAULT   4   // autorepeat cycle (lower is faster faster autorepeat)

#define DBL_PRESS       20  // double key press maximum timeout

#define ACCT    PORTEbits.RE0
#define SHFT    PORTEbits.RE1
#define CTRL    PORTEbits.RE2

#define ROW0    PORTBbits.RB0
#define ROW1    PORTBbits.RB1
#define ROW2    PORTBbits.RB2
#define ROW3    PORTBbits.RB3
#define ROW4    PORTBbits.RB4

#define COL0    LATAbits.LATA0
#define COL1    LATAbits.LATA1
#define COL2    LATAbits.LATA2
#define COL3    LATAbits.LATA3
#define COL4    LATDbits.LATD3
#define COL5    LATDbits.LATD2
#define COL6    LATDbits.LATD1
#define COL7    LATDbits.LATD0
#define COL8    LATAbits.LATA4
#define COL9    LATAbits.LATA5
#define COL10   LATAbits.LATA6
#define COL11   LATAbits.LATA7
#define COL12   LATDbits.LATD4
#define COL13   LATDbits.LATD5
#define COL14   LATDbits.LATD6
#define COL15   LATDbits.LATD7

#define KBLT    LATCbits.LATC3
#define SYSPW   LATCbits.LATC4
#define nMCLR   TRISCbits.TRISC5

#define nLEDR   TRISCbits.TRISC2
#define nLEDG   TRISCbits.TRISC1
#define nLEDB   TRISCbits.TRISC0

#define RGB(r,g,b) { nLEDR=!(r); nLEDG=!(g); nLEDB=!(b); }

#define BIT(x)      (1ul<<(x))
#define bSwap(x,b1,b2)  x=(((x) & ~(BIT(b1) | BIT(b2))) | ((!!((x) & BIT(b1)))<<(b2)) | ((!!((x) & BIT(b2)))<<(b1)))

// character and control codes table
const unsigned char skey[5][5][16][9] = {   // registers, rows, columns, sub-characters

// lower register (normal)
{
    {"\x1b",     "1",        "2",        "3",        "4",        "5",        "6",        "7",        "8",        "9",        "0",        "-",        "=",        "\x1b[3~",  "\x08",     ""},           // row 0
    {"",         "\x09",     "q",        "w",        "e",        "r",        "t",        "y",        "u",        "i",        "o",        "p",        "[",        "]",        "",         "\x0d"},       // row 1
    {"",         "",         "a",        "s",        "d",        "f",        "g",        "h",        "j",        "k",        "l",        ";",        "'",        "@",        "",         ""},           // row 2
    {"",         "",         "",         "z",        "x",        "c",        "v",        "b",        "n",        "m",        ",",        ".",        "\x1b[1~",  "\x1b[A",   "\x1b[4~",  "\x1b[5~"},    // row 3
    {"",         "",         "\\",       "",         "",         "",         "",         "",         "",         "",         "\x20",     "/",        "\x1b[D",   "\x1b[B",   "\x1b[C",   "\x1b[6~"},    // row 4
}, //col0        col1        col2        col3        col4        col5        col6        col7        col8        col9        col10       col11       col12       col13       col14       col15

// upper register ('Shift' pressed or 'Shift Caps' active)
{
    {"\x1b",     "!",        "\xa3",     "#",        "$",        "%",        "^",        "&",        "*",        "(",        ")",        "_",        "+",        "\x1b[2~",  "\x08",     ""},           // row 0
    {"",         "\x1b[Z",   "Q",        "W",        "E",        "R",        "T",        "Y",        "U",        "I",        "O",        "P",        "{",        "}",        "",         "\x0d"},       // row 1
    {"",         "",         "A",        "S",        "D",        "F",        "G",        "H",        "J",        "K",        "L",        ":",        "\"",       "~",        "",         ""},           // row 2
    {"",         "",         "",         "Z",        "X",        "C",        "V",        "B",        "N",        "M",        "<",        ">",        "\x1b[1~",  "\x1b[A",   "\x1b[4~",  "\x1b[5~"},    // row 3
    {"",         "",         "\x80",     "",         "",         "",         "",         "",         "",         "",         "\x20",     "?",        "\x1b[D",   "\x1b[B",   "\x1b[C",   "\x1b[6~"},    // row 4
}, //col0        col1        col2        col3        col4        col5        col6        col7        col8        col9        col10       col11       col12       col13       col14       col15

// accent characters ('Accent' pressed or 'Accent Lock' active)
{
    {"\x1b",     "\xb9",     "\xb2",     "\xb3",     "\xdf",     "\xea",     "",         "\xec",     "\xaa",     "\xed",     "\xf8",     "\xad",     "\xac",     "\x7f",     "\x08",     ""},           // row 0
    {"",         "\x09",     "\xe0",     "\xe1",     "\xeb",     "\xe8",     "\xf9",     "\xfd",     "\xfc",     "\xef",     "\xd6",     "\xde",     "\xe6",     "\xbd",     "",         "\x0d"},       // row 1
    {"",         "",         "\xe4",     "\xa8",     "\xf0",     "\xe9",     "\xfb",     "\xfa",     "\xee",     "\xd3",     "\xd4",     "\xff",     "\xb5",     "\xa9",     "",         ""},           // row 2
    {"",         "",         "",         "\xb8"      "\xe3",     "\xe7",     "\xe5",     "\xd5",     "\xf1",     "\xd2",     "\xb0",     "\xb7",     "\x1b[1~",  "\x1b[A",   "\x1b[4~",  "\x1b[5~"},    // row 3
    {"",         "",         "\xe2",     "",         "",         "",         "",         "",         "",         "",         "\xa0",     "\xf7",     "\x1b[D",   "\x1b[B",   "\x1b[C",   "\x1b[6~"},    // row 4
}, //col0        col1        col2        col3        col4        col5        col6        col7        col8        col9        col10       col11       col12       col13       col14       col15

// upper accent characters (('Accent' pressed or 'Accent Lock' active) + ('Shift' pressed or 'Caps Lock') active)
{
    {"\x1b",     "\xa1",     "\xa5",     "\xa7",     "\xa2",     "\xca",     "\xba",     "\xcc",     "\xd7",     "\xcd",     "\xd8",     "\xaf",     "\xb1",     "\x7f",     "\x08",     ""},           // row 0
    {"",         "\x1b[Z",   "\xc0",     "\xc1",     "\xcb",     "\xc8",     "\xd9",     "\xdd",     "\xdc",     "\xcf",     "\xf6",     "\xfe",     "\xc6",     "\xbc",     "",         "\x0d"},       // row 1
    {"",         "",         "\xc4",     "\xa6",     "\xd0",     "\xc9",     "\xdb",     "\xda",     "\xce",     "\xf3",     "\xf4",     "\xbe",     "\xb6",     "\xae",     "",         ""},           // row 2
    {"",         "",         "",         "\xb4",     "\xc3",     "\xc7",     "\xc5",     "\xf5",     "\xd1",     "\xf2",     "\xab",     "\xbb",     "\x1b[1~",  "\x1b[A",   "\x1b[4~",  "\x1b[5~"},    // row 3
    {"",         "",         "\xc2",     "",         "",         "",         "",         "",         "",         "",         "\xa0",     "\xbf",     "\x1b[D",   "\x1b[B",   "\x1b[C",   "\x1b[6~"},    // row 4
}, //col0        col1        col2        col3        col4        col5        col6        col7        col8        col9        col10       col11       col12       col13       col14       col15

// control codes ('Ctrl' pressed)
{
    {"",         "\x1b[11~", "\x1b[12~", "\x1b[13~", "\x1b[14~", "\x1b[15~", "\x1b[17~", "\x1b[18~", "\x1b[19~", "\x1b[20~", "\x1b[21~", "\x1b[23~", "\x1b[24~", "",         "",         ""},           // row 0
    {"",         "",         "\x11",     "\x17",     "\x05",     "\x12",     "\x14",     "\x19",     "\x15",     "\x09",     "\x0f",     "\x10",     "\x1b",     "\x1d",     "",         ""},           // row 1
    {"",         "",         "\x01",     "\x13",     "\x04",     "\x06",     "\x07",     "\x08",     "\x0a",     "\x0b",     "\x0c",     "",         "",         ""     ,    "",         ""},           // row 2
    {"",         "",         "",         "\x1a",     "\x18",     "\x03",     "\x16",     "\x02",     "\x0e",     "\x0d",     "",         "\x1e",     "",         "",         "",         ""},           // row 3
    {"",         "",         "\x1c",     "",         "",         "",         "",         "",         "",         "",         "",         "\x1f",     "",         "",         "",         ""},           // row 4
}, //col0        col1        col2        col3        col4        col5        col6        col7        col8        col9        col10       col11       col12       col13       col14       col15

}; // end of skey[] definition

// flag masks
#define F_POWER     0b00000001
#define F_KEY_SHFT  0b00000010
#define F_KEY_CTRL  0b00000100
#define F_KEY_ACCT  0b00001000
#define F_KEY_NCAPS 0b00010000
#define F_KEY_ALOCK 0b00100000
#define F_ESC_SEQ   0b01000000

unsigned char flags = 0;

#define CNTR_ON     0
#define CNTR_OFF    1
#define CNTR_RST    2
#define CNTR_BAT    3
#define CNTR_BLINK  4

unsigned char cntr[5];      // system event counters

unsigned char pflags = 0;
unsigned char s_cntr = 0;   // SHIFT release-press cycle counter
unsigned char a_cntr = 0;   // ACCENT release-press cycle counter
unsigned long ardelay = ARDLY_DEFAULT;
unsigned long arcycle = ARCYC_DEFAULT;
unsigned int k[5] = {-1,-1,-1,-1,-1};   // key bits; each element represents a keyboard row and the bits inside - the columns
unsigned int pk[5] = {-1,-1,-1,-1,-1};  // key bits in the previous cycle
unsigned char arcntr[5][16];            // autorepeat counters

#define RXBUF_SIZE 16       // size of UART reception buffer

unsigned char rxbuf[RXBUF_SIZE];        // UART reception buffer
unsigned char rxind = 0;    // index to the next incoming character position in the RX buffer

#define CTL_SEQS    12      // number of supported incoming control sequences

// valid incoming control sequences
const unsigned char ctlseq[CTL_SEQS][12] = {
    "\x1b;rst\r",   // reset the keyboard controller (system off)
    "\x1b;ver\r",   // enquire device firmware version
    "\x1b;bat\r",   // enquire about battery status
    "\x1b;game\r",  // gaming mode - no delays, no autorepeat
    "\x1b;ar0\r",   // disable autorepeat
    "\x1b;ar1\r",   // autorepeat mode 1
    "\x1b;ar2\r",   // autorepeat mode 2
    "\x1b;ar3\r",   // autorepeat mode 3
    "\x1b;ar4\r",   // autorepeat mode 4 (default)
    "\x1b;ar5\r",   // autorepeat mode 5
    "\x1b;ar6\r",   // autorepeat mode 6
    "\x1b;ar7\r"    // autorepeat mode 7
};


// printf() hook
void putch(char ch) {
    while(!TXSTAbits.TRMT) continue;
    TXREG=(unsigned char)ch;
}


// interrupts handler
void interrupt isr(void) {
    if(PIR1bits.RCIF) {
        unsigned char d=RCREG;
        if(rxind || (rxind==0 && d=='\x1b')) {  // waiting for an escape sequence
            if(rxind && d=='\x1b') rxind=0;     // reset the index upon every Esc code
            if(rxind<RXBUF_SIZE) rxbuf[rxind++]=d; else rxind=0;
            if(rxind<RXBUF_SIZE) rxbuf[rxind]=0;    // clear the next character
            if(rxind && d=='\r') flags|=F_ESC_SEQ;  // set the flag for received escape sequence
        }
        PIR1bits.RCIF=0;
    }
}


// scan the keyboard matrix and set bits in (k[]) to correspond to the pressed keys
// also serves the three special keys and set flags F_KEY_xxx in (flags)
// performs de-bouncing filtering
void scan(void) {
    unsigned char r;    // row counter
    unsigned char t;    // de-bouncing iteration counter
    unsigned char f = (F_KEY_SHFT | F_KEY_CTRL | F_KEY_ACCT);   // internal flag storage for the special keys
    for(r=0; r<5; r++) k[r]=-1;
    for(t=3; t; t--) {
        for(r=0; r<5; r++) {
            unsigned char c=16;     // cycle counter
            unsigned int m=0xfffe;  // columns mask
            unsigned int kt=0;      // temporary value for k[r] within the current iteration
            while(c--) {
                LATA=(m & 0xff);
                LATD=((m>>8) & 0xff);
                kt=(kt<<1) + !(PORTB & BIT(r)); // any pressed key will appear as a set bit in the key bits
                m=(m<<1)+1;
            }

            // re-ordering the (kt) bits as per the 2M's schematic
            // swap bit pairs 0-15, 1-14, 2-13, 3-12, 8-11, 9-10
            // ---------------------------------------------------
            bSwap(kt,0,15); // swap bits 0 and 15
            bSwap(kt,1,14); // swap bits 1 and 14
            bSwap(kt,2,13); // swap bits 2 and 13
            bSwap(kt,3,12); // swap bits 3 and 12
            bSwap(kt,8,11); // swap bits 8 and 11
            bSwap(kt,9,10); // swap bits 9 and 10
            // ---------------------------------------------------

            k[r]&=kt;
        }
        if(SHFT) f&=~F_KEY_SHFT;
        if(CTRL) f&=~F_KEY_CTRL;
        if(ACCT) f&=~F_KEY_ACCT;
        if(t) __delay_ms(5);
    }
    flags&=~(F_KEY_SHFT | F_KEY_CTRL | F_KEY_ACCT);
    flags|=f;
}


void main(void) {
    OSCCONbits.IRCF=0b1110; // 8MHz clock

    ANSELA=0b00000000;
    LATA=  0b00000000;
    TRISA= 0b00000000;

    ANSELB=0b11100000;  // keep ANSB5 enabled for CHGAN
    LATB=  0b00000000;
    TRISB= 0b00111111;

    ANSELC=0b00000000;
    LATC=  0b00000000;  // keep the power to system off by default
    TRISC= 0b11100111;  // RC0..RC2 and RC5 will be emulating open drain output operation

    ANSELD=0b00000000;
    LATD=  0b00000000;
    TRISD= 0b00000000;

    ANSELE=0b00000000;

    // configure the ADC
    ADCON0bits.CHS=0b01101; // select AN13
    ADCON1bits.ADCS=0b101;  // ADC clock Fosc/16

    // initialise the UART
    unsigned long b=(((unsigned long)_XTAL_FREQ/4)/38400)-1;
    RCSTAbits.SPEN=0;
    SPBRGL=(unsigned char)(b & 0xff);
    SPBRGH=(unsigned char)((b>>8) & 0xff);
    BAUDCONbits.BRG16=1;
    TXSTAbits.BRGH=1;
    TXSTAbits.SYNC=0;
    TXSTAbits.TXEN=1;
    RCSTAbits.CREN=1;
    PIE1bits.RCIE=1;
    PIR1bits.RCIF=0;

    RGB(1,1,1);
    __delay_ms(500);
    memcpy(pk,k,sizeof(k));
    memset(cntr,0,sizeof(cntr));
    memset(arcntr,0,sizeof(arcntr));
    memset(rxbuf,0,sizeof(rxbuf));
    flags=0;

    // main loop
    WDTCONbits.WDTPS=0b01001;   // 512ms WDT period
    WDTCONbits.SWDTEN=1;        // enable WDT
    INTCONbits.PEIE=1;          // enable peripheral interrupts
    INTCONbits.GIE=1;           // enable interrupts
    while(1) {

        CLRWDT();
        scan();
        if(flags & F_POWER) {   // here when powered on
            unsigned char r, g, b;

            SYSPW=1;    // keep system power on

            // hold CTRL-\ for battery status
            if((flags & F_KEY_CTRL) && !(flags & (F_KEY_SHFT | F_KEY_ACCT)) && (k[4] & BIT(2))) cntr[CNTR_BAT]++; else cntr[CNTR_BAT]=0;
            if(cntr[CNTR_BAT]>=50) {
                memset(cntr,0,sizeof(cntr));

                // ### TODO

                continue;
            }

            // hold CTRL-Esc for system reset
            if((flags & F_KEY_CTRL) && !(flags & (F_KEY_SHFT | F_KEY_ACCT)) && (k[0] & BIT(0))) cntr[CNTR_RST]++; else cntr[CNTR_RST]=0;
            if(cntr[CNTR_RST]>=150) {
                RGB(1,0,1);
                memset(cntr,0,sizeof(cntr));
                flags=F_POWER;
                ardelay=ARDLY_DEFAULT;
                arcycle=ARCYC_DEFAULT;
                memcpy(pk,k,sizeof(k));
                RCSTAbits.SPEN=0;   // disable UART
                nMCLR=0;
                __delay_ms(100);
                nMCLR=1;
                RCSTAbits.SPEN=1;   // enable UART
                rxind=0;
                continue;
            }

            // hold CTRL-Down for power off
            if((flags & F_KEY_CTRL) && !(flags & (F_KEY_SHFT | F_KEY_ACCT)) && (k[4] & BIT(13))) cntr[CNTR_OFF]++; else cntr[CNTR_OFF]=0;
            if(cntr[CNTR_OFF]>=150) {
                RGB(1,0,0);
                memset(cntr,0,sizeof(cntr));
                flags=0;
                ardelay=ARDLY_DEFAULT;
                arcycle=ARCYC_DEFAULT;
                memcpy(pk,k,sizeof(k));
                RCSTAbits.SPEN=0;   // disable UART
                SYSPW=0;    // system power off
                continue;
            }

            // detect double press for CAPS LOCK and ACCENT LOCK
            if(flags & F_KEY_CTRL) s_cntr=0;
            if((flags & F_KEY_SHFT)==0 && (pflags & F_KEY_SHFT) && !s_cntr) s_cntr=1;   // detect released SHIFT
            if((pflags & F_KEY_SHFT)==0 && (flags & F_KEY_SHFT) && s_cntr) {    // detect pressed SHIFT
                if(s_cntr<=DBL_PRESS) flags^=F_KEY_NCAPS;   // check if it is a second one within given time
                s_cntr=256-DBL_PRESS;
            }
            if(s_cntr) s_cntr++;
            if((flags & F_KEY_ACCT)==0 && (pflags & F_KEY_ACCT) && !a_cntr) a_cntr=1;   // detect released ACCENT
            if((pflags & F_KEY_ACCT)==0 && (flags & F_KEY_ACCT) && a_cntr) {    // detect pressed ACCENT
                if(a_cntr<=DBL_PRESS) flags^=F_KEY_ALOCK;   // check if it is a second one within given time
                a_cntr=256-DBL_PRESS;
            }
            if(a_cntr) a_cntr++;
            pflags=flags;

            // indications when system in on
            if((flags & F_KEY_NCAPS) && (flags & F_KEY_ALOCK)) { r=0; g=b=1; }  // accent caps lock - CYAN
            else if(flags & F_KEY_NCAPS) { r=g=b=1; }       // caps lock - WHITE
            else if(flags & F_KEY_ALOCK) { r=g=1; b=0; }    // accent lock - YELLOW
            else { r=g=0; b=1; }                            // normal indication system on - BLUE
            RGB(r,g,b);

            // record the pressed keys
            if(flags & F_KEY_CTRL) g=4;
            else if((flags & (F_KEY_SHFT | F_KEY_NCAPS)) && (flags & (F_KEY_ACCT | F_KEY_ALOCK))) g=3;
            else if(flags & (F_KEY_ACCT | F_KEY_ALOCK)) g=2;
            else if(flags & (F_KEY_SHFT | F_KEY_NCAPS)) g=1;
            else g=0;
            for(r=0; r<5; r++) {
                for(b=0; b<16; b++) {
                    if(k[r] & BIT(b)) {
                        s_cntr=a_cntr=0;
                        if(skey[g][r][b] && *skey[g][r][b]) {
                            if((pk[r] & BIT(b))==0 || ardelay==0 || arcycle==0) {
                                printf("%s",skey[g][r][b]); // send the newly pressed keys
                            } else {
                                if(++arcntr[r][b]>ardelay && (arcntr[r][b]%arcycle)==0) printf("%s",skey[g][r][b]); // autorepeat
                            }
                        }
                    } else
                        arcntr[r][b]=0;
                }
                pk[r]=k[r]; // store the current k[r]
            }

            // checking for valid escape sequences
            if(flags & F_ESC_SEQ) {
                unsigned char t;
                for(t=0; t<CTL_SEQS; t++) {
                    if(!strcmp(rxbuf,ctlseq[t])) {
                        switch(t) {
                            default: break;
                            case 0: RESET(); break;
                            case 1: printf("%s",VERSION); break;
                            case 2: break;  // ### reserved for battery test results
                            case 3: ardelay=0; arcycle=0; break;
                            case 4: ardelay=~0; arcycle=~0; break;
                            case 5: ardelay=30; arcycle=2; break;
                            case 6: ardelay=30; arcycle=3; break;
                            case 7: ardelay=40; arcycle=3; break;
                            case 8: ardelay=40; arcycle=4; break;
                            case 9: ardelay=55; arcycle=4; break;
                            case 10: ardelay=55; arcycle=5; break;
                            case 11: ardelay=60; arcycle=6; break;
                        }
                        break;  // exit the loop
                    }
                }
                rxind=0;
                flags&=~F_ESC_SEQ;
            }

        } else {    // here when NOT powered on

            SYSPW=0;    // keep system power off

            // blink indication every 10s
            if(++cntr[CNTR_BLINK]>=20) {
                cntr[CNTR_BLINK]=0;
                ADCON0bits.ADON=1;
                __delay_us(50);
                ADCON0bits.GO_nDONE=1;
                while(ADCON0bits.GO_nDONE) continue;
                ADCON0bits.ADON=0;
                if(ADRESH>=0xf0) { RGB(1,0,0); }        // not charging
                else if(ADRESH<=0x0f) { RGB(1,1,0); }   // charging
                else { RGB(0,0,0); }                    // charger fault (NTC or bad battery)
                __delay_ms(25);
            }

            // hold CTRL-Up for power on
            if((flags & F_KEY_CTRL) && !(flags & (F_KEY_SHFT | F_KEY_ACCT)) && (k[3] & BIT(13))) cntr[CNTR_ON]++; else cntr[CNTR_ON]=0;
            if(cntr[CNTR_ON]>=5) {
                memset(cntr,0,sizeof(cntr));
                flags=F_POWER;
                ardelay=ARDLY_DEFAULT;
                arcycle=ARCYC_DEFAULT;
                memcpy(pk,k,sizeof(k));
                SYSPW=1;    // system power on
                RCSTAbits.SPEN=1;   // enable UART
                rxind=0;
            }

            RGB(0,0,0);
            SLEEP();

        }
    }
    return;
}
