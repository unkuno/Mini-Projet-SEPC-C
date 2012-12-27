#include <CUnit/CUnit.h>

#include "../mem.h"

void test_bf()
{
  int multi = 0;
  int i;
  int S8 = HEAP_SIZE/8;
  void *tab[8]={};
  
#ifndef BF
  return;
#else
  multi=2;
#endif

  CU_ASSERT_FATAL(multi > 0);

  mem_init();
  
  for(i=0; i < 6; i++)
    {
      CU_ASSERT_PTR_NOT_NULL_FATAL( tab[i] = mem_alloc(S8) );
      memset(tab[i], 3, S8);
    }
  int decal = multi*sizeof(void*);
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[6] = mem_alloc(S8-decal) );
  memset(tab[6], 3, S8-decal);
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[7] = mem_alloc(S8+decal) );
  memset(tab[6], 3, S8+decal);
    

  for(i=0; i < 6; i+= 2)
    mem_free( tab[i], S8 );
  mem_free(tab[6], S8 - decal);
  
  CU_ASSERT_PTR_NULL_FATAL( mem_alloc(3*S8) );
  
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[6] = mem_alloc(S8-decal) );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[0] = mem_alloc(S8) );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[2] = mem_alloc(S8) );
  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[4] = mem_alloc(S8) );

  
  for(i=0; i < 6; i++)
    mem_free( tab[i], S8 );

  mem_free(tab[7], S8+decal);
  mem_free(tab[6], S8-decal);

  CU_ASSERT_PTR_NOT_NULL_FATAL( tab[0] = mem_alloc(HEAP_SIZE) );
  memset(tab[0], 3, HEAP_SIZE);

  mem_free(tab[0], HEAP_SIZE);

}
