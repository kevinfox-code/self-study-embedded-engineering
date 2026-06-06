---
applyTo: "**/*.cpp,**/*.hpp,**/*.cxx"
---

# C++ Firmware Rules

These rules apply to all C++ source and header files in the project. They extend the C firmware rules with C++-specific guidance.

## C++ for Embedded — What to Use

- Use C++ for type safety, encapsulation, and zero-cost abstractions — not for runtime polymorphism in hot paths.
- Prefer `constexpr` over `#define` for compile-time constants.
- Prefer `enum class` (scoped enums) over C-style `enum` to prevent implicit conversions and namespace pollution.
- Prefer `static_assert()` for compile-time invariant checks (buffer sizes, struct layouts, alignment).
- Use RAII for hardware resource management — acquire peripheral in constructor, release in destructor (e.g., GPIO lock, DMA channel reservation).
- Use `std::array<T, N>` over raw C arrays when the STL header is available and the target supports it.

## C++ for Embedded — What to Avoid

- **No exceptions** (`-fno-exceptions`). Use return codes or `std::expected`/`std::optional` (C++23) for error handling.
- **No RTTI** (`-fno-rtti`). Do not use `dynamic_cast` or `typeid`.
- **No dynamic allocation** — no `new`, `delete`, `std::make_unique`, `std::make_shared`, `std::vector`, `std::string`, `std::map`, or any STL container that allocates.
- **No virtual dispatch in ISR or real-time hot paths** — virtual function calls have unpredictable timing due to vtable indirection and potential cache misses. Use CRTP (Curiously Recurring Template Pattern) or static polymorphism instead.
- **No `std::function`** — it may allocate. Use function pointers or templates.
- **Limit template instantiation depth** — excessive template metaprogramming bloats flash. Monitor `.text` section size.

## Class Design

- Mark all non-mutating methods `const`.
- Explicitly `= delete` copy/move constructors and assignment operators for peripheral wrapper classes (hardware is not copyable):
  ```cpp
  class UartDriver {
  public:
      UartDriver(const UartDriver&) = delete;
      UartDriver& operator=(const UartDriver&) = delete;
  };
  ```
- Use `explicit` on single-argument constructors to prevent implicit conversions.
- Prefer composition over inheritance. If inheritance is used, prefer `final` classes.
- Destructor of a peripheral wrapper should de-init the peripheral (`HAL_*_DeInit()`).

## HAL Integration in C++

- Wrap `extern "C"` around all HAL includes and callback implementations:
  ```cpp
  extern "C" {
  #include "stm32f4xx_hal.h"
  }

  extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
      // C++ code is fine here — the linkage is just C-compatible
      UartDriver::instance().on_rx_complete(huart);
  }
  ```
- Use a singleton or module-level instance pattern to bridge HAL C callbacks to C++ objects.

## Naming Conventions

- Classes and structs: `PascalCase` (e.g., `GpioPin`, `UartDriver`, `SensorReading`)
- Methods and free functions: `snake_case` (e.g., `init()`, `transmit_data()`, `read_sensor()`)
- Member variables: `snake_case_` with trailing underscore (e.g., `baud_rate_`, `handle_`)
- Constants and `constexpr`: `kPascalCase` or `UPPER_SNAKE_CASE` (be consistent per project)
- Namespaces: `lowercase` (e.g., `hal`, `drivers`, `app`)
- Template parameters: `PascalCase` (e.g., `typename PeripheralType`)
