#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <cstring>
//typedef long int64_t;
//typedef unsigned long long uint64_t;

using namespace std;
struct foo
{
	foo() {}
	void m() { cout << "no const" << endl; }
	void m() const { cout << "const" << endl; }
	int a();
};

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type
is_odd(T t) {
	return bool(t % 2);
}

class id {
public:
	id() {};
};

class boo {
	boo(int x){}
	void bo1() {
		cout << "bo1" << endl;
	}
};
typedef boo* (*moo)(int);

struct key
{
	key() : type_info_(0), id_(0) {}
	const std::type_info* type_info_;   //typeid(int)返回的就是std::type_info了
	const id* id_;   //这个struct里面还有一个执行上下文下面的id类的实例的指针
} key_;   //service里的结构体key

int main() {
	foo{}.m();
	std::add_const<foo>::type{}.m();
	cout << std::alignment_of<foo>::value << endl;
	decltype(std::declval<foo>().a()) a{1};
	cout << is_odd(3) << endl;
	auto l = []() {cout << "lambda" << endl; };
	bool b = std::is_constructible<foo>::value;
	cout << b << endl;
	~std::size_t(1);
	auto k1 = key();
	auto k2 = key();
	cout << (k1.type_info_ == k2.type_info_) << endl;
	cout << (k1.id_ == k2.id_) << endl;
	moo m;
	m(1);
	const int a = 1;
	auto b = static_cast<int&&>(a);
}