///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Dimitry Ishenko
// Distributed under the GNU GPL v3. For full terms please visit:
// http://www.gnu.org/licenses/gpl.html
//
// Contact: dimitry (dot) ishenko (at) (gee) mail (dot) com
///////////////////////////////////////////////////////////////////////////////
//#define DEBUG

#ifdef DEBUG
    #debug
#else
    #nodebug
#endif

///////////////////////////////////////////////////////////////////////////////
#define STDIO_DISABLE_FLOATS
#define DINBUFSIZE 31
#define DOUTBUFSIZE 15

#define WrPortM(reg, val) WrPortI(reg, & reg ## Shadow, val)

#define set(var, v0, v1, v2, v3, v4, v5, v6, v7) \
  var[0]= v0 "\n"; \
  var[1]= v1 "\n"; \
  var[2]= v2 "\n"; \
  var[3]= v3 "\n"; \
  var[4]= v4 "\n"; \
  var[5]= v5 "\n"; \
  var[6]= v6 "\n"; \
  var[7]= v7 "\n"

///////////////////////////////////////////////////////////////////////////////
typedef char bool;
#define false 0
#define true  1

///////////////////////////////////////////////////////////////////////////////
nodebug root void sleep(unsigned len)
{
    static unsigned long time;

    time= MS_TIMER;

    if(ULONG_MAX-len<time)
        while(MS_TIMER>=time) continue; // wait for MS_TIMER to overlap

    time+= len;
    while(MS_TIMER<time) continue;
};

///////////////////////////////////////////////////////////////////////////////
char* sym[8][8];

///////////////////////////////////////////////////////////////////////////////
byte row_val, column_val;
byte row_idx, column_idx;

bool sent;

///////////////////////////////////////////////////////////////////////////////
nodebug root void setup()
{
  set(sym[0],  "+", "X",  "*", "00",  ".",  "CE", "X",  "0");
  set(sym[1],  "X", "X",  "X",  "X",  "X",   "X", "X",  "X");
  set(sym[2],  "-", "X", "M+",  "5",  "6",   "=", "X",  "4");
  set(sym[3],  "X", "X",  "#",  "2",  "3", "mul", "X",  "1");
  set(sym[4], "M*", "X", "MU", "SE", "MA",  "+-", "X", "CO");
  set(sym[5],  "%", "X", "M-",  "8",  "9", "div", "X",  "7");
  set(sym[6],  "X", "X",  "X",  "X",  "X",   "X", "X", "FE");
  set(sym[7], "T-", "X", "T+", "TC",  "~",   ">", "X", "CA");
}

///////////////////////////////////////////////////////////////////////////////
root void main()
{
  brdInit();
  WrPortM(SPCR,  0x80); // set PA as input
  WrPortM(PBDDR, 0x42); // set PB0,2-5,7 as inputs
  WrPortM(SBCR,  0x00);

  setup();
  sent= false;

  serDopen(9600);

  while(true)
  {
       row_val= ~(RdPortI(PADR) | 0x02);
    column_val= ~(RdPortI(PBDR) | 0x42);

#ifdef DEBUG
    printf("%2x %2x\r", row_val, column_val);
#endif

    for(row_idx=0; row_idx<8; ++row_idx)
    {
      if(row_val&1) goto row_got;

      row_val= row_val >> 1;
    }

row_got:
    for(column_idx=0; column_idx<8; ++column_idx)
    {
      if(column_val&1) goto column_got;

      column_val= column_val >> 1;
    }

column_got:
    if(!sent && column_idx<8 && row_idx<8)
    {
#ifdef DEBUG
      printf(sym[row_idx][column_idx]);
#endif
      serDputs(sym[row_idx][column_idx]);

      sleep(30);
      sent= true;
    }
    else if(column_idx==8 && row_idx==8)
    {
      sleep(30);
      sent= false;
    }
  }
}
