// Compile me: g++ -std=c++1z main.cpp -o test
#include <iostream>
#include <iterator>
#include <vector>

#include <boost/format.hpp>
#include <boost/range/irange.hpp>

template<class Size>
std::tuple<Size> make_cum_product(Size size)
{
    return std::make_tuple(size);
}

template<class Size, class... Sizes>
std::tuple<Sizes..., Size, Size> make_cum_product(Size product, Size second, Sizes... tail)
{
    return std::tuple_cat(std::make_tuple(product), make_cum_product(second * product, tail...));
}

template<class Functor, class RangesTuple, class TupleSizes, class TupleCumProd, std::size_t... Is>
void dereference(Functor f, std::size_t index,
                 const RangesTuple& ranges, const TupleSizes& sizes, const TupleCumProd& cum_prod,
                 std::index_sequence<Is...>)
{
    f(std::get<Is>(ranges)[(index / std::get<Is>(cum_prod)) % std::get<Is>(sizes)]...);
}

template<class Functor, class... Ranges>
void MultipleForLoop(Functor f, const Ranges&... rs)
{
    auto sizes = std::make_tuple(std::size(rs)...);
    auto cum_prod = make_cum_product(std::size_t(1), std::size(rs)...);
    auto size = std::get<sizeof...(Ranges)>(cum_prod);
    auto ranges = std::make_tuple(rs...);

    for (std::size_t i = 0; i < size; ++i)
        dereference(f, i, ranges, sizes, cum_prod, std::index_sequence_for<Ranges...>());
}

void func(int i, int j, int k)
{
    std::cout << boost::format("Received (%1%, %2%, %3%)\n") % i % j % k;
}

int main(int argc, char** argv)
{
    MultipleForLoop(func, boost::irange(0, 3), std::vector<int>({-2, 0, 1}), std::vector<int>({10, 11, 12, 13}));
    return 0;
}
