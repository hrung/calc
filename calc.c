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
char* sym[8][8];

///////////////////////////////////////////////////////////////////////////////
struct input
{
  bool state;
  bool ready;

  byte count;
  byte tally;
}
input_a[8], input_b[8], *pa, *pb;

byte val_a, val_b;
byte bit_a, bit_b;

///////////////////////////////////////////////////////////////////////////////
nodebug root void setup()
{
  set(sym[0],   "9", "X",  "*",  "1",  "5",  " ", "X", "+-");
  set(sym[1],   "X", "X",  "X",  "X",  "X",  "X", "X",  "X");
  set(sym[2],   ".", "X",  " ",  "3",  "7",  " ", "X",  " ");
  set(sym[3],   "0", "X", "00",  "2",  "6",  "L", "X", "CE");
  set(sym[4], "mul", "X", "GT", "M+",  "#",  " ", "X",  " ");
  set(sym[5],   "+", "X",  ">",  "4",  "8",  " ", "X",  " ");
  set(sym[6],   "-", "X", "T-", "M#", "M*", "T+", "X",  " ");
  set(sym[7], "div", "X", "MU", "M-",  "%",  " ", "X",  " ");
}

///////////////////////////////////////////////////////////////////////////////
nodebug root void read()
{
  int i, n;
  
  for(n=0; n<250; ++n)
  {
    val_a= RdPortI(PADR);
    val_b= RdPortI(PBDR);

    for(i=0, pa=input_a, pb=input_b; i<8; ++i, ++pa, ++pb)
    {
      bit_a= val_a&1;
      bit_b= val_b&1;
    
      if(bit_a != pa->state)
      {
	pa->state= bit_a;
	++(pa->count);
      }
	
      if(bit_b != pb->state)
      {
	pb->state= bit_b;
	++(pb->count);
      }

      val_a= val_a>>1;
      val_b= val_b>>1;
    }
  }

  for(i=0, pa=input_a, pb=input_b; i<8; ++i, ++pa, ++pb)
  {
    if(pa->count)
    {
      ++(pa->tally);
      pa->count=0;
    }
    else
    {
      pa->tally=0;
      pa->ready= true;
    }

    if(pb->count)
    {
      ++(pb->tally);
      pb->count=0;
    }
    else
    {
      pb->tally=0;
      pb->ready= true;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
nodebug root void output()
{
  byte i, x, y;

  x=8;
  for(i=0, pa=input_a; i<8; ++i, ++pa)
    if(pa->tally>1 && pa->ready)
    {
      x=i;
      pa->ready= false;
      break;
    }
  if(x==8) return; // wtf!?

  y=8;
  for(i=0, pb=input_b; i<8; ++i, ++pb)
    if(pb->tally>1 && pb->ready)
    {
      y=i;
      pb->ready= false;
      break;
    }
  if(y==8) return; // wtf!?

#ifdef DEBUG
  printf(sym[x][y]);
#endif
  
  serDputs(sym[x][y]);
}

///////////////////////////////////////////////////////////////////////////////
root void main()
{
  byte i;

  brdInit();
  WrPortM(SPCR,  0x80); // set PA as input
  WrPortM(PBDDR, 0x42); // set PB0,2-5,7 as inputs
  WrPortM(SBCR,  0x00);
  
  setup();

  // reset inputs
  for(i=0, pa=input_a, pb=input_b; i<8; ++i, ++pa, ++pb)
  {
    pa->ready= true;
    pa->count=0;
    pa->tally=0;
    
    pb->ready= true;
    pb->count=0;
    pb->tally=0;
  }
  
  serDopen(9600);

  while(true)
  {
    read();
    output();
  }
}