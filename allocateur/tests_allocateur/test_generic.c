#include <sys/time.h>
#include <sys/resource.h>

#include <CUnit/CUnit.h>

#include "../mem.h"
#include "test_run_cpp.h"
#include "test_generic.h"

void test_init()
{
  int i;

  CU_ASSERT_PTR_NULL_FATAL( mem_alloc(64) );

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );
  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);

  for(i=0; i< 100; i++)
    {
      void *m1=0;
      CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );
      CU_ASSERT_PTR_NOT_NULL_FATAL( m1  = mem_alloc(64) );
      memset(m1, i, 64);
      CU_ASSERT_EQUAL_FATAL( mem_free(m1, 64), 0 );
      CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
    }
}

void test_alloc_free()
{
  void *m1=0, *m2=0;

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0);
  CU_ASSERT_PTR_NULL_FATAL( mem_alloc(0) );  

  CU_ASSERT_PTR_NOT_NULL_FATAL( m1 = mem_alloc(HEAP_SIZE) );
  memset(m1, 1, HEAP_SIZE);
  CU_ASSERT_PTR_NULL_FATAL( mem_alloc(1) );
  CU_ASSERT_EQUAL_FATAL( mem_free(m1, HEAP_SIZE), 0);

  CU_ASSERT_PTR_NOT_NULL_FATAL( m2 = mem_alloc(HEAP_SIZE) );
  memset(m1, 2, HEAP_SIZE);
  CU_ASSERT_PTR_EQUAL_FATAL( m1, m2 );
  CU_ASSERT_EQUAL_FATAL( mem_free(m2, HEAP_SIZE), 0);

  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
}

void test_free_boundaries()
{
  void *m1=0, *m2=0;
  
  CU_ASSERT_EQUAL_FATAL( mem_init(), 0);
  CU_ASSERT_PTR_NOT_NULL_FATAL( m1 = mem_alloc(HEAP_SIZE) );
  CU_ASSERT_EQUAL_FATAL( mem_free(m1, HEAP_SIZE), 0);

  CU_ASSERT_NOT_EQUAL_FATAL( mem_free( (void *)-1, HEAP_SIZE), 0 );
  m2 =  m1 - 1 ;
  CU_ASSERT_NOT_EQUAL_FATAL( mem_free( m2, HEAP_SIZE), 0 );
  m2 =  m1 + HEAP_SIZE + 1;
  CU_ASSERT_NOT_EQUAL_FATAL( mem_free( m2, HEAP_SIZE), 0 );

  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
}

void test_bound()
{
  void *mref=0, *m1=0, *m2=0;

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );

  CU_ASSERT_PTR_NOT_NULL_FATAL( mref = mem_alloc(HEAP_SIZE) );
  memset(mref, 6, HEAP_SIZE);
  CU_ASSERT_EQUAL_FATAL( mem_free( mref, HEAP_SIZE ), 0);

  CU_ASSERT_PTR_NOT_NULL_FATAL( m1 = mem_alloc(64) );
  memset(m1, 6, 64);
  CU_ASSERT_PTR_NOT_NULL_FATAL( m2 = mem_alloc(64) );
  memset(m2, 6, 64);

  CU_ASSERT_EQUAL_FATAL( mem_free( m1, 64 ), 0 );
  CU_ASSERT_EQUAL_FATAL( mem_free( m2, 64 ), 0 );

  CU_ASSERT_FATAL( m1 >= mref );
  CU_ASSERT_FATAL( m1 < mref+HEAP_SIZE );
  CU_ASSERT_FATAL( m2 >= mref );
  CU_ASSERT_FATAL( m2 < mref+HEAP_SIZE );
      
  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);

}

void test_alloc_free_list()
{
  int nb=2*50;
  void *tab[nb];
  int i;

  CU_ASSERT( 64*nb < HEAP_SIZE );
 
  CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );
 
  for(i=0; i < nb; i++)
    {
      CU_ASSERT_PTR_NOT_NULL_FATAL( tab[i] = mem_alloc(64) );
      memset(tab[i], 3, 64);
    }

  for(i=0; i < nb; i+=2)
    {
      CU_ASSERT_EQUAL_FATAL( mem_free( tab[i], 64 ), 0 );
    }

  for(i=1; i < nb; i+=2)
    {
      CU_ASSERT_EQUAL_FATAL( mem_free( tab[i], 64 ), 0 );
    }

  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[0] = mem_alloc(HEAP_SIZE) );
  memset(tab[0], 4, HEAP_SIZE);
  CU_ASSERT_EQUAL_FATAL( mem_free( tab[0], HEAP_SIZE ), 0);
      
  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
  
}

void test_small()
{
  void *m1, *m2, *m3;
  int multi = 0;
  
#ifdef BUDDY
  multi=1;
#endif
#ifdef CFF
  multi=2;
#endif
#ifdef BF
  multi=2;
#endif

  CU_ASSERT(multi > 0);

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );
  CU_ASSERT_PTR_NOT_NULL_FATAL( m1 = mem_alloc(1) );
  memset( m1, 0, 1);

  CU_ASSERT_PTR_NOT_NULL_FATAL( m2 = mem_alloc(1) );
  memset( m2, 0, 1);

  if (m2 > m1)
    { CU_ASSERT_FATAL( (m2-m1) >= multi*sizeof(void*) ); }
  else
    { CU_ASSERT_FATAL( (m1-m2) >= multi*sizeof(void*) ); }

  CU_ASSERT_EQUAL_FATAL( mem_free( m1, 1 ), 0 );
  CU_ASSERT_EQUAL_FATAL( mem_free( m2, 1 ), 0 );
  

  CU_ASSERT_PTR_NOT_NULL_FATAL( m3 = mem_alloc(HEAP_SIZE) );
  memset(m3, 4, HEAP_SIZE);
  CU_ASSERT_EQUAL_FATAL( mem_free( m3, HEAP_SIZE ), 0);

  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
}

void test_random()
{
  int i;
  void *mref;

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0);
  CU_ASSERT_PTR_NOT_NULL_FATAL( mref = mem_alloc(HEAP_SIZE) );
  memset(mref, 4, HEAP_SIZE);
  CU_ASSERT_EQUAL_FATAL( mem_free( mref, HEAP_SIZE ), 0);

  for(i=0; i< 10; i++)
    {
      void *m1;
      random_run_cpp(100, false);
      CU_ASSERT_PTR_NOT_NULL_FATAL( m1 = mem_alloc(HEAP_SIZE) );
      memset(m1, 4, HEAP_SIZE);
      CU_ASSERT_EQUAL_FATAL( mem_free( m1, HEAP_SIZE ), 0);
    }

  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);  
}
