#include <stdio.h>
#include <math.h>
#include <linux/soundcard.h>
#include <fcntl.h>
#include <unistd.h> //para pipe

#define FREQ_SAMPLEADO 44100

#define FORMATO AFMT_U8
#define STEREO 0

#define FREQ_PLUS 12800

#define SAMPLE FREQ_SAMPLEADO/4

unsigned char buffer[SAMPLE];

int main (void)
{

  FILE *sonido_out; //para el esd

  FILE *sonido_in;

  int descf[2];

  int n=0;
  double t=0;
  double inc;
  double valor;
  double signo;
  int c;
  int inicio=0;

  int freq=FREQ_SAMPLEADO;
  int formato=FORMATO;
  int stereo=STEREO;
  int fuente=SOUND_MASK_LINE;

  /* Inicializar escritura */
  /*if ((sonido_out=open("/dev/dsp",O_WRONLY))<0) {
    printf ("Error al abrir dispositivo");
    return -1;
  }

  if (ioctl(sonido_out,SNDCTL_DSP_SPEED,&freq) < 0) {
    printf ("Error al establecer frecuencia");
    return -1;
  }

  if (ioctl(sonido_out,SNDCTL_DSP_SETFMT,&formato) < 0) {
	 printf ("Error al establecer 8 bits");
	 return -1;
  }
  if (ioctl(sonido_out,SNDCTL_DSP_STEREO,&stereo) < 0) {
	 printf ("Error al establecer mono");
	 return -1;
  } */

  /* Reproducir con esd */
  /*if (pipe(descf)==-1) {
    printf ("Error al crear tuberia");
    return -1;
  }

  if ((sonido_out=fdopen(*/


  /* Inicializar lectura */
  if ((sonido_in=open("/dev/dsp",O_RDONLY))<0) {
    printf ("Error al abrir dispositivo");
    return -1;
  }

  if (ioctl(sonido_in,SNDCTL_DSP_SPEED,&freq) < 0) {
    printf ("Error al establecer frecuencia");
    return -1;
  }

  if (ioctl(sonido_in,SNDCTL_DSP_SETFMT,&formato) < 0) {
	 printf ("Error al establecer 8 bits");
	 return -1;
  }
  if (ioctl(sonido_in,SNDCTL_DSP_STEREO,&stereo) < 0) {
	 printf ("Error al establecer mono");
	 return -1;
  }

  if (ioctl(sonido_in,SOUND_MIXER_WRITE_RECSRC,&fuente) < 0) {
	 printf ("Error al establecer grabación desde Line In");
	 return -1;
  }

  printf ("Este programa descodifica el sonido del "
  			 "CANAL+ (Nagravision)\n");
  			
  inc=(3.1416*2)/FREQ_SAMPLEADO;
  inc*=FREQ_PLUS;
  //printf ("%d %d\n",sizeof(char),sizeof(int));

  read(sonido_in,&buffer,SAMPLE);
  //En teoria este bucle se necesita. En la práctica, no.
  for (n=0;n<SAMPLE/4;n++) {
    if (buffer[n]==128) {
      inicio=n;
      break;
    }
  }


  /* Uso: decod_plus_esd|esdcat -b -m
  /*printf ("Frecuencia señal Canal+ : %d Hz\n"
  			 "Offset inicial: %d\n"
  			 "Frecuencia de sampleado: %d Hz\n"
  			 "Buffer de lectura: %d Bytes\n",
          FREQ_PLUS,inicio,FREQ_SAMPLEADO,SAMPLE);*/


  while (1) {
      for (n=inicio;n<SAMPLE;n++) {

      c=buffer[n]-128;

      valor=sin(t);

      c*=valor;

      c+=128;
      t +=inc;

      c %=256;

      //c-=128;
      buffer[n]=c;
    }
    fwrite(&buffer,1,SAMPLE,stdout);
    inicio=0;
    read(sonido_in,&buffer,SAMPLE);
  }
  //close(sonido_out);
  close(sonido_in);
}
