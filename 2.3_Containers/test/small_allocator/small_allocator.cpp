extern "C"
{
#include <sys/types.h>
#include <sys/mman.h>
}

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstdio>

constexpr size_t OLD_SIZES_COUNT = 1 + UINT16_MAX;
class SmallAllocator {
public:
	SmallAllocator() : old_sizes_id(0) {
		for(size_t i= 0; i < OLD_SIZES_COUNT; ++i)
			old_sizes[0]= 0;
	}

        void *Alloc(size_t size) {
		void* p = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
		if(isBadPrt(p))
		{
			p = nullptr;
		}
		else
		{
			old_sizes[old_sizes_id] = size;
			size_t old_size = old_sizes[old_sizes_id];
			++old_sizes_id;
		}
		return p;
        };
        void *ReAlloc(void *old, size_t size) {
		void* p = nullptr;
		size_t old_size = old_sizes[old_sizes_id - 1];
#if defined(__APPLE__) && defined(__MACH__)
		if(Free(old) < 0 )
		{
			printf("bad free of %zu\n", old_size);
			return p;
		}
		p = Alloc(size);
#else
		p = mremap(old, old_size, size, MREMAP_MAYMOVE, old);
#endif
		if(isBadPrt(p))
		{
			p = nullptr;
		}
		else 
		{
			old_sizes[old_sizes_id] = size;
		}
		return p;
        };
        int Free(void *old ) {
		--old_sizes_id;
		size_t old_size = old_sizes[old_sizes_id];
		int ec = 0;
		if(old) munmap(old,old_size);
		return ec;
        };
private:
	inline bool isBadPrt(void* p)
	{
		return p == MAP_FAILED;
	}

	size_t old_sizes[OLD_SIZES_COUNT];
	uint8_t old_sizes_id;
};

int main()
{
	SmallAllocator A1;
	int * A1_P1 = (int *) A1.Alloc(sizeof(int));
	A1_P1 = (int *) A1.ReAlloc(A1_P1, 2 * sizeof(int));
	A1.Free(A1_P1);
	SmallAllocator A2;
	int * A2_P1 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 1" << std::endl;
	int * A2_P2 = (int *) A2.Alloc(10 * sizeof(int));
	for(unsigned int i = 0; i < 10; i++) A2_P2[i] = -1;
	for(unsigned int i = 0; i < 10; i++) if(A2_P1[i] != i) std::cout << "ERROR 2" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 3" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 20 * sizeof(int));
	for(unsigned int i = 10; i < 20; i++) A2_P1[i] = i;
	for(unsigned int i = 0; i < 20; i++) if(A2_P1[i] != i) std::cout << "ERROR 4" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 5" << std::endl;
	A2_P1 = (int *) A2.ReAlloc(A2_P1, 5 * sizeof(int));
	for(unsigned int i = 0; i < 5; i++) if(A2_P1[i] != i) std::cout << "ERROR 6" << std::endl;
	for(unsigned int i = 0; i < 10; i++) if(A2_P2[i] != -1) std::cout << "ERROR 7" << std::endl;
	A2.Free(A2_P1);
	A2.Free(A2_P2);
	std::cout << "ok" << std::endl;
	return 0;
}
