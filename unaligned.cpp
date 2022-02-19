// Сборка:
// g++ -fsanitize=alignment -Wall -Wextra -std=c++2a unaligned.cpp -o unaligned
// clang++ -fsanitize=alignment -Wall -Wextra -std=c++20 unaligned.cpp -o
// unaligned

#include <cstring>
#include <iostream>
#include <string_view>
#include <vector>

// Реализуйте шаблон класса UnalignedPtr.
// Класс должен обладать интерфейсом указателя и реализовывать безопасное
// чтение невыровненных данных.
template <class T> class UnalignedPtr {};

// По мере доработки увеличивайте уровень. На каждом уровне добавляются новые
// тесты. Каждый уровень предполагает изучение новой техники.
//
// 0. Чтение невыровненных данных через сырой указатель. Убедитесь, что можете
//    скомпилировать программу со включенным санитайзером и увидеть его
//    диагностику при запуске.
// 1. Простое чтение. Темы: конкретные классы, перегрузка операторов.
// 2. Запись невыровненных данных. Паттерн Proxy.
// 3. Const-correctness. Type traits. https://en.cppreference.com/w/cpp/types
#ifndef TEST_LEVEL
#define TEST_LEVEL 0
#endif

// Вспомогательный код и тесты.
namespace {

namespace gears {

using TestCaseFunction = void (*)();

struct TestCase {
  std::string_view test_name_;
  TestCaseFunction test_case_function_;
};

std::vector<TestCase> g_test_cases;

struct TestAppender {
  constexpr TestAppender(std::string_view name, TestCaseFunction f) {
    g_test_cases.push_back({name, f});
  }
};

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define TEST(FUNCTION_NAME)                                                    \
  static void FUNCTION_NAME();                                                 \
  gears::TestAppender CONCAT(ta, __LINE__)(#FUNCTION_NAME, FUNCTION_NAME);     \
  static void FUNCTION_NAME()

#define ENSURE_THROW(expr, msg)                                                \
  do {                                                                         \
    if (!(expr)) {                                                             \
      throw std::runtime_error(msg);                                           \
    }                                                                          \
  } while (0)

} // namespace gears

namespace tests {

#if TEST_LEVEL == 0
TEST(unsafe_read) {
  char buf[100] = {};
  char *ptr = buf + 1;
  int value = 42;
  std::memcpy(ptr, &value, sizeof(int));

  const int *valueptr = reinterpret_cast<int *>(ptr);
  const auto read_value = *valueptr;
  ENSURE_THROW(read_value == 42, "Bad unsafe read");
}
#endif

#if TEST_LEVEL >= 1
TEST(safe_read_mut_buf_mut_ptr) {
  char buf[100] = {};
  char *ptr = buf + 1;
  int value = 42;
  std::memcpy(ptr, &value, sizeof(int));

  {
    UnalignedPtr<int> uptr(ptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
  {
    const UnalignedPtr<int> uptr(ptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
}
#endif

#if TEST_LEVEL >= 2
TEST(safe_write_mut_buf_mut_ptr) {
  char buf[100] = {};
  char *ptr = buf + 1;

  {
    UnalignedPtr<int> uptr(ptr);
    *uptr = 24;
    ENSURE_THROW(*uptr == 24, "Bad safe write");
  }
  {
    const UnalignedPtr<int> uptr(ptr);
    *uptr = 21;
    ENSURE_THROW(*uptr == 21, "Bad safe write");
  }
}
#endif

#if TEST_LEVEL >= 3
TEST(safe_read_mut_buf_const_ptr) {
  char buf[100] = {};
  char *ptr = buf + 1;
  int value = 42;
  std::memcpy(ptr, &value, sizeof(int));

  {
    UnalignedPtr<const int> uptr(ptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
  {
    const UnalignedPtr<const int> uptr(ptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
}

TEST(safe_write_const_buf_const_ptr) {
  char buf[100] = {};
  char *ptr = buf + 1;
  int value = 42;
  std::memcpy(ptr, &value, sizeof(int));

  const char *cptr = buf + 1;
  {
    UnalignedPtr<const int> uptr(cptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
  {
    const UnalignedPtr<const int> uptr(cptr);
    const auto read_value = *uptr;
    ENSURE_THROW(read_value == 42, "Bad safe read");
  }
}
#endif

} // namespace tests

} // namespace

int main() {
  int rv = 0;

  for (const auto &test_case : gears::g_test_cases) {
    try {
      std::cout << test_case.test_name_ << '\n';
      test_case.test_case_function_();
    } catch (const std::runtime_error &ex) {
      std::cout << "  " << ex.what() << '\n';
      rv = 1;
    }
  }
  return rv;
}
