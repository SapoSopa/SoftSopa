#include "../lib/kernel/float_point.h"
#include "devices/timer.h"

/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
// Multi-level queue; 
static struct list ready_multi[PRI_MAX + 1];

/////////////////////////////////////////////////////////////////

// Lista de threads bloqueadas;
static struct list block_list;
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
void
thread_init (void) 
{
  ASSERT (intr_get_level () == INTR_OFF);

  lock_init (&tid_lock);
  // Inicializando a lista de threads prontas;
  init_ready_lists();
  list_init (&all_list);
  // Inicializando a lista de threads bloqueadas;
  list_init(&block_list);

  /* Set up a thread structure for the running thread. */
  initial_thread = running_thread ();
  init_thread (initial_thread, "main", PRI_DEFAULT);
  initial_thread->status = THREAD_RUNNING;
  initial_thread->tid = allocate_tid ();
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
/* Called by the timer interrupt handler at each timer tick.
   Thus, this function runs in an external interrupt context. */
void
thread_tick (void) 
{  
  struct thread *t = thread_current ();
  // Atualiza o cpu_recent_time da thread atual;
  if(thread_mlfqs){
    enum intr_level old_level = intr_disable();
    if (thread_current() != idle_thread)
      add_cpu();

    if(timer_ticks() % TIMER_FREQ == 0){
      avg_cal();
    }
    update_info(timer_ticks());
    
    intr_set_level(old_level);
  }

  /* Update statistics. */
  if (t == idle_thread)
    idle_ticks++;

#ifdef USERPROG
  else if (t->pagedir != NULL)
    user_ticks++;
#endif
  else
    kernel_ticks++;
   
  /* Enforce preemption. */
  if (++thread_ticks >= TIME_SLICE)
    intr_yield_on_return ();
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
void
thread_unblock (struct thread *t) 
{
  enum intr_level old_level;

  ASSERT (is_thread (t));

  old_level = intr_disable ();
  ASSERT (t->status == THREAD_BLOCKED);
  // Adiciona a thread na lista de prontos;
  add_ready (&t->elem);
  t->status = THREAD_READY;
  intr_set_level (old_level);
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
void
thread_yield (void) 
{
  struct thread *cur = thread_current ();
  enum intr_level old_level;
  
  ASSERT (!intr_context ());

  old_level = intr_disable ();
  if (cur != idle_thread) 
    // Adiciona a thread na lista de prontos;
    add_ready(&cur->elem);
  cur->status = THREAD_READY;
  schedule ();
  intr_set_level (old_level);
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
// Determina a prioridade da thread atual;
void
thread_set_priority (int new_priority) 
{
  if (!thread_mlfqs) {
    thread_current ()->priority = new_priority;
  }
}

/* Returns the current thread's priority. */
int
thread_get_priority (void) 
{
  return thread_current ()->priority;
}

/* Sets the current thread's nice value to NICE. */
void
thread_set_nice (int nice UNUSED) 
{
  struct thread *t = thread_current();
  enum intr_level old_level;
  
  t->nice = nice;

  // Atualiza a prioridade;
  update_priority(t);
  // Ajusta a ordem atual de execução das threads de acordo com a nova prioridade;
  if (thread_mlfqs && t->priority < hightest_priority())
    thread_yield(); 
}

/* Returns the current thread's nice value. */
int
thread_get_nice (void) 
{
  return thread_current()->nice;
}

// Calcula o avg usando a implementação do float que criamos no lib/kernel;
void
avg_cal()
{ 
  float_type a = FLOAT_DIV(INT_FLOAT(59), INT_FLOAT(60));
  float_type b = FLOAT_DIV(INT_FLOAT(1), INT_FLOAT(60));

  avg = FLOAT_MUL(a, avg) + FLOAT_INT_MUL(b, ((int64_t)ready_list_size() + (int64_t)(thread_current() != idle_thread)));
}

/* Returns 100 times the system load average. */
int
thread_get_load_avg (void) 
{ 
  enum intr_level old_level = intr_disable();
  int r =  FLOAT_INT_ROUND(avg * 100);
  intr_set_level(old_level);

  return r;
}

// Calcula o recent_cpu_time da thread atual;
void
cpu_calc(struct thread *t, void *aux)
{
  if(t != idle_thread)
    t->recent_cpu_time = FLOAT_MUL(t->recent_cpu_time, FLOAT_DIV(2*avg, FLOAT_INT_ADD(2*avg, 1))) + INT_FLOAT(t->nice);
}

// Soma o recent_cpu_time da thread atual em 1;
void
add_cpu()
{
  struct thread *t = thread_current();
  if(t != idle_thread)
    t->recent_cpu_time = FLOAT_INT_ADD(t->recent_cpu_time, 1);
}

/* Returns 100 times the current thread's recent_cpu value. */
int
thread_get_recent_cpu (void) 
{
  enum intr_level old_level = intr_disable();
  int c = FLOAT_INT_ROUND(thread_current()->recent_cpu_time * 100);
  intr_set_level(old_level);
  
  return c;
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
/* Does basic initialization of T as a blocked thread named
   NAME. */
static void
init_thread (struct thread *t, const char *name, int priority)
{
  enum intr_level old_level;

  ASSERT (t != NULL);
  ASSERT (PRI_MIN <= priority && priority <= PRI_MAX);
  ASSERT (name != NULL);

  memset (t, 0, sizeof *t);
  t->status = THREAD_BLOCKED;
  strlcpy (t->name, name, sizeof t->name);
  t->stack = (uint8_t *) t + PGSIZE;

// Iniciando os valores adicionais das threads conforme previsto;
  if (!thread_mlfqs) {
    t->priority = priority;
  }
  else {
    t->priority = PRI_MAX;
  }
  t->sleep_time = 0;
  t->recent_cpu_time = 0;
  t->nice = 0;
  t->magic = THREAD_MAGIC;

  if(t != initial_thread){
      struct thread* y = thread_current();
      t->recent_cpu_time = y->recent_cpu_time;
      t->nice = y->nice;
  } else {
      t->recent_cpu_time = 0;
      t->nice = 0;
  }
/////////////////////////////////////////////////////////////////

  old_level = intr_disable ();
  list_push_back (&all_list, &t->allelem);
  intr_set_level (old_level);
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
static struct thread *
next_thread_to_run (void) 
{
  // Verifica se a lista de prontos está vazia;
  if(!list_empty(&block_list))
    wake(timer_ticks());

  if (ready_empty())
    return idle_thread;
  else
    return list_entry (pop_next_ready(), struct thread, elem);
}
/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////
// Ordenas as threads de acordo com o tempo de bloqueio;
bool
ord (const struct list_elem *a, const struct list_elem *b, void *aux)
{
  struct thread *A = list_entry(a, struct thread, elem), *B = list_entry(b, struct thread, elem);

  return A->sleep_time > B->sleep_time;
}

// Bloqueia a thread atual por um tempo determinado;
void
thread_yield_block(int sleep_time)
{
  struct thread *t = thread_current();
  enum intr_level old_level;

  ASSERT (!intr_context ());

  if(t != idle_thread){
    t->sleep_time = sleep_time;
    old_level  = intr_disable();
    
    list_insert_ordered(&block_list, &(t->elem), ord, NULL);
    thread_block();
    intr_set_level(old_level);
  }
}

// Acorda as threads que estão bloqueadas;
void
wake(int64_t ticks)
{  
  enum intr_level old_level;
  old_level = intr_disable();
  intr_set_level(old_level);
  struct list_elem *e = list_rbegin(&block_list);
  // Verifica se a thread está bloqueada;
  while(e != list_rend(&block_list))
  {
    struct thread *t = list_entry(e, struct thread, elem);

    // Verifica se o tempo de bloqueio da thread é menor ou igual ao tempo atual;
    if(t->sleep_time <= ticks){
      old_level = intr_disable();
      list_pop_back(&block_list);

      ASSERT(t->status == THREAD_BLOCKED);
      thread_unblock(t);

      // Desbloquear;
      if(!list_empty(&block_list)){
        e = list_rbegin(&block_list);
      } else {
        break;
      }
    } else {
      break;
    }
  }
  
  intr_set_level(old_level);
}

// Inicializa as listas de prontos;
void
init_ready_lists(void)
{
  switch (thread_mlfqs)
  {
  case true:
    for(int i = 0; i < PRI_MAX + 1; i++)
      list_init (&ready_multi[i]);
    break;
  case false:
    list_init (&ready_list);
    break;
  default:
    break;
  }
}

// Retorna o tamanho da lista de prontos;
size_t
ready_list_size()
{
  switch (thread_mlfqs)
  {
  case true:
    size_t total = 0;
    for (int i = 0; i <= PRI_MAX; i++)
      total += list_size(&ready_multi[i]);
    return total;
  case false:
    return list_size(&ready_list);    
  default:
    break;
  }
}

// Retorna a prioridade mais alta;
int
hightest_priority()
{
  switch (thread_mlfqs)
  {
  case true:
    for (int i = PRI_MAX; i >= 0; i--)
      if (!list_empty(&ready_multi[i]))
        return i;
    return -1;
  case false:
    if (list_empty(&ready_list))
      return -1;
    return list_entry(list_begin(&ready_list), struct thread, elem)->priority;
  default:
    break;
  }
}

/////////////////////////////////////////////////////////////////
// round_robin;
void
rr_add_ready(struct list_elem* elem)
{
  list_push_back(&ready_list, elem);
}

bool
rr_ready_empty(void)
{
  return list_empty(&ready_list);
}

struct list_elem *
rr_pop_next_ready(void)
{
  return list_pop_front(&ready_list);
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// MLFQS;
void
ml_add_ready(struct list_elem* elem)
{
  struct thread *t = list_entry(elem, struct thread, elem);
  list_push_back(&ready_multi[(int) t->priority], elem);
}

bool
ml_ready_empty(void)
{
  for(int i = 0; i <= PRI_MAX; i++)
    if(!list_empty(&ready_multi[i]))
      return false;

  return true;
}

struct list_elem *
ml_pop_next_ready(void)
{
  for(int i = PRI_MAX; i >= 0; i--)
    if(!list_empty(&(ready_multi[i])))
      return list_pop_front(&(ready_multi[i]));

  return NULL;
}
/////////////////////////////////////////////////////////////////

// Atualiza a prioridade da thread;
void
update_priority(struct thread *t)
{
  if (t == idle_thread)
    return;

  t->priority = FLOAT_INT_ZERO( (INT_FLOAT(PRI_MAX) - FLOAT_INT_DIV(t->recent_cpu_time, ((int64_t) 4)) - INT_FLOAT((2 * t->nice)) ));
  
  if (t->priority < PRI_MIN)
    t->priority = PRI_MIN;
  
  if (t->priority > PRI_MAX)
    t->priority = PRI_MAX;
}

// Atualiza as informações das threads;
void
update_info(int64_t time)
{
  struct list_elem *e;
  struct thread *t;

  ASSERT (intr_get_level () == INTR_OFF);

  for (e = list_begin (&all_list); e != list_end (&all_list);
       e = list_next (e))
  {
  
    t = list_entry (e, struct thread, allelem);

    if (time % TIMER_FREQ == 0) {
      cpu_calc(t, NULL);
    }
    
    if (t == idle_thread || !(time % 4 == 0)) {
      continue;
    }
    
    int past_priority = t->priority;  
    
    update_priority(t);

    if (t->status == THREAD_READY && t->priority != past_priority) {
      list_remove(&t->elem);
      list_push_back(&ready_multi[(int) t->priority], &t->elem);
    }
  }
}

/////////////////////////////////////////////////////////////////
// For handling multiple schedules;
void
add_ready(struct list_elem* elem)
{
  switch (thread_mlfqs)
  {
  case true:
    return ml_add_ready(elem);
  case false:
    return rr_add_ready(elem);
  default:
    break;
  }
}

bool
ready_empty(void)
{
  switch (thread_mlfqs)
  {
  case true:
    return ml_ready_empty();
  case false:
    return rr_ready_empty();
  default:
    break;
  }
}

struct list_elem *
pop_next_ready(void)
{
  switch (thread_mlfqs)
  {
  case true:
    return ml_pop_next_ready();
  case false:
    return rr_pop_next_ready();
  default:
    return NULL;
  }
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado //////////////////////////////////
/////////////////////////////////////////////////////////////////