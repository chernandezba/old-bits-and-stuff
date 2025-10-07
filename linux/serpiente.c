#include <curses.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

//compile with gcc serpiente.c -lncurses -o serpiente

#define ANCHO 30
#define ALTO 17
#define MAX_LONGITUD 1000

#define TIEMPO_COMIDA_EXTRA 30 //Tiempo que esta activa una comida extra
#define PUNTOS_COMIDA_EXTRA 10 //Puntos que se cogen (multiplicado por el nivel)
#define MAX_VECES_COMIDA_EXTRA 5
//la comida extra sale cada 5 comidas normales

//#define DEBUG

//#define LIRC //Habilitar soporte para control remoto

#ifdef LIRC
	#include <lirc/lirc_client.h>
	#include <fcntl.h>
#endif

enum direcciones {IZQUIERDA=1,DERECHA,ARRIBA,ABAJO};

#define Y_INICIAL 3

int puntos;

int puntos_maximo;

int cabeza_x,cabeza_y;
int longitud;
int choque;
int siguiente_comida;
int comida_x,comida_y; //Comida "normal"
int alargar;

int direccion;
int nivel;

int comida_extra_x,comida_extra_y; //comida "extra"
//no aumenta serpiente, solo puntos
int comida_extra_tiempo=0; //tiempo que lleva activo la comida
//0 si no hay ninguna activa
int veces_comida_extra;

#ifdef LIRC
	int lirc_activo; //indica si el soporte para lirc funciona (contiene -1 si no funciona)
	struct lirc_config *config_remoto;
	char *codigo_lirc;
	char *cadena_lirc;

	struct lirc_teclas {
		char *cadena_lirc;
		int tecla;
	};

	#define TECLAS_LIRC 12

	struct lirc_teclas tabla_teclas[TECLAS_LIRC]={
	{"UP",KEY_UP},
	{"DOWN",KEY_DOWN},
	{"LEFT",KEY_LEFT},
	{"RIGHT",KEY_RIGHT},
	{"END",'f'},
	{"ENTER",13},
	{"1",'1'},
	{"2",'2'},
	{"3",'3'},
	{"4",'4'},
	{"5",'5'},
	{"6",'6'}
	};
#endif

WINDOW *w_juego;

struct pos {
  int x;
  int y;
};

struct pos posiciones[MAX_LONGITUD];

int leer_tecla_lirc(void);

int espera_tecla(void)
{
	int tecla;
	do {


		tecla=leer_tecla_lirc();

		usleep(1e5);
	} while (tecla==ERR);
	return tecla;
}

void acabar_error(char *s) {
	endwin();
  printf ("%s: %s\n",s,strerror(errno));
	exit(-1);
}

void escribe_puntos(void)
{
	move(2,0);
	clrtoeol();
	printw ("Puntos:%d   Maximo:%d",puntos,puntos_maximo);
}

void escribe_tiempo_comida_extra(void)
{
	move(2,30);
	clrtoeol();
	printw ("%d",comida_extra_tiempo);
	refresh();

}

void pinta_pantalla (void) {
  int i;

  move (0,0);
  printw ("Serpiente v.3.\n"
					"Creado por Cesar Hernandez (02/04/2002)");
  move (Y_INICIAL,0);
  for (i=0;i<ANCHO+2;i++) printw ("X");

  for (i=0;i<ALTO+2;i++) {
    move (i+Y_INICIAL,0);
    printw("X");
    move (i+Y_INICIAL,ANCHO+1);
    printw ("X");
  }

  move (Y_INICIAL+ALTO+1,0);
  for (i=0;i<ANCHO+2;i++) printw ("X");


}

//Busca una posicion libre para la comida
void busca_posicion_comida(int *x,int *y)
{

  int i,repetir;

	do {
    *x=rand() % ANCHO;
    *y=rand() % ALTO;

    repetir=0;
    for (i=0;i<longitud;i++) {
      if (*x==posiciones[i].x && *y==posiciones[i].y) {
        repetir=1;
        break;
      }
    }

  } while (repetir);


}

//Activa la comida normal
void activa_comida(void)
{

	do {
  	busca_posicion_comida(&comida_x,&comida_y);
	} while (comida_extra_tiempo &&
					comida_x==comida_extra_x && comida_y==comida_extra_y);

}

//Activa la comida extra
void activa_comida_extra(void)
{
	do {
	  busca_posicion_comida(&comida_extra_x,&comida_extra_y);
	} while (comida_x==comida_extra_x && comida_y==comida_extra_y);



}


void pinta_comida(void)
{
  wmove (w_juego,comida_y,comida_x);
  wprintw(w_juego,"o");

}

void pinta_comida_extra(void)
{
  wmove (w_juego,comida_extra_y,comida_extra_x);
  wprintw(w_juego,"*");

}

