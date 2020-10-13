//ADAMAKIS CHRISTOS 2148 
//CHRISTODOULOU DIMITRIS 2113
//OMADA 2
// dhmiourgei n thread (dinetai san parametros apo to pliktrologio) pou ilopoioun 
// tin mandel_Calc gia ka8e tmhma kai apo8ikeuoun to apotelesma 
// gia na to sxediasei i main 
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mandelCore.h"

#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;

//orismos mtx prostasias kai conditions gia sygxronismo
pthread_mutex_t mtx;
pthread_cond_t cond_data, cond_paint;

volatile int worker_paint=0;
volatile int *value;

// orismos struct gia na pername sta thread tis parametrous tis mandel_Calc
struct assignments{
	mandel_Pars *slices;
	int *res;
	int maxIterations;
	int num_of_worker;
};


/* basic win management rountines */

static void openDisplay() {
  if (dsp == NULL) { 
    dsp = XOpenDisplay(NULL); 
  } 
}

static void closeDisplay() {
  if (dsp != NULL) { 
    XCloseDisplay(dsp); 
    dsp=NULL;
  }
}

void openWin(const char *title, int width, int height) {
  unsigned long blackC,whiteC;
  XSizeHints sh;
  XEvent evt;
  long evtmsk;

  whiteC = WhitePixel(dsp, DefaultScreen(dsp));
  blackC = BlackPixel(dsp, DefaultScreen(dsp));
  curC = blackC;
 
  win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);

  sh.flags=PSize|PMinSize|PMaxSize;
  sh.width=sh.min_width=sh.max_width=WinW;
  sh.height=sh.min_height=sh.max_height=WinH;
  XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);

  XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
  XMapWindow(dsp, win);
  do {
    XWindowEvent(dsp, win, StructureNotifyMask, &evt);
  } while (evt.type != MapNotify);

  gc = XCreateGC(dsp, win, 0, NULL);

}

void closeWin() {
  XFreeGC(dsp, gc);
  XUnmapWindow(dsp, win);
  XDestroyWindow(dsp, win);
}

void flushDrawOps() {
  XFlush(dsp);
}

void clearWin() {
  XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
  XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
  flushDrawOps();
  XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
  XDrawPoint(dsp, win, gc, x, WinH-y);
  flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
  XEvent evt;

  XSelectInput(dsp, win, ButtonPressMask);
  do {
    XNextEvent(dsp, &evt);
  } while (evt.type != ButtonPress);
  *x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
  XColor clr1,clr2;

  if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
    printf("failed\n"); return;
  }
  XSetForeground(dsp, gc, clr1.pixel);
  curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
  static char cname[128];

  if (v == maxIterations) {
    return("black");
  }
  else {
    sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
    return(cname);
  }
}

// sinartisi worker pou kalei to ka8e thread
void *worker(void * arg){
	
	struct assignments *args;
	int num_of_worker, check;
	
	args = (struct assignments *) arg;
	num_of_worker=args->num_of_worker;
	
	while(1){
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock %d\n", num_of_worker);
		}
		// elegxoume an exoun anatethei dedomena sto worker gia na upologisei tin mandel_Calc
		if(value[num_of_worker]!=1){
			check=pthread_cond_wait(&cond_data,&mtx);//an den exoun perimenoume mexri na anatethoum
			if(check){
				printf("error wait %d\n", num_of_worker);
			}
		}
		
		value[num_of_worker]=3;
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock %d\n", num_of_worker);
		}
		
		printf("starting slice nr. %d\n",num_of_worker+1);
		mandel_Calc(&args->slices[num_of_worker],args->maxIterations,&args->res[num_of_worker*args->slices[num_of_worker].imSteps*args->slices[num_of_worker].reSteps]);
		printf("done\n");
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock %d\n", num_of_worker);
		}
		
		//allazoume to value se 2 wste na kserei i main pws mporei na ektipwsei to sygkekrimeno slice kai tin eidopioume
		value[num_of_worker]=2;
		
		check=pthread_cond_signal(&cond_paint);
		if(check){
			printf("error signal %d\n", num_of_worker);
		}
		
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock %d\n", num_of_worker);
		}
 		
	}
}

