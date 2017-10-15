#include <iostream>

template<class T>
class SmartPointer {
public:
	SmartPointer(T* p) : pointer(p) {}
	operator T*() { return pointer; }
	T* operator->() { 
		if(!pointer) {
			pointer = new T();
			std::cerr << "Bad pointer!" << std::endl;
		}
		return pointer; 
	}
	std::ptrdiff_t operator-(SmartPointer<T> p)
	{
		return pointer - p;
	}
	std::ptrdiff_t operator-(void* p)
	{
		return pointer - p;
	}
private:
	T* pointer;
};

class Foo {
private:
	int a, b;
public:
	Foo() : a(0), b(0) {}
	Foo(int a, int b) : a(a), b(b) {}
	int Sum() { return a + b; }
};

int main()
{
	SmartPointer<Foo> sp(nullptr);
	std::cout << sp->Sum() << std::endl;
	return 0;
}
