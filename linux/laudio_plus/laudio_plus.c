#include <stdio.h>
#include <math.h>
#include <linux/soundcard.h>
#include <fcntl.h>


//
#include <linux/ioctl.h>
//

#define FREQ_SAMPLEADO 44100

#define FORMATO AFMT_S16_LE
#define STEREO 0

#define FREQ_PLUS 12800

#define SAMPLE FREQ_SAMPLEADO/16

unsigned short buffer[SAMPLE];
char *dispositivo="/dev/dsp";

int main (int argc,char *argv[])
{

  FILE *sonido_out;
  FILE *sonido_in;

  int n=0;
  double t=0;
  double inc;
  double valor;

  short c;

  int freq=FREQ_SAMPLEADO;
  int formato=FORMATO;
  int stereo=STEREO;
  int fuente=SOUND_MASK_LINE;

  if (argc==2) {
    dispositivo=argv[1];
  }

  /* Inicializar escritura */

  //
/*#define prueba SNDCTL_FM_LOAD_INSTR
  printf ("SNDCTL_FM_LOAD_INSTR dir %u type %u nr %u size %u\n",
				_IOC_DIR(prueba),
				_IOC_TYPE(prueba),
				_IOC_NR(prueba),
				_IOC_SIZE(prueba));
//*/

  if ((sonido_out=open(dispositivo,O_WRONLY))<0) {
    printf ("Error al abrir dispositivo %s",dispositivo);
    return -1;
  }

  if (ioctl(sonido_out,SNDCTL_DSP_SPEED,&freq) < 0) {
    printf ("Error al establecer frecuencia");
    return -1;
  }

  if (ioctl(sonido_out,SNDCTL_DSP_SETFMT,&formato) < 0) {
	 printf ("Error al establecer 16 bits");
	 return -1;
  }
  if (ioctl(sonido_out,SNDCTL_DSP_STEREO,&stereo) < 0) {
	 printf ("Error al establecer mono");
	 return -1;
  }

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
	 printf ("Error al establecer 16 bits");
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

  printf ("LAUDIO+ V1.1\n"
          "(C) César Hernández Bañó (23/8/2001)\n\n"
          "Este programa descodifica el sonido del "
          "CANAL+ (Nagravision)\n\n"
  	 );

  inc=(3.1416*2)/FREQ_SAMPLEADO;
  inc*=FREQ_PLUS;

  read(sonido_in,&buffer,SAMPLE*2);

  printf ("Frecuencia señal Canal+ : %d Hz\n"
  	 "Frecuencia de sampleado: %d Hz\n"
  	 "Buffer de lectura: %d Bytes = %f sec\n",
          FREQ_PLUS,FREQ_SAMPLEADO,SAMPLE*2,
          (float) SAMPLE/FREQ_SAMPLEADO
          );


  while (1) {
      for (n=0;n<SAMPLE;n++) {

      c=buffer[n];

      valor=sin(t);

      c*=valor;

      t +=inc;

      buffer[n]=c;


    }
    write(sonido_out,&buffer,SAMPLE*2);
    read(sonido_in,&buffer,SAMPLE*2);
  }
  close(sonido_out);
  close(sonido_in);
}
