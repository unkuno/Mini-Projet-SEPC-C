#include <CUnit/CUnit.h>

#include "../mem.h"

void test_cff()
{
  int multi = 0;
  int i;
  int S8 = HEAP_SIZE/8;
  void *tab[8]={};
  
#ifndef CFF
  return;
#else
  multi=1;
#endif

  CU_ASSERT_FATAL(multi > 0);

  CU_ASSERT_EQUAL_FATAL( mem_init(), 0 );

  for(i=0; i < 8; i++)
    {
      CU_ASSERT_PTR_NOT_NULL_FATAL( tab[i] = mem_alloc(S8) );
      memset(tab[i], 3, S8);
    }

  for(i=1; i < 8; i+= 2)
    CU_ASSERT_EQUAL_FATAL( mem_free( tab[i], S8 ), 0 );

  CU_ASSERT_PTR_NULL_FATAL( mem_alloc(3*S8) );
  CU_ASSERT_EQUAL_FATAL( mem_free( tab[2], S8 ), 0 );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[2] = mem_alloc(3*S8) );
  memset(tab[2], 3, 3*S8);

  CU_ASSERT_EQUAL_FATAL( mem_free( tab[6], S8 ), 0 );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[6] = mem_alloc(3*S8) );
   memset(tab[6], 3, 3*S8);

  CU_ASSERT_EQUAL_FATAL( mem_free( tab[2], 3*S8 ), 0 );
  CU_ASSERT_EQUAL_FATAL( mem_free( tab[6], 3*S8 ), 0 );
  CU_ASSERT_PTR_NULL_FATAL( tab[2] = mem_alloc(7*S8) );


  CU_ASSERT_EQUAL_FATAL( mem_free( tab[4], S8 ), 0 );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[4] = mem_alloc(7*S8) );
  memset(tab[4], 3, 7*S8);

  CU_ASSERT_EQUAL_FATAL( mem_free( tab[4], 7*S8 ), 0 );
  CU_ASSERT_EQUAL_FATAL( mem_free( tab[0], S8 ), 0 );

  CU_ASSERT_EQUAL_FATAL( mem_destroy(), 0);
}
