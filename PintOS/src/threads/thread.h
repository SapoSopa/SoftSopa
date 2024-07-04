#include "../lib/kernel/float_point.h"

//////////////////////////////////////
// Resto do arquivo inalterado////////
//////////////////////////////////////

struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */
    // Criar uma variável para armazenar o priority e nice de maneira adequada;
    int64_t priority, nice;             /* Priority e nice. */

    struct list_elem allelem;           /* List element for all threads list. */

    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    // Criar uma variável para armazenar o sleep_time de maneira adequada;
    uint64_t sleep_time;                /* Sleep time. */
    // Criar uma variável para armazenar o recent_cpu de maneira adequada;
    int recent_cpu_time;                /* Recent CPU time. */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory. */
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
    // Criar uma variável para armazenar o tempo que a thread irá dormir;
    int64_t sleep_ticks;                /* Sleep ticks. */
  };

// Criar uma lista para armazenar as threads bloqueadas;
static struct list block_list;

// Criar o avg global;
static float_type avg = 0;

//////////////////////////////////////
// Resto do arquivo inalterado////////
//////////////////////////////////////

// Funções modificadas para a implementação;
int thread_get_priority (void);
void thread_set_priority (int);
int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

// Funções criadas para a implementação;

bool ord (const struct list_elem *a, const struct list_elem *b, void *aux);
void avg_cal(void);
void add_cpu(void);
void cpu_calc(struct thread *t, void *aux);
void thread_yield_block(int sleep_time);
void wake(int64_t ticks);

void init_ready_lists();
size_t ready_list_size();

   // round_robin; 
void rr_add_ready(struct list_elem* elem);
bool rr_ready_empty(void);
struct list_elem *rr_pop_next_ready(void);

   // mlfqs;
int hightest_priority();
void ml_add_ready(struct list_elem* elem);
bool ml_ready_empty(void);
struct list_elem *ml_pop_next_ready(void);
void update_priority_one(struct thread *t);
void update_priority(struct thread *t);
void update_info(int64_t time);

   // for handling multiple schedules;
void add_ready(struct list_elem* elem);
bool ready_empty(void);
struct list_elem *pop_next_ready(void);
//////////////////////////////////////
// Resto do arquivo inalterado ///////
//////////////////////////////////////