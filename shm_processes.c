#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

int main() 
{
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    // Create shared memory for two integers: 
    // ShmPTR[0] = BankAccount
    // ShmPTR[1] = Turn
    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }
    printf("Process has received a shared memory of two integers...\n");

    // Attach shared memory
    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }
    printf("Process has attached the shared memory...\n");

    // Initialize BankAccount and Turn to 0
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn
    printf("Orig Bank Account = %d\n", ShmPTR[0]);

    pid = fork();

    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    }

    // ----------------------------------------------------
    // Child Process (Poor Student)
    // ----------------------------------------------------
    if (pid == 0) 
    {
        srand(getpid()); // Seed random number generator

        for (int i = 0; i < 25; i++) {
            // 1. Sleep some random amount of time between 0 - 5 seconds
            sleep(rand() % 6);
            
            // 3. loop while Turn != 1 do no-op (Strict Alternation spinlock)
            while (ShmPTR[1] != 1) {
                // no-op
            }
            
            // 2. Copy the value in BankAccount to a local variable account
            int account = ShmPTR[0];
            
            // 4. Randomly generate a balance amount that the Student needs between 0 - 50
            int balance = rand() % 51;
            printf("Poor Student needs $%d\n", balance);
            
            // 5. If the balance needed is <= the account
            if (balance <= account) {
                account -= balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
            } 
            // 6. If the balance needed is > than the account
            else {
                printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }
            
            // 7. Copy value from local variable account back to BankAccount
            ShmPTR[0] = account;
            
            // 8. Set Turn = 0
            ShmPTR[1] = 0;
        }
        exit(0);
    } 
    // ----------------------------------------------------
    // Parent Process (Dear Old Dad)
    // ----------------------------------------------------
    else 
    {
        srand(getpid()); // Seed random number generator

        for (int i = 0; i < 25; i++) {
            // 1. Sleep some random amount of time between 0 - 5 seconds
            sleep(rand() % 6);
            
            // 3. loop while Turn != 0 do no-op (Strict Alternation spinlock)
            while (ShmPTR[1] != 0) {
                // no-op
            }
            
            // 2. Copy the value in BankAccount to a local variable account
            int account = ShmPTR[0];
            
            // 4. If the account is <= 100, then try to Deposit Money
            if (account <= 100) {
                // Deposit Money Rule 1: Generate amount between 0 - 100
                int balance = rand() % 101;
                
                // Deposit Money Rule 2: If even, deposit
                if (balance % 2 == 0) {
                    account += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
                } 
                // Deposit Money Rule 3: If odd, do not deposit
                else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } 
            // 4 (else). If account is > 100
            else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }
            
            // Deposit Money Rule 4: Copy value back to BankAccount
            ShmPTR[0] = account;
            
            // Deposit Money Rule 5: Set Turn = 1
            ShmPTR[1] = 1;
        }
        
        // Wait for the child to finish executing
        wait(&status);
        
        // Detach and clean up shared memory
        shmdt((void *) ShmPTR);
        shmctl(ShmID, IPC_RMID, NULL);
    }
    
    return 0;
}