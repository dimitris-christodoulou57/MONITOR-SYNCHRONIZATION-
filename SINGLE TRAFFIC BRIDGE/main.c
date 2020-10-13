//ADAMAKIS CHRISTOS 2148 
//CHRISTODOULOU DIMITRIS 2113
//OMADA 2

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <error.h>

pthread_mutex_t mtx;
pthread_cond_t  bridge_red, bridge_blue, finish;
#define MAX 2



// arxikopoiisi metablitwn pou xrisimopoioume sto sigxronismo
volatile int red_run=0, blue_run=0;
volatile int red_wait=0, blue_wait=0;
volatile int enter_blue=0, exit_blue=0;//boi8itikes metablites gia tin emfanisei ton minimaton
volatile int enter_red=0, exit_red=0;//boi8itikes metablites gia tin emfanisei ton minimaton 
volatile int enter=0;
volatile int red_pass=0, blue_pass=0;

void *blue_function(void *assignment){
	
	int check;
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	//eisagwgi neou autokinitou
	enter++;
	
	if(red_run + red_wait > 0){//ama perimenoun i trexoun red 
		blue_wait++;
		
		check=pthread_cond_wait(&bridge_blue,&mtx);//anamonh mexri na kanei kapoios signal blue
		if(check){
			printf("error signal\n");
		}
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock\n");
		}
		
		if((blue_wait > 0) && (blue_run < MAX)){//ama perimenoun blue kai den trexoun panw apo MAX eidopoioume ton epomeno
			blue_wait--;
			blue_run++;
			check=pthread_cond_signal(&bridge_blue);
			if(check){
				printf("error signal\n");
			}
		}
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
	}
	else{
		if(blue_run >= MAX){//ama trexoun panw apo MAX blue perimenoun 
			blue_wait++;
			
			check=pthread_cond_wait(&bridge_blue,&mtx);
			if(check){
				printf("error waitl\n");
			}
			
			if((blue_wait > 0) && (blue_run < MAX)){//ama perimenoun blue kai den trexoun panw apo MAX eidopoioume ton epomeno
				blue_wait--;
				blue_run++;
				check=pthread_cond_signal(&bridge_blue);
				if(check){
					printf("error signal\n");
				}
			}
			
			check=pthread_mutex_unlock(&mtx);
			if(check){
				printf("error unlock\n");
			}
 		}
 		else{//ama den trexoun i perimenoun kokkina kai den trexoun panw apo MAX blue eiserxete to blue stin gefura
			blue_run++;
			check=pthread_mutex_unlock(&mtx);
			if(check){
				printf("error unlock\n");
			}
		}
		
	}
	
	
	enter_blue++;
	printf("+ + + enter bridge blue %d\n", enter_blue);
	sleep(2);
	exit_blue++;
	printf("- - - exit bridge blue %d\n",exit_blue);
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	blue_pass++;
	blue_run--;
	//ama perimenoun kokkina kai exoun perasei ola ta blue pou etrexan kai o arithmos tous einai mikroteros apo 2*MAX paraxoroume proteraiotita sta kokkina
	if((red_wait>0) && (blue_run==0) && (blue_pass>=MAX)){
		red_wait--;
		red_run++;
		check=pthread_cond_signal(&bridge_red);
		if(check){
			printf("error signal\n");
		}
		blue_pass=0;
	}
	else if((blue_run< MAX ) && (blue_wait > 0)&& (blue_pass<MAX)){//ama den trexoun max aytokinita blue kai perimenei kapoio blue kai den exoun perasei idi 2*max blue ksipnaei o epomenos 
		blue_wait--;
		blue_run++;
		check=pthread_cond_signal(&bridge_blue);
		if(check){
			printf("error signal\n");
		}
	}
	else if((blue_pass>=MAX) && (red_wait==0)){//gia tin periptwsi pou den perimenei kapoio red
		blue_wait--;
		blue_run++;
		check=pthread_cond_signal(&bridge_blue);
		if(check){
			printf("error signal\n");
		}
	}
	else if((blue_pass<MAX) && (red_wait>0)){//gia tin periptwsi pou perimenei kapoio red alla den uparxoun epipleon blue na trexoun
		red_wait--;
		red_run++;
		check=pthread_cond_signal(&bridge_red);
		if(check){
			printf("error signal\n");
		}
		blue_pass=0;
	}
	
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	
	if(enter == exit_blue + exit_red){//ama o arithmos autwn pou vgikan einai idios me auton pou vgikan eidopoioume tin main
		check=pthread_cond_signal(&finish);
		if(check){
			printf("error signal\n");
		}
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	
	return NULL;
}


