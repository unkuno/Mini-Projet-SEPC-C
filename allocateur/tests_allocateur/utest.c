#include <CUnit/CUnit.h>
#include <CUnit/Basic.h> 

#include "../mem.h"
#include "test_generic.h"
#include "test_buddy.h"
#include "test_cff.h"
#include "test_bf.h"

int
main(int argc, char **argv)
{
  CU_pSuite sg;
#ifdef BUDDY
  CU_pSuite sb;
#endif 
#ifdef CFF
  CU_pSuite sf;
#endif 
#ifdef BF
  CU_pSuite sb;
#endif


  CU_initialize_registry(); 

  
  sg = CU_add_suite("generic test", 0, 0);
  CU_ADD_TEST(sg, test_init); 
  CU_ADD_TEST(sg, test_alloc_free);
  CU_ADD_TEST(sg, test_bound);
  CU_ADD_TEST(sg, test_alloc_free_list);
  CU_ADD_TEST(sg, test_free_boundaries);
  CU_ADD_TEST(sg, test_small);
  CU_ADD_TEST(sg, test_random);
	
#ifdef BUDDY
  sb = CU_add_suite("buddy test", 0, 0);
  CU_ADD_TEST(sb, test_buddy);
#endif

#ifdef CFF
  sf = CU_add_suite("cff test", 0, 0);
  CU_ADD_TEST(sf, test_cff);
#endif

#ifdef BF
  sb = CU_add_suite("bf test", 0, 0);
  CU_ADD_TEST(sb, test_bf);
#endif

  CU_basic_run_tests();
  CU_cleanup_registry();

  return 0;
}