void borra_comida_extra(void)
{
  wmove (w_juego,comida_extra_y,comida_extra_x);
  wprintw(w_juego," ");

}

void borra_tiempo_comida_extra(void)
{
	move(2,30);
	clrtoeol();

}



void pinta_serpiente(void)
{
  int i;

  for (i=1;i<longitud;i++) {
    wmove (w_juego,posiciones[i].y,posiciones[i].x);
    wprintw (w_juego,"#");
  }
	wmove (w_juego,cabeza_y,cabeza_x);
	wprintw(w_juego,"@");
}

void mover_serpiente(void)
{
  int i;

  if (!alargar) {
    //borrar cola
    wmove (w_juego,posiciones[longitud-1].y,posiciones[longitud-1].x);
    wprintw(w_juego," ");
  }
  if (alargar) {
    posiciones[longitud].x=posiciones[longitud-1].x;
    posiciones[longitud].y=posiciones[longitud-1].y;
  }
  for (i=longitud-1;i>0;i--) {
    posiciones[i].y=posiciones[i-1].y;
    posiciones[i].x=posiciones[i-1].x;
  }
  if (alargar) {
    longitud++;
    alargar--;
  }

  //asignar cabeza
  if (direccion==IZQUIERDA) {
    cabeza_x--;
  }

  else if (direccion==DERECHA) {
    cabeza_x++;
  }

  else if (direccion==ABAJO) {
    cabeza_y++;
  }

  else if (direccion==ARRIBA) {
    cabeza_y--;
  }


  posiciones[0].x=cabeza_x;
  posiciones[0].y=cabeza_y;

}

void mirar_comida(void)
{

  if (cabeza_x==comida_x && cabeza_y==comida_y)
  {
    alargar +=(nivel+1)/2;
    siguiente_comida=1;
		puntos +=nivel*2;
		if (veces_comida_extra) veces_comida_extra--;

		escribe_puntos();
  }
}

void mirar_comida_extra(void)
{

	if (comida_extra_tiempo!=0) {
		if (cabeza_x==comida_extra_x && cabeza_y==comida_extra_y)
  	{
			puntos +=PUNTOS_COMIDA_EXTRA*nivel;
			comida_extra_tiempo=0;

			escribe_puntos();
			refresh();
  	}
	}
}


void mirar_choque_lateral(void)
{
  if (cabeza_x<0 || cabeza_y<0 ||
			cabeza_x==ANCHO || cabeza_y==ALTO) choque=1;
}

void mirar_choque_serpiente(void)
{
  int i;

	for (i=1;i<longitud;i++)
	  if (cabeza_x==posiciones[i].x && cabeza_y==posiciones[i].y) choque=1;

}


#ifdef LIRC
void convertir_codigo_tecla(char *cadena_lirc,int *tecla)
{

	int i;

	for (i=0;i<TECLAS_LIRC;i++) {
		if (strcmp(tabla_teclas[i].cadena_lirc,cadena_lirc)==0) {
			*tecla=tabla_teclas[i].tecla;
			return;
		}
	}

	*tecla=-1;

}
#endif


int leer_tecla_lirc0(void)
{


	int tecla=-1;

#ifdef LIRC


	if (lirc_activo!=-1) {
		//leer control remoto
		if (lirc_nextcode(&codigo_lirc)==0) {
			if (codigo_lirc!=NULL) {
				if (lirc_code2char(config_remoto,codigo_lirc,&cadena_lirc)==0) {
					if (cadena_lirc!=NULL) {
					//Convertir cadena a codigo de tecla
						convertir_codigo_tecla(cadena_lirc,&tecla);
					}
				}

				free(codigo_lirc);
			}

		}

	}

#endif

	return tecla;

}

int leer_tecla_lirc(void)
{

	int tecla;

	tecla=leer_tecla_lirc0();
	if (tecla==-1) tecla=wgetch(w_juego);

	return tecla;
}