int main(int argc, char *argv[]) {
	mandel_Pars pars,*slices;
	struct assignments *assignment;
	pthread_t *pthread;
	int i,j,x,y,nofslices,maxIterations,level,*res, check;
	int xoff,yoff;
	long double reEnd,imEnd,reCenter,imCenter;

  printf("\n");
  printf("This program starts by drawing the default Mandelbrot region\n");
  printf("When done, you can click with the mouse on an area of interest\n");
  printf("and the program will automatically zoom around this point\n");
  printf("\n");
  printf("Press enter to continue\n");
  getchar();

  pars.reSteps = WinW; /* never changes */
  pars.imSteps = WinH; /* never changes */
 
  /* default mandelbrot region */

  pars.reBeg = (long double) -2.0;
  reEnd = (long double) 1.0;
  pars.imBeg = (long double) -1.5;
  imEnd = (long double) 1.5;
  pars.reInc = (reEnd - pars.reBeg) / pars.reSteps;
  pars.imInc = (imEnd - pars.imBeg) / pars.imSteps;

  printf("enter max iterations (50): ");
  scanf("%d",&maxIterations);
  printf("enter no of slices: ");
  scanf("%d",&nofslices);
  
  
  /* adjust slices to divide win height */

  while (WinH % nofslices != 0) { nofslices++;}

  /* allocate slice parameter and result arrays */
  
	slices = (mandel_Pars *) malloc(sizeof(mandel_Pars)*nofslices);
	res = (int *) malloc(sizeof(int)*pars.reSteps*pars.imSteps);
	pthread= (pthread_t *) malloc(sizeof(pthread_t)*nofslices);
	assignment = (struct assignments*)malloc(sizeof(struct assignments)*nofslices);
	value=(int *) malloc(sizeof(int)*nofslices);
	
	//iniitialization twn mtx kai twn conditions
	check=pthread_mutex_init(&mtx, NULL);
	if(check){
		printf("error mutex init\n");
	}
	
	check=pthread_cond_init(&cond_data, NULL);
	if(check){
		printf("error cond data init\n");
	}
	
	check=pthread_cond_init(&cond_paint, NULL);
	if(check){
		printf("error cond paint init\n");
	}
	
	for(i=0; i<nofslices; i++){//arxikopoihsh twn value se 0
		value[i]=0;
	}
	
	for(i=0; i<nofslices; i++){//dimiourgeia nofslices worker
		assignment[i].num_of_worker = i;
		check = pthread_create(&pthread[i],NULL,worker, &assignment[i]);
		if (check){
			printf("error pthread %d\n", i);
		}
	}
 
  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);

  level = 1;

  while (1) {
	  
	  

    clearWin();

    mandel_Slice(&pars,nofslices,slices);
    
    y=0;
// 	ana8esi se ka8e epanalipsi timwn stis parametrous tou i thread
    for (i=0; i<nofslices; i++) {
		//i = counter_worker
		assignment[i].slices= slices;
		assignment[i].maxIterations = maxIterations;
		assignment[i].res = res;
	}
// 	afou ana8esoume se ola ta thread times tote allazoume to value se 1 gia na ksekinisoun oi worker

	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock %d\n", i);
	}
	
	for (i=0; i<nofslices; i++){
//	 	afou ana8esoume se ola ta thread times tote allazoume to value se 1 gia na ksekinisoun oi worker
		value[i]=1;
// 		eidopoioume tous worker
		check=pthread_cond_signal(&cond_data);
		if(check){
			printf("error signal %d\n", i);
		}
		
		
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock %d\n", i);
	}
	
	for (i=0; i<nofslices; i++){
 		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock %d\n", i);
		}
// 		perimenoume twn i worker na upologisei ta apotelesmata kai eidopoihsei thn main
		while(value[i]!=2){
			check=pthread_cond_wait(&cond_paint,&mtx);
			if(check){
				printf("error wait %d\n", i);
			}
		}
		
		
		for (j=0; j<slices[i].imSteps; j++) {
			for (x=0; x<slices[i].reSteps; x++) {
				setColor(pickColor(res[y*slices[i].reSteps+x],maxIterations));
				drawPoint(x,y);
			}
			y++;
		}
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock %d\n", i);
		}
		
	}
	
	
    /* get next focus/zoom point */
    
    getMouseCoords(&x,&y);
    xoff = x;
    yoff = WinH-y;
    
    /* adjust region and zoom factor  */
    
    reCenter = pars.reBeg + xoff*pars.reInc;
    imCenter = pars.imBeg + yoff*pars.imInc;
    pars.reInc = pars.reInc*ZoomStepFactor;
    pars.imInc = pars.imInc*ZoomStepFactor;
    pars.reBeg = reCenter - (WinW/2)*pars.reInc;
    pars.imBeg = imCenter - (WinH/2)*pars.imInc;
    
    maxIterations = maxIterations*ZoomIterationFactor;
    level++;

  } 
  
  /* never reach this point; for cosmetic reasons */

  
  free(slices);
  free(res);
  free(assignment);
  free(pthread);

  closeWin();
  closeDisplay();

}
