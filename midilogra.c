#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int totalbytes = 0;

void decode (unsigned time, unsigned event, FILE *f)
{
  static unsigned lasttime = 0;
  static int laststatus = 0;
  

  
  int status = event & 0xff;

  int d1, d2;
  
  if (status & 0x80)
  {
    laststatus = status;
    d1 = event >> 8 & 0xff;
	d2 = event >> 16 & 0xff;
  }
  else
  {
    d1 = status;
	status = laststatus;
	d2 = event >> 8 & 0xff;
  }

  unsigned delta = time - lasttime;
  lasttime = time;
  
  // write varlen
  char writer[4] = {0, 0, 0, 0};
  int pos = 0;
  while (delta)
  {
    writer[pos] = delta & 127;
	if (pos)
	  writer[pos] |= 0x80;
	pos++;
	delta /= 128;
  }
  if (!pos)
  {
    writer[0] = 0;
    fwrite (writer, 1, 1, f);
    totalbytes++;
  }
  else
  {
    while (pos > 0)
    {
      fwrite (writer + pos - 1, 1, 1, f);
	  totalbytes++;
	  pos--;
    }
  }
  
  // aren't actually writing out running status
  switch (status & 0xf0)
  {
    case 0x80: // off
	case 0x90: // on
	case 0xa0: // aftertouch
	case 0xb0: // controller
	case 0xe0: // bend
	  fwrite (&status, 1, 1, f);
	  fwrite (&d1, 1, 1, f);
	  fwrite (&d2, 1, 1, f);
	  totalbytes += 3;
	  break;
	
	case 0xc0: // program
	case 0xd0: // channel aftertouch
      fwrite (&status, 1, 1, f);
	  fwrite (&d1, 1, 1, f);
	  totalbytes += 2;
	  break;
    default:
	  fprintf (stderr, "OH MY GOD\n");
	  exit (0);
  }
  
  return;
}


int main (void)
{
  FILE *f = fopen ("midilogra.mid", "wb");
  
  char buff[256];
  
  
  char bigheader[] = {'M', 'T', 'h', 'd',
    0, 0, 0, 6,
	0, 0,
	0, 1,
	0, 1}; // midi clocks per quarternote
  fwrite (bigheader, 1, 14, f);
  
  char chunkheader[] = {'M', 'T', 'r', 'k', 0xde, 0xad, 0xbe, 0xef};
  fwrite (chunkheader, 1, 8, f);
  
  // tempo event (microseconds per quarternote
  char openevent[] = {0x00, 0xff, 0x51, 0x03, 0x00, 0x03, 0xe8};
  fwrite (openevent, 1, 7, f);
  totalbytes += 6;
  
  while (fgets (buff, 256, stdin))
  {
    unsigned time;
	unsigned ev;
	
	if (sscanf (buff, "%u,%x", &time, &ev) != 2)
	{
      fprintf (stderr, "NO WAY MAN\n");
	  exit (0);
	}
	decode (time, ev, f);
  
  }
  
  // end of track event
  // lets give ~10 seconds
  // 10000ms = 78, 16  (ce 10)
  char endevent[] = {0xce, 0x10, 0xff, 0x2f, 0x00};
  fwrite (endevent, 1, 5, f);
  totalbytes += 5;
  
  
  fseek (f, 18, SEEK_SET);
  char scratch;
  scratch = totalbytes >> 24;
  fwrite (&scratch, 1, 1, f);
  scratch = totalbytes >> 16;
  fwrite (&scratch, 1, 1, f);
  scratch = totalbytes >> 8;
  fwrite (&scratch, 1, 1, f);
  scratch = totalbytes;
  fwrite (&scratch, 1, 1, f);

  fclose (f);  
  

  return 0;
}

