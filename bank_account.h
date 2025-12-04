#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>


struct bank_account
{
    pthread_mutex_t lock;
    int id;
    double balance;
    FILE *logfile;
};

// Armazena a quantidade de contas existentes.
int global_id = 0;

// Cria uma conta com ID único e saldo zerado.
struct bank_account create_account()
{
    struct bank_account acc;
    acc.lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    acc.id = global_id++;
    acc.balance = 0;

    char filename[64];
    sprintf(filename, "conta_%d.txt", acc.id);
    acc.logfile = fopen(filename, "w");

    
    fprintf(acc.logfile, "================ EXTRATO CONTA #%d ================\n", acc.id);
    fprintf(acc.logfile, "%-12s | %-12s | %s\n", "OPERACAO", "VALOR", "SALDO");
    fprintf(acc.logfile, "---------------------------------------------------\n");
    fflush(acc.logfile);
    fflush(acc.logfile);

    return acc;
}

// Destrói o mutex e limpa toda a memória referente à conta especificada.
void remove_account(struct bank_account *account)
{
    fclose(account->logfile);
    pthread_mutex_destroy(&account->lock);
    // free(account);
}

// Imprime o saldo da conta especificada.
void inquire(struct bank_account *account)
{
    pthread_mutex_lock(&account->lock);

    printf("O saldo da conta #%d é $%#.2f.\n", account->id, account->balance);

    fprintf(account->logfile, "[CONSULTA]   |              | $%.2f\n", account->balance);
    fflush(account->logfile);

    pthread_mutex_unlock(&account->lock);
}

// Deposita uma quantidade de dinheiro na conta especificada.
void deposit(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);

    account->balance += amount;
    
    printf("Depositando $%#.2f na conta #%d.\n", amount, account->id);

    fprintf(account->logfile, "[DEPOSITO]   | +$%-10.2f | $%.2f\n", amount, account->balance);
    fflush(account->logfile);

    pthread_mutex_unlock(&account->lock);
}

// Subtrai uma quantidade de dinheiro da conta especificada, se ela tem saldo suficiente.
void withdraw(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);

    if (amount <= account->balance) {
        printf("Retirando $%#.2f da conta #%d.\n", amount, account->id);
        account->balance -= amount;
        fprintf(account->logfile, "[SAQUE]      | -$%-9.2f  | $%.2f\n", amount, account->balance);
    } else {
        printf("A conta #%d não tem saldo suficiente para retirar $%#.2f.\n", account->id, amount);
        fprintf(account->logfile, "[FALHA SAQUE]| (Tentou %.0f)  | $%.2f (Insuficiente)\n", amount, account->balance);
    }
    fflush(account->logfile);

    pthread_mutex_unlock(&account->lock);
}

// Transfere uma quantidade de dinheiro de uma conta para outra, se o transferidor tem saldo suficiente.
void transfer(struct bank_account *transferor, struct bank_account *transferee, double amount)
{
    // sempre pega a chave da conta menor id primeiro
    struct bank_account *first  = (transferor->id < transferee->id) ? transferor : transferee;
    struct bank_account *second = (transferor->id < transferee->id) ? transferee : transferor;

    pthread_mutex_lock(&first->lock);
    pthread_mutex_lock(&second->lock);

    if (amount <= transferor->balance) {
        transferor->balance -= amount;
        transferee->balance += amount;

        printf("Transferindo $%#.2f da conta #%d para a conta #%d.\n", amount, transferor->id, transferee->id);

        if (transferor->logfile) {
            fprintf(transferor->logfile, "[TRANSF ENV] | -$%-9.2f | $%.2f (Para #%d)\n", amount, transferor->balance, transferee->id);
            fflush(transferor->logfile);
        }

        if (transferee->logfile) {
            fprintf(transferee->logfile, "[TRANSF REC] | +$%-9.2f | $%.2f (De #%d)\n", amount, transferee->balance, transferor->id);
            fflush(transferee->logfile);
        }

    } else {
        printf("Falha ao transferir #%d -> #%d (Saldo insuficiente)\n", transferor->id, transferee->id);
        if (transferor->logfile) {
            fprintf(transferor->logfile, "[FALHA TRANS]| (Tentou %.0f)  | $%.2f (Insuficiente)\n", amount, transferor->balance);
            fflush(transferor->logfile);
        }
    }

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

#endif