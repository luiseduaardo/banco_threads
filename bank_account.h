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

    fprintf(acc.logfile, "--- EXTRATO DA CONTA #%d ---\n", acc.id);
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

    fprintf(account->logfile, "[CONSULTA]     Conta #%d      |    -   | Saldo atual:         $%.2f\n", account->id, account->balance);
    fflush(account->logfile);

    pthread_mutex_unlock(&account->lock);
}

// Deposita uma quantidade de dinheiro na conta especificada.
void deposit(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);
    
    printf("Depositando $%#.2f na conta #%d.\n", amount, account->id);

    fprintf(account->logfile, "[DEPÓSITO]     Conta #%d      | +$%.2f | Novo saldo: $%.2f\n", account->id, amount, account->balance);
    fflush(account->logfile);

    account->balance += amount;

    pthread_mutex_unlock(&account->lock);
}

// Subtrai uma quantidade de dinheiro da conta especificada, se ela tem saldo suficiente.
void withdraw(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);

    if (amount <= account->balance) {
        printf("Retirando $%#.2f da conta #%d.\n", amount, account->id);
        account->balance -= amount;
        fprintf(account->logfile, "[SAQUE]        Conta #%d      | -$%.2f | Novo saldo: $%.2f\n", account->id, amount, account->balance);
    } else {
        printf("A conta #%d não tem saldo suficiente para retirar $%#.2f.\n", account->id, amount);
        fprintf(account->logfile, "[ERRO]         Conta #%d      | -$%.2f | Saldo insuficiente: $%.2f\n", account->id, amount, account->balance);
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
            fprintf(transferor->logfile, "[ENVIOU]       Para Conta #%d      | Valor: $%.2f | Saldo Restante: $%.2f\n", 
                    transferee->id, amount, transferor->balance);
            fflush(transferor->logfile);
        }

        if (transferee->logfile) {
            fprintf(transferee->logfile, "[RECEBEU]      De Conta #%d        | Valor: $%.2f | Novo Saldo: $%.2f\n", 
                    transferor->id, amount, transferee->balance);
            fflush(transferee->logfile);
        }

    } else {
        printf("Falha ao transferir #%d -> #%d (Saldo insuficiente)\n", transferor->id, transferee->id);
        if (transferor->logfile) {
            fprintf(transferor->logfile, "[FALHA TRANSF] Tentou enviar $%.2f p/ #%d | Saldo Insuficiente\n", 
                    amount, transferee->id);
            fflush(transferor->logfile);
        }
    }

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

#endif