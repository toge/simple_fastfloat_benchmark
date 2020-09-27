#include "fast_float/fast_float.h"
#include <cstdint>
#include <random>

class RandomEngine {
public:
  RandomEngine() = delete;
  RandomEngine(int new_seed) { wyhash64_x_ = new_seed; };
  uint64_t next() {
    // Adapted from https://github.com/wangyi-fudan/wyhash/blob/master/wyhash.h
    // Inspired from
    // https://github.com/lemire/testingRNG/blob/master/source/wyhash.h
    wyhash64_x_ += UINT64_C(0x60bee2bee120fc15);
    __uint128_t tmp;
    tmp = (__uint128_t)wyhash64_x_ * UINT64_C(0xa3b195354a39b70d);
    uint64_t m1 = (tmp >> 64) ^ tmp;
    tmp = (__uint128_t)m1 * UINT64_C(0x1b03738712fad5c9);
    uint64_t m2 = (tmp >> 64) ^ tmp;
    return m2;
  }
  bool next_bool() { return (next() & 1) == 1; }
  int next_int() { return static_cast<int>(next()); }
  char next_char() { return static_cast<char>(next()); }
  double next_double() { return static_cast<double>(next()); }

  int next_ranged_int(int min, int max) { // min and max are include
    // Adapted from
    // https://lemire.me/blog/2019/06/06/nearly-divisionless-random-integer-generation-on-various-systems/
    /*  if (min == max) {
         return min;
    }*/
    int s = max - min + 1;
    uint64_t x = next();
    __uint128_t m = (__uint128_t)x * (__uint128_t)s;
    uint64_t l = (uint64_t)m;
    if (l < s) {
      uint64_t t = -s % s;
      while (l < t) {
        x = next();
        m = (__uint128_t)x * (__uint128_t)s;
        l = (uint64_t)m;
      }
    }
    return (m >> 64) + min;
  }
  int next_digit() { return next_ranged_int(0, 9); }

private:
  uint64_t wyhash64_x_;
};

size_t build_random_string(RandomEngine &rand, char *buffer) {
  size_t pos{0};
  if (rand.next_bool()) {
    buffer[pos++] = '-';
  }
  int number_of_digits = rand.next_ranged_int(1, 100);
  int location_of_decimal_separator = rand.next_ranged_int(1, number_of_digits);
  for (size_t i = 0; i < number_of_digits; i++) {
    if (i == location_of_decimal_separator) {
      buffer[pos++] = '.';
    }
    buffer[pos++] = char(rand.next_digit() + '0');
  }
  if (rand.next_bool()) {
    if (rand.next_bool()) {
      buffer[pos++] = 'e';
    } else {
      buffer[pos++] = 'E';
    }
    if (rand.next_bool()) {
      buffer[pos++] = '-';
    } else {
      if (rand.next_bool()) {
        buffer[pos++] = '+';
      }
    }
    number_of_digits = rand.next_ranged_int(1, 3);
    for (size_t i = 0; i < number_of_digits; i++) {
      buffer[pos++] = char(rand.next_digit() + '0');
    }
  }
  buffer[pos] = '\0'; // null termination
  return pos;
}

std::pair<double, bool> strtod_from_string(char *st) {
  double d;
  char *pr;
#ifdef _WIN32
  static _locale_t c_locale = _create_locale(LC_ALL, "C");
  d = _strtod_l(st, &pr, c_locale);
#else
  static locale_t c_locale = newlocale(LC_ALL_MASK, "C", NULL);
  d = strtod_l(st, &pr, c_locale);
#endif
  if (st == pr) {
    std::cerr << "strtod_l could not parse '" << st << std::endl;
    return std::make_pair(0, false);
  }
  return std::make_pair(d, true);
}

bool tester(int seed, size_t volume) {
  char buffer[1024]; // large buffer (can't overflow)
  RandomEngine rand(seed);
  for (size_t i = 0; i < volume; i++) {
    if((i%100000) == 0) { std::cout << "."; std::cout.flush(); }
    size_t length = build_random_string(rand, buffer);
    std::pair<double, bool> expected = strtod_from_string(buffer);
    if (expected.second) {
      double result_value;
      auto result =
          fast_float::from_chars(buffer, buffer + length, result_value);
      if (result.ec != std::errc()) {
        printf("parsing %.*s\n", int(length), buffer);
        std::cerr << " I could not parse " << std::endl;
        return false;
      }
      if (result.ptr != buffer + length) {
        printf("parsing %.*s\n", int(length), buffer);
        std::cerr << " Did not get to the end " << std::endl;
        return false;
      }
      if (result_value != expected.first) {
        printf("parsing %.*s\n", int(length), buffer);
        std::cerr << std::hexfloat << result_value << std::endl;
        std::cerr << std::hexfloat << expected.first << std::endl;
        std::cerr << " Mismatch " << std::endl;
        return false;
      }
    }
  }
  return true;
}

int main() {
  if (tester(1234344, 100000000)) {
    std::cout << "All tests ok." << std::endl;
    return EXIT_SUCCESS;
  }
  std::cout << "Failure." << std::endl;
  return EXIT_FAILURE;
}