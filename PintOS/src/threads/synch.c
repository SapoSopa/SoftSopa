///////////////////////////////////////////////////////////////
// Resto do arquivo inalterado ////////////////////////////////
///////////////////////////////////////////////////////////////
/* Up or "V" operation on a semaphore.  Increments SEMA's value
   and wakes up one thread of those waiting for SEMA, if any.

   This function may be called from an interrupt handler. */
void
sema_up (struct semaphore *sema) 
{
  enum intr_level old_level;

  ASSERT (sema != NULL);

  old_level = intr_disable ();
  struct list_elem *next_thread = NULL;
  int8_t max_priority = PRI_MIN - 1;
  if (!list_empty (&sema->waiters)) {
    // Encontra a thread de maior prioridade para desbloquear;
    if (thread_mlfqs) {
      for (struct list_elem* e = list_begin (&sema->waiters); 
            e != list_end (&sema->waiters); e = list_next (e)) 
      {
        struct thread *t = list_entry (e, struct thread, elem);

        if (t->priority > max_priority) {
          max_priority = t->priority;
          next_thread = e;
        }
      }  

      list_remove(next_thread);      
    } 
    else {
      next_thread = list_pop_front(&sema->waiters);
    }

    struct thread *t = list_entry (next_thread, struct thread, elem);
    // Atualiza a prioridade da thread que vai ser liberada se for uma mlfqs;
    if (thread_mlfqs)
      update_priority(t);
    
    // Desbloqueia a thread;
    ASSERT(t->status == THREAD_BLOCKED);
    thread_unblock (t);
  }
    
  sema->value++;
  intr_set_level (old_level);
  
  // Verificar se a prioridade da thread atual é menor que a da thread desbloqueada;
  if (thread_mlfqs && max_priority > thread_current()->priority) {
    thread_yield();
  }
}
///////////////////////////////////////////////////////////////
// Resto do arquivo inalterado ////////////////////////////////
///////////////////////////////////////////////////////////////