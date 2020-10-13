//ADAMAKIS CHRISTOS 2148 
//CHRISTODOULOU DIMITRIS 2113
//OMADA 2

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#define N 5


// ka8olikes metablites gia na kseroume posoi epibates mpikan kai posoi bgikan
volatile int customer_wait=0, customer_wake_up=0, customer_in=0, all_customer=0, customer_exit=0, customer_enter=0;
volatile int train_situation=0;
pthread_mutex_t mtx;
pthread_cond_t train, next_customer, customer_train, finish;


// sinartisi train elegxei pote 8a ksekinisei to treno
void *thread_train(void *assignment){
	
	int check;
	
	while(1){
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock\n");
		}
	
		if(customer_in<N){//ama den perimenoun idi N epibates to treno perimenei
			check=pthread_cond_wait(&train,&mtx);
			if(check){
				printf("error signal\n");
			}
		}
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
		
		printf("-> -> ->\n");
		sleep(5);
		printf("[FINISH]\n");
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock\n");
		}
	
		check=pthread_cond_signal(&customer_train);//eidopoihsh twn customers pws to treno eftase sto telos
		if(check){
			printf("error signal\n");
		}
		
		if(customer_in!=0){//anamonh gia tin apobibash
			check=pthread_cond_wait(&train,&mtx);
			if(check){
				printf("error signal\n");
			}
		}
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
		
		printf("<- <- <-\n");
		sleep(5);
		printf("[START]\n");
		
		check=pthread_mutex_lock(&mtx);
		if(check){
			printf("error lock\n");
		}
	
		if(customer_wait!=0){//ama perimenoun stin afetiria customers tous ksupnaei
			check=pthread_cond_signal(&next_customer);
			if(check){
				printf("error signal\n");
			}
		}
		
		if(customer_exit==all_customer){//ama perasoun oloi oi customers apo to train eidopoioume tin main gia na termatisei
			check=pthread_cond_signal(&finish);
			if(check){
				printf("error signal\n");
			}
		}
		
		customer_wake_up=0;
		
		check=pthread_mutex_unlock(&mtx);
		if(check){
			printf("error unlock\n");
		}
		
	}
	
	
	return(NULL);
	
	
}

void *thread_customer(void *assignment){
	
	int check;
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	
	if(train_situation!=0){//ama to treno den einai stin afetiria perimenei
		
		customer_wait++;
		
		check=pthread_cond_wait(&next_customer,&mtx);
		if(check){
			printf("error signal\n");
		}
		customer_wait--;
		customer_wake_up++;
		
		if(customer_wake_up < N){//ama den exei gemisei to treno eidopoiume ton epomeno customer 
			check=pthread_cond_signal(&next_customer);
			if(check){
				printf("error signal\n");
			}
		}
		
		if(customer_wait==0){
			train_situation=0;
		}
	}
	
	customer_in++;
	customer_enter++;
	
	printf("o epibatis %d in\n", customer_enter);
	
	if(customer_in == N){//ama epibibastoun n epibates eidopoioume to train
		train_situation=1;
		check=pthread_cond_signal(&train);
		if(check){
			printf("error signal\n");
		}
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	
	check=pthread_cond_wait(&customer_train,&mtx);//anamonh gia apobibash
	if(check){
		printf("error signal\n");
	}
	
	customer_in--;
	customer_exit++;
	
	printf("o epibatis %d out\n", customer_exit);
	
	if (customer_in > 0){//eidopoihsh tou epomenou pelati gia apobibash
		check=pthread_cond_signal(&customer_train);
		if(check){
			printf("error signal\n");
		}
	}
	
	if(customer_in==0){//ama apobibastoun oloi eidopoioume to train
		check=pthread_cond_signal(&train);
		if(check){
			printf("error signal\n");
		}
	}
	
	check=pthread_mutex_unlock(&mtx);
	if(check){
		printf("error unlock\n");
	}
	
	return(NULL);
}


int main (int argc, char * argv[]){
	
	pthread_t train_thread,customer_thread;
	int i, check;
	
	if(argc!=2){
		printf("error argument\n");
	}
	
	all_customer=atoi(argv[1]);
	
	check=pthread_mutex_init(&mtx, NULL);
	if(check){
		printf("error mutex init\n");
	}
	
	
// 	initialization tou mutex kai twn conditions
	check=pthread_cond_init(&train, NULL);
	if(check){
		printf("error train init\n");
	}
	
	check=pthread_cond_init(&next_customer, NULL);
	if(check){
		printf("error next_customer init\n");
	}
	
	check=pthread_cond_init(&customer_train, NULL);
	if(check){
		printf("error customer train init\n");
	}
	
	check=pthread_cond_init(&finish, NULL);
	if(check){
		printf("error finish init\n");
	}
	
	pthread_create(&train_thread , NULL , thread_train, NULL);
	
	
	for(i=0; i<all_customer; i++){
		sleep(1);
		pthread_create(&customer_thread , NULL , thread_customer, NULL);
	}
	
	check=pthread_mutex_lock(&mtx);
	if(check){
		printf("error lock\n");
	}
	//anamonh mexri na trexoun oloi oi epibates
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
