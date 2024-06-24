#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>

#define MAX_CUST 5
#define MAX_ORD 100
#define MENU "menu.txt"

int menuSize = 0;


//structure for calculating bill for each set of cutomers sitting at the table
typedef struct {
        int sig;
        int sets;
        int bills[100];
} Bill;



//structure for menu items, same as the one defined in table process
typedef struct {
    int sr_no;
    char item[100];
    int price;
} Menu;



//structure for table data, same as the one defined in taSble process
typedef struct {
    int sig;
    int tableNo;
    int bill;
    int cust;
    int isValid;
    int order[MAX_CUST][MAX_ORD]; // Assuming each customer can order multiple items
} Table;



int main() {
           Menu menu[100];

FILE *file = fopen(MENU, "r");
    if (file == NULL) {
        perror("Error opening menu file");
        exit(EXIT_FAILURE);
    }

    int ij = 0;

    while ( fscanf(file, "%d. %[^0-9]%d INR\n", &menu[ij].sr_no, menu[ij].item, &menu[ij].price) == 3) {
        ij++;
        menuSize++;
    }

    	fclose(file);

    	int wId;
    	printf("Enter Waiter ID: ");
    	scanf("%d", &wId);

    	// Create shared memory segment for communication of waiter process with table process
    	key_t key_table = ftok("table.c", wId);
    	int shmid_table;
    	while( (shmid_table = shmget(key_table, sizeof(Table),0666)) == -1);
    	Table *shm_data = (Table *)shmat(shmid_table, NULL, 0);

    	// Create shared memory segment for communication of waiter process with hotel manager process
    	key_t key_manager = ftok("manager.c", wId);
    	int shmid_manager = shmget(key_manager, sizeof(int), IPC_CREAT | 0666);
    	Bill *shm_ptr_manager = (Bill *)shmat(shmid_manager, NULL, 0);
    	memset(shm_ptr_manager , 0 , sizeof(int)*102);

    	while (1) {
        		// Wait for signal from table process
        		while (shm_data->sig != 1);
        		int numCust = shm_data->cust;
        		int orders[shm_data->cust*100];
        		int ord_num = 0;
       		int valid = 1;
      
        

for(int i=0;i<shm_data->cust;i++)
{
    for(int j=0;j<100;j++)
    {
    	int a = shm_data->order[i][j];
    	if(a > 0){
           	if(a > menuSize)
                  		valid = 0;
    	orders[ord_num++] = a;
}
    }
 }

        // Process order if valid, otherwise reset table state. Asks for order of entire table again. 
        if (valid==0) {
            shm_data->isValid = 0;
            printf("Invalid Order. Please enter again\n");
            shm_data->sig = 2;
        }

        else {
            
//calculating bill in waiter process and conveying it to the table process by updating shared memory
            int total_bill = 0;
            for (int i = 0; i < ord_num; i++) {
                total_bill += menu[orders[i]-1].price;
            }

            // Display bill amount and update for manager through shared memory
            printf("Bill Amount for Table %d: %d INR\n", wId, total_bill);
            shm_ptr_manager->bills[shm_ptr_manager->sets++] = total_bill;
            shm_data->bill = total_bill;
        }

while(shm_data->sig == 1);
        // Check for termination signal
        if (shm_data->sig == -1) {
            break;
        }
        shm_data->sig = 0;
    }
    shm_ptr_manager->sig = -1;

    // Detach and remove shared memory segments
    shmdt(shm_data);
    shmctl(shmid_table, IPC_RMID, NULL);
    return 0;
}



