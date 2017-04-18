// Compile me:  g++ -std=c++11 main.cpp -o test -ldl -Wno-main
#include <iostream>
#include <vector>
#include <string>

#include <boost/format.hpp>

#include <dlfcn.h>

using namespace std;

typedef int libc_start_main_f(...);

extern "C" int __libc_start_main(int (*main)(int, char **, char **), long argc, char** ubp_av,
                                 void (*init), void (*fini), void (*rtld_fini), void (*stack_end))
{
    static vector<string> args(ubp_av, ubp_av + argc);
    auto real_libc_start_main = reinterpret_cast<libc_start_main_f*>(dlsym(RTLD_NEXT, "__libc_start_main"));
    return real_libc_start_main(main, (long)&args, ubp_av, init, fini, rtld_fini, stack_end);
}

int main(const vector<string>& args)
{
    cout << "argc = " << args.size() << endl;
    for (size_t i = 0, e = args.size(); i < e; ++i)
        cout << boost::format("arg #%1% = %2%\n") % i % args[i];

    return 0;
}
