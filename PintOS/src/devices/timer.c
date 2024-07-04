#include "list.h"

//////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado ///////////////////////////////////
//////////////////////////////////////////////////////////////////
/* Sleeps for approximately TICKS timer ticks.  Interrupts must
   be turned on. */
void
timer_sleep (int64_t ticks_) 
{
  // Checar se não é um valor negativo ou zero;
  if(ticks_ <= 0)
    return;
  
  ASSERT (intr_get_level () == INTR_ON);
  // Função para definir o tempo que a thread vai demorar para acordar;
  thread_yield_block(ticks_ + timer_ticks());
}
//////////////////////////////////////////////////////////////////
// Resto do arquivo inalterado ///////////////////////////////////
//////////////////////////////////////////////////////////////////