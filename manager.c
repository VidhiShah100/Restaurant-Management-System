#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#define MAX_CUST 5
#define MAX_ORD 100
#define MAX_TABLES 10
#define MAX_WAITERS 10

typedef struct {
       int sig ;
       int sets;
       int bills[100] ;
} Bill;

typedef struct {
    int sig;
    int tableNo;
    int bill;
    int cust;
    int isValid;
    int order[MAX_CUST][MAX_ORD];
} Table;

int main (){

key_t adminkey = ftok("admin.c", 'A');
int admin_id = shmget(adminkey, sizeof(int), IPC_CREAT | 0666);
int *admin_ptr = shmat(admin_id, NULL, 0);

int numTable;
int totalSal = 0;



// opening the earnings.txt file for manager to update the total earnings, wage and profit earned.
FILE *earnings_file = fopen("earnings.txt", "w");
    if (earnings_file == NULL) {
        perror("Error opening earnings.txt");
        exit(EXIT_FAILURE);
    }
printf("Enter the Total Number of Tables at the Hotel: \n");
scanf("%d" , &numTable);

Bill* tableBills[10];
int shmid_manager[10];

for(int i = 0 ; i < 10 ; i++){
    key_t key_manager = ftok("manager.c", i+1);
    shmid_manager[i] = shmget(key_manager, sizeof(int), IPC_CREAT | 0666);
    tableBills[i] = (Bill *)shmat(shmid_manager[i], NULL, 0);
    memset(tableBills[i] , 0 , sizeof(int)*102);
    }

    int check = 0;
    while(1){
   	 for(int i = 0 ; i < numTable ; i++){
   	 	if(tableBills[i]->sig == -1){
    			check++;
    			tableBills[i]->sig = 0;
    			for(int j = 0 ; j < tableBills[i]->sets ; j++){
    				if(tableBills[i]->bills[j] != -1){
    					totalSal += tableBills[i]->bills[j];
    					printf("Earnings from Table number-%d = %d\n", i+1 , tableBills[i]->bills[j]);
    					fprintf(earnings_file , "Table %d , BILL : %d\n", i+1 , tableBills[i]->bills[j]);
   				 	tableBills[i]->bills[j] = 0;

    				}
    			}
   		 }
  	  }
    	if(check >= numTable){
    		printf("Waiting for admin to close\n");
    		break;
    	}
    	sleep(5);
    }
    int tp = 0;
    tp = 1;
    tp = 2;
    //while loop here to check admin termination
    while(*admin_ptr == 0);

    printf("Total Earnings of hotel = %d\nTotal wage of waiters = %d\nTotal Net Profit = %d\n",totalSal , totalSal*4/10 , totalSal*6/10);
    fprintf(earnings_file,"Total Earnings of hotel = %d\nTotal wage of waiters = %d\nTotal Net Profit = %d\n",totalSal , totalSal*4/10 , totalSal*6/10);
    printf("Thank you for visiting the Hotel !\n");

    for(int i = 0 ; i < numTable ; i++){
    shmdt(tableBills[i]);
    shmctl(shmid_manager[i], IPC_RMID, NULL);
    }
fclose(earnings_file);
return 0;
}


