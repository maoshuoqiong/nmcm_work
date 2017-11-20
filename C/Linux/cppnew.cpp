#include <iostream>

int main(int argc, const char * argv[])
{
	auto a = 1, &b = a;
	auto c = b;
	c++;
	std::cout<<a<<","<<b<<","<<c<<std::endl;
	return 0;
}