void *red_function(void *assignment){
	
	int check;
	
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	
	enter++;
	
	if(blue_run + blue_wait > 0){//ama perimenoun i trexoun blue
		red_wait++;
		
		check=pthread_cond_wait(&bridge_red,&mtx);//anamonh mexri na kanei kapoios signal red
		if(check){
			printf("error signal\n");
		}
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock\n");
		}
		
		if((red_wait > 0) && (red_run < MAX)){//ama perimenoun red kai den trexoun panw apo MAX eidopoioume ton epomeno
			red_wait--;
			red_run++;
			check=pthread_cond_signal(&bridge_red);
			if(check){
				printf("error signal\n");
			}
		}
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
	}
	else{
		if(red_run >= MAX){//ama trexoun panw apo MAX blue perimenoun
			red_wait++;
			
			check=pthread_cond_wait(&bridge_red,&mtx);
			if(check){
				printf("error waitl\n");
			}
			if((red_wait > 0) && (red_run < MAX)){//ama perimenoun red kai den trexoun panw apo MAX eidopoioume ton epomeno
				red_wait--;
				red_run++;
				check=pthread_cond_signal(&bridge_red);
				if(check){
					printf("error signal\n");
				}
			}
			
			check=pthread_mutex_unlock(&mtx);
			if(check){
				printf("error unlock\n");
			}
 		}
 		else{//ama den trexoun i perimenoun blue kai den trexoun panw apo MAX red eiserxete to red stin gefura
			red_run++;
			check=pthread_mutex_unlock(&mtx);
			if(check){
				printf("error unlock\n");
			}
		}
		
	}
	
	enter_red++;
	printf("-> -> -> enter bridge red %d\n", enter_red);
	sleep(2);
	exit_red++;
	printf("<- <- <- exit bridge red %d\n",exit_red);
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	red_pass++;
	red_run--;
	//ama perimenoun blue kai exoun perasei ola ta red pou etrexan kai o arithmos tous einai mikroteros apo 2*MAX paraxoroume proteraiotita sta blue
	if((blue_wait>0) && (red_run==0) && (red_pass>= MAX)){
		blue_wait--;
		blue_run++;
		check=pthread_cond_signal(&bridge_blue);
		if(check){
			printf("error signal\n");
		}
		red_pass=0;
	}
	else if((red_run< MAX ) && (red_wait > 0)&&(red_pass < MAX)){//ama den trexoun max aytokinita red kai perimenei kapoio red kai den exoun perasei idi 2*max red ksipnaei o epomenos
		red_wait--;
		red_run++;
		check=pthread_cond_signal(&bridge_red);
		if(check){
			printf("error signal\n");
		}
	}
	else if((red_pass>=MAX) && (blue_wait==0)){//gia tin periptwsi pou den perimenei kapoio blue
		red_wait--;
		red_run++;
		check=pthread_cond_signal(&bridge_red);
		if(check){
			printf("error signal\n");
		}
	}
	else if((red_pass<MAX) && (blue_wait>0)){//gia tin periptwsi pou perimenei kapoio blue alla den uparxoun epipleon red na trexoun
		blue_wait--;
		blue_run++;
		check=pthread_cond_signal(&bridge_blue);
		if(check){
			printf("error signal\n");
		}
		red_pass=0;
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	
	if(enter == exit_blue + exit_red){//ama o arithmos autwn pou vgikan einai idios me auton pou vgikan eidopoioume tin main
		check=pthread_cond_signal(&finish);
		if(check){
			printf("error signal\n");
		}
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	
	return NULL;
}



int main(int argc, char *argv[]) {
	
	int res, check;
	int file_read;
	pthread_t thread_red,thread_blue;
	char c;

	
	if(argc!=2){
		printf("error argument\n");
		return -1;
	}
	
	
	file_read=open(argv[1], O_RDONLY, 0);
	if(file_read==-1){
		printf("file not found.\n");
		return -1;
	}
	
	check=pthread_mutex_init(&mtx,NULL);
	if (check){
		printf("error init mtx \n");
	}
	
// 	 initilazation twn mutexes kai ton conditions

	check=pthread_cond_init(&bridge_red,NULL);
	if (check){
		printf("error init bridge_red \n");
	}
	
	check=pthread_cond_init(&bridge_blue,NULL);
	if (check){
		printf("error init bridge blue \n");
	}
	
	check=pthread_cond_init(&finish,NULL);
	if (check){
		printf("error init finish \n");
	}
	
	while(read(file_read, &c, sizeof(char)) != 0){
		
		if(c=='b'){
			res = pthread_create(&thread_blue, NULL, blue_function, NULL);
			if (res){
				printf("error pthread controler \n");
			}
		}
		if(c=='r'){
			res = pthread_create(&thread_red, NULL, red_function, NULL);
			if (res){
				printf("error pthread controler \n");
			}
		}
		
	}
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	//anamonh na perasoun ola ta amaxia tin gefyra
	check=pthread_cond_wait(&finish,&mtx);
	if(check){
		printf("error signal\n");
	}

	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
		
	
	
	return 0;
	
}
