#include "bank_account.h"

#define ACCOUNT_COUNT 5
#define MAX_TRANSACTION_VALUE 100

#define INQUIRE 0
#define DEPOSIT 1
#define WITHDRAW 2
#define TRANSFER 3

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

size_t get_thread_count(int argc, char **argv)
{
    if (argc == 2) return atoll(argv[1]);

    if (argc == 1) return 1;

    printf("Argumentos inesperados: use ./main [no. threads]\n");
    exit(1);
}

void *do_random_action(void *x)
{
    struct bank_account *accounts = (struct bank_account *) x;

    size_t i = rand() % ACCOUNT_COUNT;
    struct bank_account *account = &accounts[i];

    int action = rand() % 4;
    int amount = 1 + (rand() % MAX_TRANSACTION_VALUE);
    switch (action)
    {
        case INQUIRE:
            inquire(account);
            break;
        case DEPOSIT:
            deposit(account, amount);
            break;
        case WITHDRAW:
            withdraw(account, amount);
            break;
        case TRANSFER:
            struct bank_account *account2;
            do
            {
                size_t j = rand() % ACCOUNT_COUNT;
                account2 = &accounts[j];
            } while (account == account2);

            transfer(account, account2, amount);
            break;
    }
}

int main(int argc, char **argv)
{
    logfile = fopen("extrato.txt", "w");

    fprintf(logfile, "--- INICIO DA SIMULACAO BANCARIA ---\n");
    fprintf(logfile, "Contas Ativas: %d\n", ACCOUNT_COUNT);
    fprintf(logfile, "------------------------------------\n");
    fprintf(logfile, "EXTRATO\n");

    size_t thread_count = get_thread_count(argc, argv);
    pthread_t *threads = malloc(thread_count * sizeof(pthread_t));

    struct bank_account accounts[ACCOUNT_COUNT];
    for (int i = 0; i < ACCOUNT_COUNT; i++) accounts[i] = create_account();

    printf("--------EXTRATO--------\n");
    for (size_t t = 0; t < thread_count; t++)
        pthread_create(&threads[t], NULL, do_random_action, (void *) &accounts);

    do_random_action((void *) &accounts);

    for (size_t t = 0; t < thread_count; t++)
        pthread_join(threads[t], NULL);

    printf("\n--- SIMULACAO CONCLUIDA ---\n");
    fprintf(logfile, "------------------------------------\n");
    fprintf(logfile, "--- SALDOS FINAIS ---\n");
    for (int i = 0; i < ACCOUNT_COUNT; i++) 
    {
        inquire(&accounts[i]);
        remove_account(&accounts[i]);
    }

    fclose(logfile);
}