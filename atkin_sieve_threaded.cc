#include <iostream>
#include <vector>
#include <sstream>
#include <thread>
#include <future>
#include <set>
#include <chrono>

using namespace std;

void test(string msg) {
    cout << msg << endl;
}

void print(const set<long long>& s) {
    for (const auto& p: s) {
        cout << p << " ";
    }
    cout << endl;
}

void print(const vector<long long>& s) {
    for (const auto& p: s) {
        cout << p << " ";
    }
    cout << endl;
}

void compute(
    promise<set<long long>> && p, long long limit, uint threadID,
    uint nthreads
) {
    set<long long> firstLoopIndices;
    long long x = threadID + 1;
    long long x2 = x*x;
    while (x2 <= limit) {
        long long y = 1;
        long long y2 = 1;
        while (y2 <= limit) {
            long long n2 = 3 * x2 + y2;
            if (n2 > limit) {
                break;
            }
            long long n1 = 4 * x2 + y2;
            long long m1 = n1 % 12;
            if (n1 <= limit && (m1 == 1 || m1 == 5)) {
                auto i = (n1 - 5) / 2;
                if (firstLoopIndices.erase(i) == 0) {
                    firstLoopIndices.insert(i);
                }
            }
            if (n2 % 12 == 7) {
                auto i = (n2 - 5) / 2;
                if (firstLoopIndices.erase(i) == 0) {
                    firstLoopIndices.insert(i);
                }
            }
            ++y;
            y2 = y * y;
        }
        long long z = x;
        while (z >= 1) {
            long long n3 = 3 * x2 - z * z;
            if (n3 > limit) {
                break;
            }
            if (n3 % 12 == 11) {
                auto i = (n3 - 5) / 2;
                if (firstLoopIndices.erase(i) == 0) {
                    firstLoopIndices.insert(i);
                }
            }
            --z;
        }
        x += nthreads;
        x2 = x * x;
    }
    p.set_value(firstLoopIndices);
}

void computeExcludes(
    promise<set<long long>> && p, const set<long long>& primeCandidateIndices,
    long long limit, uint threadID, uint nthreads
) {
    // exclude contains the indices, not the values, that need to be
    // excluded
    set<long long> exclude;
    long long r = 5 + 2*threadID;
    long long r2 = r * r;
    auto end = primeCandidateIndices.end();
    while (r2 <= limit) {
        // auto i = (r - 5) / 2;
        if (primeCandidateIndices.find((r - 5)/2) != end) {
            for (long long j = r2; j <= limit; j += 2 * r2) {
                auto k = (j - 5)/2;
                exclude.insert((j - 5)/2);
            }
        }
        r += 2*nthreads;
        r2 = r * r;
    }
    p.set_value(exclude);
}

set<long long> sieveOfAtkin(long long limit, int nthreads) {
    chrono::time_point<chrono::high_resolution_clock> start
        = chrono::high_resolution_clock::now();
    auto off = limit % 2 == 0 ? 5 : 6;
    auto size = (limit - off) / 2 + 1;
    vector<promise<set<long long>>> promises(nthreads);
    vector<future<set<long long>>> futures(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        futures[i] = promises[i].get_future();
    }
    vector<unique_ptr<thread>> threads(nthreads);
    for (uint i = 0; i < nthreads; ++i) {
        threads[i].reset(
            new thread(&compute, move(promises[i]), limit, i, nthreads)
        );
    }
    // auto sbegin = sieve.begin();
    // auto send = sieve.end();
    set<long long> primeIndices;
    
    vector<set<long long>> initialLoopRes(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        threads[i]->join();
        initialLoopRes[i] = futures[i].get();
    }
    chrono::time_point<chrono::high_resolution_clock> stop
        = chrono::high_resolution_clock::now();
    chrono::duration<double> d = stop - start;
    cout << "duration 1 " << d.count() << endl;


    set<long long> primeCandidateIndices;
    for (uint i=0; i<nthreads; ++i) {
        auto& target = initialLoopRes[i];
        for (auto iter=target.begin(); iter!=target.end(); ++iter) {
            auto index = *iter;
            uint count = 1;
            for (uint j=i+1; j<nthreads; ++j) {
                auto& comp = initialLoopRes[j];
                count += comp.erase(index);
            }
            if (count % 2 == 1) {
                primeCandidateIndices.insert(index);
            }
        }
    }
    
    vector<promise<set<long long>>> promises2(nthreads);
    vector<future<set<long long>>> futures2(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        futures2[i] = promises2[i].get_future();
    }
    for (uint i = 0; i < nthreads; ++i) {
        threads[i].reset(
            new thread(&computeExcludes, move(promises2[i]), primeCandidateIndices,
            limit, i, nthreads)
        );
    }
    vector<set<long long>> excludes(nthreads);
    for (uint i=0; i<nthreads; ++i) {
        threads[i]->join();
        excludes[i] = futures2[i].get();
        for (auto iter=excludes[i].begin(); iter!=excludes[i].end(); ++iter) {
            primeCandidateIndices.erase(*iter);
        } 
    }
    // print(excludes);
    /*
    for (const auto e: excludes) {
        primeIndices.erase(e);
    }
    */
    set<long long> primes;
    primes.insert(2);
    primes.insert(3);
    for (const auto i: primeCandidateIndices) {
        primes.insert(2*i + 5);
    }
    return primes;
}

long long stringToInt(const char *s) {
    string x(s);
    stringstream y(x);
    long long z;
    y >> z;
    return z;
}

int main(int argc, char *argv[]) {
    long long count = 100;
    uint nthreads = 1;
    if (argc >= 2) {
        count = stringToInt(argv[1]);
        if (argc >= 3) {
            nthreads = (int)stringToInt(argv[2]);
        }
    }
    auto primes = sieveOfAtkin(count, nthreads);
    // print(primes);
    cout << "n primes " << primes.size() << endl;
    return 0;
}