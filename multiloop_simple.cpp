// Compile me: g++ -std=c++14 main.cpp -o test
#include <iostream>
#include <list>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/irange.hpp>

template<class Functor, class Range, class... Ranges>
void MultipleForLoop(Functor f, const Range& r, const Ranges&... rs)
{
    for (auto x : r)
    {
        // https://en.wikipedia.org/wiki/Currying
        auto func = [&f, x](auto&... xs) {
            f(x, xs...);
        };
        MultipleForLoop(func, rs...);
    }
}

// https://en.wikipedia.org/wiki/Tail_call
template<class Functor, class Range>
void MultipleForLoop(Functor f, const Range& r)
{
    for (auto x : r)
        f(x);
}

void func(int i, int j, int k)
{
    std::cout << boost::format("Received (%1%, %2%, %3%)\n") % i % j % k;
}

int main(int argc, char** argv)
{
    MultipleForLoop(func, boost::irange(0, 3), std::vector<int>({-2, 0, 1}), std::list<int>({10, 11, 12, 13}));
    return 0;
}
