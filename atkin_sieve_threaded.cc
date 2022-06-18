#include <future>
#include <math.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

using namespace std;

void first_loop(
    shared_ptr<bool[]> &sieve, size_t limit, size_t n_min, size_t n_max
) {
    size_t x = 1;
    size_t x2 = 1;
    while (x2 <= limit) {
        const auto _3x2 = 3*x2;
        const auto _4x2 = 4*x2;
        size_t y = 1;
        size_t y2 = 1;
        /*
        if (_4x2 > n_min) {
            y2 = _4x2 - n_min;
            y = sqrt(y2);
        }
        size_t y2_stop = 0;
        if (n_max > _3x2) {
            y2_stop = n_max - _3x2;
        }
        */
        while (y2 <= n_max) {
            auto n2 = _3x2 + y2;
            if (n2 >= n_min && n2 <= n_max  && n2 % 12 == 7) {
                auto i = (n2 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            auto n1 = _4x2 + y2;
            if (n1 >= n_min && n1 <= n_max) {
                auto m1 = n1 % 12;
                if (m1 == 1 || m1 == 5) {
                    auto i = (n1 - 5)/2;
                    sieve[i] = ! sieve[i];
                }
            }
            ++y;
            y2 = y*y;
        }
        auto z = x;
        while (z >= 1) {
            auto n3 = _3x2 - z*z;
            if (n3 > n_max) {
                break;
            }
            if (n3 >= n_min && n3 <= n_max && n3 % 12 == 11) {
                auto i = (n3 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            --z;
        }
        ++x;
        x2 = x*x;
    }
}

void second_loop(
    shared_ptr<bool []> sieve, long long limit, long long n_min, long long n_max
) {
    auto r = max((size_t)5, (size_t)sqrt(n_min));
    if (r % 2 == 0) {
        // r must be odd
        ++r;
    }
    auto r2 = r*r;
    while (r2 <= min(n_max, limit)) {
        auto i = (r - 5)/2;
        if (sieve[i]) {
            for (auto j=r2; j<=limit; j+=2*r2) {
                auto k = (j - 5)/2;
                sieve[k] = false;
            }
        }
        r += 2;
        r2 = r*r;
    }
}

vector<size_t> sieveOfAtkin(size_t limit, uint nthreads) {
    auto off = limit % 2 == 0 ? 5 : 6;
    auto size = (limit - off)/2 + 1; 
    // vector<bool> sieve(size, false);
    // values initialized to 0, which should be false in the bool case
    shared_ptr<bool[]> sieve(new bool[size], std::default_delete<bool[]>());
    vector<thread> threads(nthreads);
    vector<uint> n_min(nthreads, 0);
    vector<uint> n_max(nthreads, 0);
    uint n_nom = int((limit - 4)/nthreads) - 1;
    uint mod = (limit - 4) % nthreads;
    for (uint i = 0; i < nthreads; ++i) {
        n_min[i] = i == 0 ? off : n_max[i-1] + 1;
        n_max[i] = n_min[i] + n_nom;
        if (i < mod) {
            ++n_max[i];
        }
        // cout << " i " << i << " n_min " << n_min[i] << " nmax " << n_max[i] << endl;
        threads[i] = thread(&first_loop, std::ref(sieve), limit, n_min[i], n_max[i]);
    }
    for (uint i=0; i<nthreads; ++i) {
        threads[i].join();
    }
    //cout << "begin z loop" << endl;
    /*
    size_t x = 1;
    size_t x2 = 1;
    while (x2 <= limit) {
        auto z = x;
        while (z >= 1) {
            auto n3 = 3*x2 - z*z;
            if (n3 > limit) {
                break;
            }
            if (n3 % 12 == 11) {
                auto i = (n3 - 5)/2;
                sieve[i] = ! sieve[i];
            }
            --z;
        }
        ++x;
        x2 = x*x;
    }
    */
    for (uint i = 0; i < nthreads; ++i) {
        threads[i] = thread(&second_loop, ref(sieve), limit, n_min[i], n_max[i]);
    }
    for (uint i=0; i<nthreads; ++i) {
        threads[i].join();
    }
    /*
    auto r = off;
    if (r % 2 == 0) {
        // r must be odd
        ++r;
    }
    auto r2 = r*r;
    while (r2 <= limit) {
        auto i = (r - 5)/2;
        if (sieve[i]) {
            for (auto j=r2; j<=limit; j+=2*r2) {
                auto k = (j - 5)/2;
                sieve[k] = false;
            }
        }
        r += 2;
        r2 = r*r;
    }
    */
    vector<size_t> primes;
    size_t num = 5;
    // for (const auto& s: *sieve) {
    for (size_t i=0; i<size; ++i) {
        if (sieve[i]) {
            primes.push_back(num);
        }
        num += 2;
    }
    primes.insert(primes.begin(), 3);
    primes.insert(primes.begin(), 2);
    return primes;
}
    
int main(int argc, char *argv[]) {
    size_t count = 100;
    uint nthreads = 1;
    if (argc >= 2) {
        string x(argv[1]);
        stringstream y(x);
        y >> count;
    }
    if (argc >= 3) {
        string x(argv[2]);
        stringstream y(x);
        y >> nthreads;
    }
    auto primes = sieveOfAtkin(count, nthreads);
    cout << primes.size() << endl;
    
    if (count <= 1000) {
        for (const auto& p: primes) {
            cout << p << endl;
        }
    }
    return 0;
}
