
/**
 * @file gen.cpp
 * code : this code refers to https://github.com/ShiZhan/generator/blob/master/src/er.cpp

 */
#include <iostream>
#include <fstream>
#include <functional>
#include <random>
#include <ctime>
#include <stdint.h>
#include <algorithm>
#include <string>
#include <sstream>

class normal_weight_generator_t {
    std::default_random_engine gen;
    std::normal_distribution<float> dist;
    float min_val, max_val;
public:
    normal_weight_generator_t(float minval, float maxval) : dist((minval + maxval) / 2, (maxval - minval) / 6), min_val(minval), max_val(maxval) { }
    float operator()() {
        float sample;
        do {
            sample = dist(gen);
        } while(sample < min_val || sample > max_val);
        return sample;
    }
};

char* getOption(char** begin, char** end, const std::string& option) {
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end) return *itr;
	return 0;
}

bool chkOption(char** begin, char** end, const std::string& option) {
    return std::find(begin, end, option) != end;
}

template <typename T>
T getValue(char** begin, char** end, const std::string& option, T d) {
    T value = d;
    char* str = getOption(begin, end, option);
    if (str) {
        try {
            std::stringstream(str) >> value;
        } catch(std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    return value;
}

template<>
bool getValue<bool>(char** begin, char** end, const std::string& option, bool d) {
    bool value = d;
    value = chkOption(begin, end, option);
    return value;
}

typedef std::function<void(const uint64_t u, const uint64_t v)> output_function;
typedef std::function<void(const uint64_t u, const uint64_t v, const float w)> output_weighted_function;

uint64_t er_gen(int scale, int degree, int seed, output_function o)
{
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<uint64_t> ud(0, 0xffffffffffffffff);

    uint64_t e, r, u, v;
    uint8_t bits_per_rand = 64 / scale, nbits = 0;
    uint64_t mask = (uint64_t)(1 << scale) - 1;
    for (e = 0; e < (uint64_t)(1 << scale) * degree; e++)
    {
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        u = r & mask;
        r >>= scale;
        nbits--;
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        v = r & mask;
        r >>= scale;
        nbits--;
        o(u, v);
    }
    return e;
}

uint64_t er_gen_weighted(int scale, int degree, int seed, output_weighted_function o)
{
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<uint64_t> ud(0, 0xffffffffffffffff);

    std::random_device rd;
    std::mt19937_64 wht_gen(rd());
    std::uniform_real_distribution<float> dist(1.0, 10.0);

    uint64_t e, r, u, v;
    uint8_t bits_per_rand = 64 / scale, nbits = 0;
    uint64_t mask = (uint64_t)(1 << scale) - 1;
    float w;
    for (e = 0; e < (uint64_t)(1 << scale) * degree; e++)
    {
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        u = r & mask;
        r >>= scale;
        nbits--;
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        v = r & mask;
        r >>= scale;
        nbits--;
        w = dist(wht_gen);
        o(u, v, w);
    }
    return e;
}

uint64_t er_gen_normal_weighted(int scale, int degree, int seed, output_weighted_function o)
{
    std::mt19937_64 gen(seed);
    std::uniform_int_distribution<uint64_t> ud(0, 0xffffffffffffffff);

    normal_weight_generator_t dist(1.0, 10.0);

    uint64_t e, r, u, v;
    uint8_t bits_per_rand = 64 / scale, nbits = 0;
    uint64_t mask = (uint64_t)(1 << scale) - 1;
    float w;
    for (e = 0; e < (uint64_t)(1 << scale) * degree; e++)
    {
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        u = r & mask;
        r >>= scale;
        nbits--;
        if (nbits == 0)
        {
            r = ud(gen);
            nbits = bits_per_rand;
        }
        v = r & mask;
        r >>= scale;
        nbits--;
        w = dist();
        o(u, v, w);
    }
    return e;
}

int main(int argc, char *argv[])
{
    using namespace std;

    if (chkOption(argv, argv + argc, "-h"))
    {
        cout
            << "er [options]" << endl
            << " -h:\t ask for help" << endl
            << " -s:\t scale,  default: 8" << endl
            << " -d:\t degree, default: 8" << endl
            << " -r:\t srand,  default: current time" << endl
            << " -w:\t weighted, default: false" << endl
            << " -n:\t normal, require weighted is true, default: false" << endl
            << " -o:\t output, default: console" << endl;
        return 0;
    }

    int scale = getValue(argv, argv + argc, "-s", 8);
    int degree = getValue(argv, argv + argc, "-d", 8);
    int seed = getValue(argv, argv + argc, "-r", time(NULL));
    bool weighted = getValue<bool>(argv, argv + argc, "-w", false);
    char *ofn = getOption(argv, argv + argc, "-o");

    ofstream ofile;
    if(ofn) ofile.open(ofn, ios::binary);

    if(!weighted) {
        output_function func;
        if(ofn) func = [&ofile](uint64_t u, uint64_t v) -> void {
            //ofile.write((char*)(&u), sizeof(uint64_t));
            //ofile.write((char *)(&v), sizeof(uint64_t));
            ofile<<u<<" "<<v<<std::endl;
        };
        else func = [](uint64_t u, uint64_t v) -> void {
            cout << u << " " << v << endl;
        };
        er_gen(scale, degree, seed, func);
    } else {
        output_weighted_function func;
        if(ofn) func = [&ofile](uint64_t u, uint64_t v, float w) -> void {
            ofile.write((char*)(&u), sizeof(uint64_t));
            ofile.write((char *)(&v), sizeof(uint64_t));
            ofile.write((char *)(&w), sizeof(float));
        };
        else func = [](uint64_t u, uint64_t v, float w) -> void {
            cout << u << " " << v << " " << w << endl;
        };

        bool normal = getValue<bool>(argv, argv + argc, "-n", false);
        if(normal) er_gen_normal_weighted(scale, degree, seed, func);
        else er_gen_weighted(scale, degree, seed, func);
    }

    if(ofn) ofile.close();

    return 0;
}
