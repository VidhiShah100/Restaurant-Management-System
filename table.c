#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#define MAX_CUST 5
#define MAX_ORD 100
#define MENU "menu.txt"

int menuSize = 0;

typedef struct {
    int sr_no;
    char item[100];
    int price;
} Menu;



typedef struct {
    int sig;
    int tableNo;
    int bill;
    int cust;
    int isValid;
    int order[MAX_CUST][MAX_ORD]; // Assuming each customer can order multiple items
} Table;

void read_menu(Menu menu[], int items) {
    FILE *file = fopen(MENU, "r");
    if (file == NULL) {
        perror("Error opening menu file");
        exit(EXIT_FAILURE);
    }

    int i = 0;

    while ( fscanf(file, "%d. %[^0-9]%d INR\n", &menu[i].sr_no, menu[i].item, &menu[i].price) == 3) {
        i++;
        menuSize++;
    }
    fclose(file);
}
void display_menu(Menu menu[], int items) {
    printf("Menu:\n");
    int i = 0;
    for (int i = 0; i < menuSize; i++) {
        printf("%d. %s %d INR\n", menu[i].sr_no, menu[i].item, menu[i].price);
    }
}

int validate_order(Menu menu[], int items, int order[], int orders) {
    for (int i = 0; i < orders; i++) {
        int valid = 0;
        for (int j = 0; j < items; j++) {
            if (order[i] == menu[j].sr_no) {
                valid = 1;
                break;
            }
        }
        if (!valid) {
            return 0;
        }
    }
    return 1;
}

int main() {
    int tableNo, cust, i;
    Menu menu[100];

    read_menu(menu,menuSize);

    printf("Enter Table Number: ");
    scanf("%d", &tableNo);


    // Create shared memory segment for communication with waiter process
    key_t key = ftok("table.c", tableNo);
    int shm_id = shmget(key, sizeof(Table), IPC_CREAT | 0666);
    Table *shm_data = (Table *)shmat(shm_id, NULL, 0);
    int flag=1;
    while(flag>=1 && flag<=5){
    while (1)
    {
    printf("Enter Number of Customers at Table (maximum no. of customers can be 5): ");
    scanf("%d", &cust);
    if (cust >5)
    {printf ("invlid, customers can be between 1-5 at a table, please enter again \n");
    }
    else
    { break;}
    }
    shm_data->cust = cust;
    shm_data->isValid = 1;

    for(int i = 0 ; i < MAX_CUST ; i++){
                   for(int j = 0 ; j < MAX_ORD ; j++){
                                  shm_data->order[i][j] = 0;
                   }
    }
 display_menu(menu, menuSize);

    if (shm_data == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < cust; i++) {
        pid_t pid;
        int orders = 0;
        int fd[2];
       if(pipe(fd) == -1)
        {
            perror( "pipe Failed" );
            continue;
        }
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) {
             printf("Customer %d: Enter the serial number(s) of the item(s) to order from the menu. Enter -1 when done: \n", i + 1);
            close(fd[0]);
            int ords[100]= {0};
            while (1) {
            int ab = 0;
                scanf("%d", &ab);
                if (ab == -1) {
                    break; // Exit the loop if -1 is entered
                }
                ords[orders] = ab;
                orders++;
                if (orders >= MAX_ORD) {
                    printf("Maximum number of orders reached.\n");
                    break;
                }
            }
            write(fd[1] , ords , sizeof(ords));
            close(fd[1]);
            exit(EXIT_SUCCESS);
        }
        else{
        wait(NULL);
        int a = 0;
        int temp = 0;
        int ordersArr[100] = {0};
        read(fd[0], ordersArr, sizeof(ordersArr));
        for(int j = 0 ; j < 100 ; j++){
         int temp = ordersArr[j];
         if(temp > 0){  
            shm_data->order[i][j] =  temp;
            }
          }
        }
    }
   
    shm_data->sig = 1;

    for(int i=0;i<cust;i++)
    {
              for(int j=0;j<100;j++)
             {
                        int a = shm_data->order[i][j];
             }
    }

    //while(shm_data->bill ==0);
    sleep(3);

    if(shm_data->isValid == 0){
    shm_data->sig = 2;
    printf("Order was invalid , Enter again\n");
    continue;
    }

    printf("BILL is : %d\n", shm_data->bill);
    // Prompt for continuation or termination
    while (1)
    {
    printf("Do you want to seat a new set of customers at the table (Enter -1 to terminate, 1-5 to continue): ");
    scanf("%d", &flag);
    if(flag == 1)
    {shm_data->sig = 2; break;}
    else if((flag<-1||flag>=6)|| flag==0)
    { printf ("Invalid choice, please enter again \n");}
    else
    {shm_data->sig = flag; break;}
    }

    // Detach and remove shared memory segment
    //shmdt(shm_data);
    //shmctl(shmid, IPC_RMID, NULL);
    }
    return 0;
}



