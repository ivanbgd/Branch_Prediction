#define BRANCH_PREDICTION
#ifdef BRANCH_PREDICTION

/* Why is it faster to process a sorted array than an unsorted array?
http://stackoverflow.com/questions/11227809/why-is-it-faster-to-process-a-sorted-array-than-an-unsorted-array */

/* A general rule of thumb is to avoid data-dependent branching in critical loops. (such as in this example) */

/* CPU: Intel Core i3-540; Compiler: MSVC++ 2015
My results show that branch-less versions are generally slower.
That probably depends on both CPU and compiler.
They generate more instructions, as can be seen in the comments
below in the code at appropriate places.
It is said that Intel's CPUs have a great Branch predictor unit.
Even sorting doesn't always help, and it sometimes even slows
the program down. That probably means that branch prediction
really works well in Intel, at least in this particular problem. */

#include <algorithm>
#include <ctime>
#include <iostream>

#define LOOP 10000                                             // Default: 100000

void branchRandom(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // Test
    clock_t start = clock();
    long long sum = 0;

    for (unsigned i = 0; i < LOOP; ++i) {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c) {
            if (data[c] >= 128)
                sum += data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchSorted(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;

    /*  Optimization disabled:

                if (data[c] >= 128)
    012D4193  mov         eax,dword ptr [ebp-20024h]  
    012D4199  cmp         dword ptr data[eax*4],80h  
    012D41A4  jl          branchSorted+11Ch (012D41CCh)
                sum += data[c];
    012D41A6  mov         eax,dword ptr [ebp-20024h]
    012D41AC  mov         eax,dword ptr data[eax*4]
    012D41B3  cdq
    012D41B4  add         eax,dword ptr [sum]
    012D41BA  adc         edx,dword ptr [ebp-20018h]
    012D41C0  mov         dword ptr [sum],eax
    012D41C6  mov         dword ptr [ebp-20018h],edx


        /O2 & /Ot:

                if (data[c] >= 128)
    00BA51F0  cmp         ebx,80h
    00BA51F6  jl          branchSorted+0BFh (0BA520Fh)
                sum += data[c];
    00BA51F8  mov         eax,ebx
    00BA51FA  cdq
    00BA51FB  add         edi,eax
    00BA51FD  adc         esi,edx
    00BA51FF  add         edi,eax
    00BA5201  adc         esi,edx
    00BA5203  add         edi,eax
    00BA5205  adc         esi,edx
                sum += data[c];
    00BA5207  add         edi,eax
    00BA5209  adc         esi,edx
    00BA520B  add         edi,eax
    00BA520D  adc         esi,edx
    */
    for (unsigned i = 0; i < LOOP; ++i) {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c) {
            if (data[c] >= 128)
                sum += data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchlessRandom(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // Test
    clock_t start = clock();
    long long sum = 0;

    for (unsigned i = 0; i < LOOP; ++i) {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c) {
            int t = (data[c] - 128) >> 31;
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchlessSorted(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;

    /*  Optimization disabled:

                int t = (data[c] - 128) >> 31;
    012D44F6  mov         eax,dword ptr [ebp-2001Ch]  
    012D44FC  mov         ecx,dword ptr data[eax*4]  
    012D4503  sub         ecx,80h  
    012D4509  sar         ecx,1Fh  
    012D450C  mov         dword ptr [ebp-20020h],ecx  
                sum += ~t & data[c];
    012D4512  mov         eax,dword ptr [ebp-20020h]  
    012D4518  not         eax  
    012D451A  mov         ecx,dword ptr [ebp-2001Ch]  
    012D4520  and         eax,dword ptr data[ecx*4]  
    012D4527  cdq  
    012D4528  add         eax,dword ptr [sum]  
    012D452E  adc         edx,dword ptr [ebp-20010h]  
    012D4534  mov         dword ptr [sum],eax  
    012D453A  mov         dword ptr [ebp-20010h],edx


        /O2 & /Ot:

    00BA55B0  mov         ecx,dword ptr data[eax*4]
    00BA55B7  xorps       xmm0,xmm0
    00BA55BA  movlpd      qword ptr [ebp-2001Ch],xmm0
    00BA55C2  lea         eax,[ecx-80h]
    00BA55C5  sar         eax,1Fh
    00BA55C8  not         eax
    00BA55CA  and         eax,ecx
    00BA55CC  mov         ecx,dword ptr [ebp-20018h]
    00BA55D2  cdq
    00BA55D3  mov         ebx,eax
    00BA55D5  mov         dword ptr [ebp-20008h],edx
    00BA55DB  mov         eax,dword ptr [ebp-2001Ch]
    00BA55E1  mov         edx,1388h
                int t = (data[c] - 128) >> 31;
                sum += ~t & data[c];
    00BA55E6  add         edi,ebx
    00BA55E8  adc         esi,dword ptr [ebp-20008h]
    00BA55EE  add         eax,ebx
    00BA55F0  adc         ecx,dword ptr [ebp-20008h]
    00BA55F6  sub         edx,1
    00BA55F9  jne         branchlessSorted+0C6h (0BA55E6h)
    */
    for (unsigned i = 0; i < LOOP; ++i) {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c) {
            int t = (data[c] - 128) >> 31;
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

/* To avoid the shifting hack you could write something like int t=-((data[c]>=128)) to generate the mask.
This should be faster too.
The result is different than in other functions, though. */
void branchlessSortedAlt(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;

    /*  Optimization disabled:

                int t = -(data[c] >= 128);
    012D4A57  mov         eax,dword ptr [ebp-20024h]  
    012D4A5D  cmp         dword ptr data[eax*4],80h  
    012D4A68  jl          branchlessSortedAlt+106h (012D4A76h)  
    012D4A6A  mov         dword ptr [ebp-20234h],1  
    012D4A74  jmp         branchlessSortedAlt+110h (012D4A80h)  
    012D4A76  mov         dword ptr [ebp-20234h],0  
    012D4A80  mov         ecx,dword ptr [ebp-20234h]  
    012D4A86  neg         ecx  
    012D4A88  mov         dword ptr [ebp-20028h],ecx  
                sum += ~t & data[c];
    012D4A8E  mov         eax,dword ptr [ebp-20028h]  
    012D4A94  not         eax  
    012D4A96  mov         ecx,dword ptr [ebp-20024h]  
    012D4A9C  and         eax,dword ptr data[ecx*4]  
    012D4AA3  cdq  
    012D4AA4  add         eax,dword ptr [sum]  
    012D4AAA  adc         edx,dword ptr [ebp-20018h]  
    012D4AB0  mov         dword ptr [sum],eax  
    012D4AB6  mov         dword ptr [ebp-20018h],edx


        /O2 & /Ot:

                int t = -(data[c] >= 128);
    00BAA550  mov         ecx,dword ptr data[eax*4]
    00BAA557  xorps       xmm0,xmm0
    00BAA55A  xor         eax,eax
    00BAA55C  movlpd      qword ptr [ebp-2001Ch],xmm0
    00BAA564  cmp         ecx,80h
    00BAA56A  setge       al
    00BAA56D  dec         eax
    00BAA56E  and         eax,ecx
    00BAA570  mov         ecx,dword ptr [ebp-20018h]
    00BAA576  cdq
    00BAA577  mov         ebx,eax
    00BAA579  mov         dword ptr [ebp-20008h],edx
    00BAA57F  mov         eax,dword ptr [ebp-2001Ch]
    00BAA585  mov         edx,1388h
    00BAA58A  nop         word ptr [eax+eax]
                sum += ~t & data[c];
    00BAA590  add         edi,ebx
    00BAA592  adc         esi,dword ptr [ebp-20008h]
    00BAA598  add         eax,ebx
    00BAA59A  adc         ecx,dword ptr [ebp-20008h]
    00BAA5A0  sub         edx,1
    00BAA5A3  jne         branchlessSortedAlt+0D0h (0BAA590h)
    */
    for (unsigned i = 0; i < LOOP; ++i) {
        // Primary loop
        for (unsigned c = 0; c < arraySize; ++c) {
            int t = -(data[c] >= 128);
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

/* LOOP INTERCHANGE */

/* Intel Compiler 11 does something miraculous.
It interchanges the two loops, thereby hoisting the unpredictable branch to the outer loop.
So not only is it immune the mispredictions... */

void branchRandomLI(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // Test
    clock_t start = clock();
    long long sum = 0;

    // Primary loop
    for (unsigned c = 0; c < arraySize; ++c) {
        for (unsigned i = 0; i < LOOP; ++i) {
            if (data[c] >= 128)
                sum += data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchSortedLI(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;

    // Primary loop
    for (unsigned c = 0; c < arraySize; ++c) {
        for (unsigned i = 0; i < LOOP; ++i) {
            if (data[c] >= 128)
                sum += data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchlessRandomLI(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // Test
    clock_t start = clock();
    long long sum = 0;

    // Primary loop
    for (unsigned c = 0; c < arraySize; ++c) {
        for (unsigned i = 0; i < LOOP; ++i) {
            int t = (data[c] - 128) >> 31;
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

void branchlessSortedLI(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;
    
    // Primary loop
    for (unsigned c = 0; c < arraySize; ++c) {
        for (unsigned i = 0; i < LOOP; ++i) {
            int t = (data[c] - 128) >> 31;
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}

/* To avoid the shifting hack you could write something like int t=-((data[c]>=128)) to generate the mask.
This should be faster too.
The result is different than in other functions, though. */
void branchlessSortedAltLI(void) {
    // Generate data
    const unsigned arraySize = 32768;
    int data[arraySize];

    for (unsigned c = 0; c < arraySize; ++c)
        data[c] = std::rand() % 256;

    // !!! With this, the next loop runs faster
    std::sort(data, data + arraySize);

    // Test
    clock_t start = clock();
    long long sum = 0;

    // Primary loop
    for (unsigned c = 0; c < arraySize; ++c) {
        for (unsigned i = 0; i < LOOP; ++i) {
            int t = -(data[c] >= 128);
            sum += ~t & data[c];
        }
    }

    double elapsedTime = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;

    std::cout << elapsedTime << std::endl;
    std::cout << "sum = " << sum << std::endl << std::endl;
}


int main() {
    branchRandom();
    branchSorted();
    branchlessRandom();
    branchlessSorted();
    branchlessSortedAlt();

    std::cout << std::endl << "LOOP INTERCHANGE\n" << std::endl;
    
    branchRandomLI();
    branchSortedLI();
    branchlessRandomLI();
    branchlessSortedLI();
    branchlessSortedAltLI();

    //std::cout << 0 << " " << ~0 << std::endl;
    //printf_s("%x %x\n", 0, ~0);
    //printf_s("%x %x\n", 1, ~1);

    char c;
    std::cin >> c;
}

#endif // BRANCH_PREDICTION 