int main (void)
 {

	int tecla,final;
	char *directorio_home;
	char nombre_fichero[256];
	char buffer[256];

	FILE *fd_fichero_configuracion;

#ifdef LIRC
	lirc_activo=lirc_init("serpiente",1);

	if (lirc_activo!=-1) {
		if(lirc_readconfig(NULL,&config_remoto,NULL)!=0) {
			lirc_activo=-1;
		}
		else {
			//Poner socket lirc a no bloqueante
			if (fcntl(lirc_activo,F_SETFL,O_NONBLOCK)==-1) lirc_activo=-1;
		}
	}

#endif



	initscr();
  cbreak();
  noecho();


  nodelay(stdscr,TRUE);
  curs_set(0);//desactivar cursor

  choque=0;
	final=0;

  pinta_pantalla();

	w_juego=newwin(ALTO,ANCHO,Y_INICIAL+1,1);

	keypad(w_juego,TRUE);
	nodelay(w_juego,TRUE);

  if ( (directorio_home=getenv("HOME"))==NULL)
    acabar_error("Directorio HOME no definido!");

  sprintf(nombre_fichero,"%s/.serpiente",directorio_home);
  if ( (fd_fichero_configuracion=fopen(nombre_fichero,"r"))!=NULL) {
 	  if (!feof(fd_fichero_configuracion)) {
		  fgets(buffer,250,fd_fichero_configuracion);
		  puntos_maximo=atoi(buffer);
    }
  }
	else puntos_maximo=0;

	fclose(fd_fichero_configuracion);

	move (4,4);
	printw ("Elije nivel: (1-6)");
	nivel=0;

	do {
		tecla=leer_tecla_lirc0();
		if (tecla==-1) tecla=getch();

	  if (tecla!=ERR && tecla>='1' && tecla<='6') nivel=tecla-'0';

		//crear semilla aleatoria
		rand();
		usleep(1e5);



  } while (!nivel);

  do {
	  choque=0;
		comida_extra_tiempo=0;
		veces_comida_extra=MAX_VECES_COMIDA_EXTRA;

		puntos=0;
		escribe_puntos();
    //inicializar cabeza
    cabeza_x=ANCHO/2;
    cabeza_y=ALTO/2;

    posiciones[0].x=cabeza_x;
    posiciones[0].y=cabeza_y;
    posiciones[1].x=cabeza_x+1;
    posiciones[1].y=cabeza_y;
    posiciones[2].x=cabeza_x+2;
    posiciones[2].y=cabeza_y;

    longitud=3;
    direccion=IZQUIERDA;

    siguiente_comida=1;
    alargar=0;

		werase(w_juego);

    do {

			pinta_serpiente();
      if (siguiente_comida==1) {
        siguiente_comida=0;
        activa_comida();
        pinta_comida();

				refresh();
      }

			//Mirar si se activa la comida extra
			if (comida_extra_tiempo==0) {

				if (veces_comida_extra==0) {
					veces_comida_extra=MAX_VECES_COMIDA_EXTRA;
					//Buscar posicion para comida extra
					activa_comida_extra();
					comida_extra_tiempo=TIEMPO_COMIDA_EXTRA;
					pinta_comida_extra();
					refresh();
				}
			}

			if (comida_extra_tiempo) {
				comida_extra_tiempo--;
				if (comida_extra_tiempo==0) {
					borra_comida_extra();
					borra_tiempo_comida_extra();
				}
				else escribe_tiempo_comida_extra();
				refresh();
			}

#ifdef DEBUG
	    wmove(w_juego,0,0);
	    wprintw (w_juego,"c:%d,%d p:%d,%d l:%d  ",comida_x,comida_y,cabeza_x,cabeza_y,longitud);
#endif

			wrefresh(w_juego);

			//mirar si coge comida
      mirar_comida();

			//mirar si coge comida extra
      mirar_comida_extra();

			usleep(1e6/(nivel*2.5));


			tecla=leer_tecla_lirc();

      if (tecla!=ERR) {
        switch (tecla) {
          case KEY_LEFT:
            direccion=IZQUIERDA;
            break;
          case KEY_RIGHT:
            direccion=DERECHA;
            break;
          case KEY_UP:
            direccion=ARRIBA;
            break;
          case KEY_DOWN:
            direccion=ABAJO;
            break;
          case 'f':
			    //case 27:
					  final=1;
						break;

        }
      }

			if (!final) {
			  mover_serpiente();
   		  //mirar choque lateral
			  mirar_choque_lateral();

			  if (!choque) {
			    //mirar choque con serpiente
			    mirar_choque_serpiente();
			  }
				/*if (choque) {
					escribe_puntos();
					borra_comida_extra();
					refresh();
					comida_extra_tiempo=0;
				}*/
			}
    } while (!choque && !final);
		if (puntos>puntos_maximo) puntos_maximo=puntos;
		wmove(w_juego,3,3);
		wclrtoeol(w_juego);
		wprintw(w_juego,"Final. Puntuacion: %d",puntos);
		wrefresh(w_juego);
		sleep(1);
		espera_tecla();
  } while (!final);


	if ( (fd_fichero_configuracion=fopen(nombre_fichero,"w"))==NULL) {
		acabar_error("Error al abrir configuracion");
  }

	sprintf(buffer,"%d",puntos_maximo);
	fputs(buffer,fd_fichero_configuracion);
	fclose(fd_fichero_configuracion);


  endwin();

#ifdef LIRC
	if (lirc_activo!=-1) {
		lirc_freeconfig(config_remoto);
		lirc_deinit();
	}
#endif

  return 0;

}
