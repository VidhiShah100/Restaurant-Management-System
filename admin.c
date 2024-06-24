#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main() {
    char ch;

    //creating shared memory between admin and hotel manager
    key_t key = ftok("admin.c", 'A');
    int shm_id = shmget(key, sizeof(int), IPC_CREAT | 0666);
    int *shm_ptr = shmat(shm_id, NULL, 0);
    *shm_ptr = 0;
    

    //loop to ask admin if they want to close the hotel or not. It gives an error statement if any answer other than Y/N/y/n is provided. 
    while (1) {
        printf("Do you want to close the hotel? Enter Y for Yes and N for No: ");
        scanf(" %c", &ch);

        if (ch == 'Y' || ch == 'y') {
            *shm_ptr = 1;
            break;
        } 
        else if (ch == 'N' || ch == 'n') {
            continue;
        } 
        else {
            printf("Invalid choice. Please enter Y or N.\n");
        }

    }

    //clearing all the shared memory if the admin chooses to close the hotel
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);
    return 0;
}



