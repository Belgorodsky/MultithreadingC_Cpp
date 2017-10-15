#include <iostream>
#include <set>
#include <unordered_set>

#if 0
struct comparator
{
	bool operator()(const int &a, const int &b) const {
		return b < a;
	};
};
#endif

struct hasher {

	size_t operator()(const int &a) const {
		return a%2;
	}
};


int main (int argc, char ** argv)
{
	std::unordered_set<int,hasher> s;
	s.insert(1);
	s.insert(3);
	s.insert(2);
	s.insert(5);
	s.insert(4);

	for(auto i = s.begin(); i != s.end(); ++i)
	{
		std::cout << *i << ' ';
	}
	std::cout << std::endl;
	return 0;
} 
