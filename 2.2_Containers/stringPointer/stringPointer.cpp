#include <iostream>
#include <string>
#include <set>

class StringPointer {
public:
	StringPointer(std::string* p) : pointer(p),extraAlloc(false) {
		if(p == nullptr) {
			extraAlloc = true;
			pointer = new std::string;
		}
	}
	operator std::string*() { 
		return pointer; 
	}
	std::string* operator->() { 
		return pointer; 
	}
    
	~StringPointer() {
		if(extraAlloc) delete pointer;
	
	} 
private:
	std::string* pointer;
	bool extraAlloc;
};

int main()
{
	std::string s1 = "Hello, world!";

	StringPointer sp1(&s1);
	StringPointer sp2(NULL);

	std::cout << sp1->length() << std::endl;
	std::cout << *sp1 << std::endl;
	std::cout << sp2->length() << std::endl;
	std::cout << *sp2 << std::endl;
	return 0;
}
