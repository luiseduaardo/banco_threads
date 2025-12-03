#ifndef BANK_ACCOUNT_H
#define BANK_ACCOUNT_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

FILE *logfile;
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER;

struct bank_account
{
    pthread_mutex_t lock;
    int id;
    double balance;
};

// Armazena a quantidade de contas existentes.
int global_id = 0;

void add_to_log(char* message) {
    pthread_mutex_lock(&file_mutex);

    if (logfile != NULL) {
        fprintf(logfile, "%s", message);
        fflush(logfile);
    }

    pthread_mutex_unlock(&file_mutex);
}

// Cria uma conta com ID único e saldo zerado.
struct bank_account create_account()
{
    return (struct bank_account) { PTHREAD_MUTEX_INITIALIZER, global_id++, 0 };
}

// Destrói o mutex e limpa toda a memória referente à conta especificada.
void remove_account(struct bank_account *account)
{
    pthread_mutex_destroy(&account->lock);
    // free(account);
}

// Imprime o saldo da conta especificada.
void inquire(struct bank_account *account)
{
    pthread_mutex_lock(&account->lock);

    char buffer[128];
    sprintf(buffer, "[CONSULTA] Conta #%d |    -   | Saldo atual:         $%.2f\n", account->id, account->balance);
    add_to_log(buffer);

    printf("O saldo da conta #%d é $%#.2f.\n", account->id, account->balance);

    pthread_mutex_unlock(&account->lock);
}

// Deposita uma quantidade de dinheiro na conta especificada.
void deposit(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);

    char buffer[128];
    sprintf(buffer, "[DEPÓSITO] Conta #%d | +$%.2f | Novo saldo: $%.2f\n", account->id, amount, account->balance);
    add_to_log(buffer);

    printf("Depositando $%#.2f na conta #%d.\n", amount, account->id);
    account->balance += amount;

    pthread_mutex_unlock(&account->lock);
}

// Subtrai uma quantidade de dinheiro da conta especificada, se ela tem saldo suficiente.
void withdraw(struct bank_account *account, double amount)
{
    pthread_mutex_lock(&account->lock);

    char buffer[128];

    if (amount <= account->balance) {
        printf("Retirando $%#.2f da conta #%d.\n", amount, account->id);
        account->balance -= amount;
        sprintf(buffer, "[SAQUE]     Conta #%d | -$%.2f | Novo saldo: $%.2f\n", account->id, amount, account->balance);
    } else {
        printf("A conta #%d não tem saldo suficiente para retirar $%#.2f.\n", account->id, amount);
        sprintf(buffer, "[ERRO]     Conta #%d | -$%.2f | Saldo insuficiente: $%.2f\n", account->id, amount, account->balance);
    }

    add_to_log(buffer);

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

    char buffer[256];

    if (amount <= transferor->balance) {
        sprintf(buffer, "[TRANSF]   De #%d Para #%d | Valor: $%.2f | Saldo Origem: $%.2f | Saldo Destino: $%.2f\n", transferor->id, transferee->id, amount, transferor->balance, transferee->balance);
        printf("Transferindo $%#.2f da conta #%d para a conta #%d.\n", amount, transferor->id, transferee->id);
        transferor->balance -= amount;
        transferee->balance += amount;
    } else {
        sprintf(buffer, "[ERRO TRANSF] De #%d Para #%d | Valor: $%.2f | Saldo Insuficiente na Origem\n", transferor->id, transferee->id, amount);
        printf("A conta #%d não tem saldo suficiente para retirar $%#.2f.\n", transferor->id, amount);
    }

    add_to_log(buffer);

    pthread_mutex_unlock(&second->lock);
    pthread_mutex_unlock(&first->lock);
}

#endif