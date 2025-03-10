#include <vector>

#include "common.hpp"

namespace CUDA
{

__global__ void kernel_test()  
{
    // calcul multiprocesseur 
} 
 
void pTest() 
{     
    kernel_test<<<1, 1>>>();
}
}
