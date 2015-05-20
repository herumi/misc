#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>

namespace std {
	template<class T> struct _Unique_if {
		typedef unique_ptr<T> _Single_object;
	};

	template<class T> struct _Unique_if<T[]> {
		typedef unique_ptr<T[]> _Unknown_bound;
	};

	template<class T, size_t N> struct _Unique_if<T[N]> {
		typedef void _Known_bound;
	};

	template<class T, class... Args>
		typename _Unique_if<T>::_Single_object
		make_unique(Args&&... args) {
			return unique_ptr<T>(new T(std::forward<Args>(args)...));
		}

	template<class T>
		typename _Unique_if<T>::_Unknown_bound
		make_unique(size_t n) {
			typedef typename remove_extent<T>::type U;
			return unique_ptr<T>(new U[n]());
		}

	template<class T, class... Args>
		typename _Unique_if<T>::_Known_bound
		make_unique(Args&&...) = delete;
}

#include <iostream>
#include <string>
using namespace std;

int main() {
	cout << *make_unique<int>() << endl;
	cout << *make_unique<int>(1729) << endl;
	cout << "\"" << *make_unique<string>() << "\"" << endl;
	cout << "\"" << *make_unique<string>("meow") << "\"" << endl;
	cout << "\"" << *make_unique<string>(6, 'z') << "\"" << endl;

	auto up = make_unique<int[]>(5);

	for (int i = 0; i < 5; ++i) {
		cout << up[i] << " ";
	}

	cout << endl;

#if defined(ERROR1)
	auto up1 = make_unique<string[]>("error");
#elif defined(ERROR2)
	auto up2 = make_unique<int[]>(10, 20, 30, 40);
#elif defined(ERROR3)
	auto up3 = make_unique<int[5]>();
#elif defined(ERROR4)
	auto up4 = make_unique<int[5]>(11, 22, 33, 44, 55);
#endif
}
