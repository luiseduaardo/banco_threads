/* bankAccount.h simulates a simple bank account
 *
 * Joel Adams, Calvin College, Fall 2013.
 */


// Shared Variables
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
double bankAccountBalance = 0;

// add amount to bankAccountBalance
void deposit(double amount) {
   pthread_mutex_lock(&lock);
   bankAccountBalance += amount;
   pthread_mutex_unlock(&lock);
}

// subtract amount from bankAccountBalance
void withdraw(double amount) {
   pthread_mutex_lock(&lock);
   bankAccountBalance -= amount;
   pthread_mutex_unlock(&lock);
}

void cleanup() {
   pthread_mutex_destroy(&lock);
}

// transferencia

// checar saldo

// fazer isso para pelo menos 5 contas

// conta a ser manipulada por cada thread é definida aleatoriamente, assim como a ação

// mostrar balanço de todas as threads a cada operação para checar saldo