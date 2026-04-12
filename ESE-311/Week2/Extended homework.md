# HW2: System Architecture — Diagrams \& Driver Interfaces

**Course:** Making Embedded Systems — 16-Week Self-Study
**Week:** 2
**Reading:** Ch. 2 — *Creating a System Architecture* (pp. 31–77)
**Estimated Time:** 6–10 hours
**Weight:** 2.8 points

***

## Learning Objectives

By the end of this assignment, you will be able to:

- Describe a system at two levels of abstraction using a context diagram and a block diagram
- Apply White's module encapsulation principles to carve a system into clean boundaries[^1]
- Implement a hardware-independent driver interface skeleton using the open/close/read/write/ioctl pattern from Chapter 2[^2]
- Explain *why* the adapter pattern decouples hardware details from application logic[^2]

***

## Part 1 — System Diagrams (35%)

### 1.1 Choose Your System

Pick **one** of the following simple embedded systems to use as your subject for the entire assignment. Choose something small enough to diagram completely, but real enough to have 3–5 meaningful hardware peripherals.


| Option | Description |
| :-- | :-- |
| A | Digital thermostat — temperature sensor, display, relay-controlled heater, button UI |
| B | Bicycle speedometer — hall-effect sensor, small OLED display, optional BLE module |
| C | Smart plant watering — soil moisture ADC, water pump via MOSFET, RTC for scheduling |
| D | *Your own choice* — must have at least one sensor, one actuator, and one communication peripheral |

Write a 2–3 sentence **product definition** before drawing anything: what does it do, who uses it, and what are its key constraints (power, cost, response time)?

***

### 1.2 Context Diagram

Draw a **context diagram** (also called a system context diagram) showing your system as a single black box in the center, with all external actors and data flows around it. This diagram answers: *"What touches this system, and what data flows in and out?"*[^1]

**Requirements:**

- The system itself is a single labeled box
- External actors (user, sensors, power supply, host PC, cloud, etc.) are shown as labeled entities outside the box
- Arrows represent data or signal flows; label each arrow with what is flowing (e.g., "temperature reading," "button press," "PWM signal")
- Use any tool: paper + photo, draw.io, Lucidchart, Mermaid, or KiCad[^3]

**Deliverable:** One PNG or PDF of the context diagram embedded in your README or as a separate file named `context_diagram.png`.

***

### 1.3 Software Block Diagram

Draw a **software block diagram** that decomposes the interior of the system into its major software modules. This diagram answers: *"What are the software components, and how do they depend on each other?"*[^1]

**Requirements:**

- At minimum, show: Application layer, one or more driver modules, and a HAL (Hardware Abstraction Layer) or BSP layer at the bottom
- Show dependencies between blocks with arrows (dependencies point downward in a layered architecture)
- Every hardware peripheral in your context diagram must map to at least one block in the software block diagram
- Label each block with its module name (e.g., `temp_sensor_driver`, `display_driver`, `main_app`, `scheduler`)

**Deliverable:** One PNG or PDF named `block_diagram.png`.

***

### 1.4 Short Written Analysis

In **4–6 sentences** (include in your README), answer:

1. Where in your block diagram is the application layer most likely to change if the hardware is swapped out?
2. Which module has the most dependencies? Is that a design problem?
3. Did the act of drawing the diagrams reveal any interface or dependency you hadn't thought about?

***

## Part 2 — Driver Interface Skeleton in C (50%)

Implement a **hardware-independent driver interface** for one peripheral from your chosen system, following the open/close/read/write/ioctl pattern described in Chapter 2.[^2]

### 2.1 File Structure

Your submission must contain exactly these files:

```
hw2/
├── README.md
├── context_diagram.png
├── block_diagram.png
├── drivers/
│   ├── peripheral_driver.h     ← your chosen peripheral name
│   ├── peripheral_driver.c
│   └── peripheral_driver_test.c
└── Makefile  (or build instructions in README)
```


***

### 2.2 The Header File — `peripheral_driver.h`

Your `.h` file must define:

**1. An opaque handle or config struct** representing an instance of the peripheral. Use a forward declaration to keep the struct opaque to callers:

```c
// peripheral_driver.h
#ifndef PERIPHERAL_DRIVER_H
#define PERIPHERAL_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// Opaque handle — callers hold a pointer; internals are hidden
typedef struct PeripheralHandle PeripheralHandle_t;

// Status codes returned by all driver functions
typedef enum {
    DRIVER_OK        = 0,
    DRIVER_ERR_INIT  = -1,
    DRIVER_ERR_BUSY  = -2,
    DRIVER_ERR_PARAM = -3,
    DRIVER_ERR_IO    = -4
} DriverStatus_t;
```

**2. The five standard interface functions:**

```c
// Allocate and initialize the peripheral; returns a handle or NULL on failure
PeripheralHandle_t* peripheral_open(uint32_t device_id, uint32_t baud_or_config);

// Release resources and shut down the peripheral
DriverStatus_t peripheral_close(PeripheralHandle_t *handle);

// Read `len` bytes into `buf`; returns number of bytes read or negative error
int32_t peripheral_read(PeripheralHandle_t *handle, uint8_t *buf, uint32_t len);

// Write `len` bytes from `buf`; returns number of bytes written or negative error
int32_t peripheral_write(PeripheralHandle_t *handle,
                         const uint8_t *buf, uint32_t len);

// Device-specific control: set sample rate, resolution, power mode, etc.
DriverStatus_t peripheral_ioctl(PeripheralHandle_t *handle,
                                uint32_t command, void *arg);

// IOCTL command codes — add device-specific ones here
#define IOCTL_SET_SAMPLERATE   0x01
#define IOCTL_GET_STATUS       0x02
#define IOCTL_POWER_DOWN       0x03

#endif // PERIPHERAL_DRIVER_H
```

> **Why this interface?** The caller never sees register addresses. You can swap the underlying hardware by replacing only `peripheral_driver.c` — the application and test code remain unchanged. This is the key design-for-change principle from Chapter 2.[^1]

***

### 2.3 The Implementation File — `peripheral_driver.c`

Implement each function as a **stub with logging** — no real hardware needed. The goal is a compilable, testable skeleton, not a complete HAL.

```c
// peripheral_driver.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "peripheral_driver.h"

// Internal state — hidden from callers because the struct is defined HERE, not in .h
struct PeripheralHandle {
    uint32_t device_id;
    uint32_t config;
    bool     is_open;
    uint32_t sample_rate_hz;
};

PeripheralHandle_t* peripheral_open(uint32_t device_id, uint32_t config) {
    PeripheralHandle_t *h = malloc(sizeof(PeripheralHandle_t));
    if (!h) return NULL;

    h->device_id      = device_id;
    h->config         = config;
    h->is_open        = true;
    h->sample_rate_hz = 10;  // default 10 Hz

    printf("[peripheral_open] device_id=%u config=0x%08X\n", device_id, config);
    return h;
}

DriverStatus_t peripheral_close(PeripheralHandle_t *handle) {
    if (!handle || !handle->is_open) return DRIVER_ERR_PARAM;
    printf("[peripheral_close] device_id=%u\n", handle->device_id);
    handle->is_open = false;
    free(handle);
    return DRIVER_OK;
}

int32_t peripheral_read(PeripheralHandle_t *handle, uint8_t *buf, uint32_t len) {
    if (!handle || !handle->is_open || !buf || len == 0) return DRIVER_ERR_PARAM;
    // Stub: fill buffer with incrementing dummy data
    for (uint32_t i = 0; i < len; i++) buf[i] = (uint8_t)(i + 0xA0);
    printf("[peripheral_read] read %u bytes\n", len);
    return (int32_t)len;
}

int32_t peripheral_write(PeripheralHandle_t *handle,
                         const uint8_t *buf, uint32_t len) {
    if (!handle || !handle->is_open || !buf || len == 0) return DRIVER_ERR_PARAM;
    printf("[peripheral_write] wrote %u bytes: 0x%02X ...\n", len, buf[^0]);
    return (int32_t)len;
}

DriverStatus_t peripheral_ioctl(PeripheralHandle_t *handle,
                                uint32_t command, void *arg) {
    if (!handle || !handle->is_open) return DRIVER_ERR_PARAM;

    switch (command) {
        case IOCTL_SET_SAMPLERATE:
            handle->sample_rate_hz = *(uint32_t *)arg;
            printf("[peripheral_ioctl] sample rate set to %u Hz\n",
                   handle->sample_rate_hz);
            return DRIVER_OK;

        case IOCTL_GET_STATUS:
            *(bool *)arg = handle->is_open;
            return DRIVER_OK;

        case IOCTL_POWER_DOWN:
            printf("[peripheral_ioctl] powering down\n");
            handle->is_open = false;
            return DRIVER_OK;

        default:
            printf("[peripheral_ioctl] unknown command 0x%08X\n", command);
            return DRIVER_ERR_PARAM;
    }
}
```


***

### 2.4 The Test File — `peripheral_driver_test.c`

Write a `main()` that exercises all five interface functions and validates return values. This runs on your **PC** with no hardware.[^1]

```c
// peripheral_driver_test.c
#include <stdio.h>
#include <assert.h>
#include "peripheral_driver.h"

static void test_open_close(void) {
    printf("--- test_open_close ---\n");
    PeripheralHandle_t *h = peripheral_open(1, 0x0001);
    assert(h != NULL);
    DriverStatus_t s = peripheral_close(h);
    assert(s == DRIVER_OK);
    printf("PASS\n\n");
}

static void test_read(void) {
    printf("--- test_read ---\n");
    PeripheralHandle_t *h = peripheral_open(1, 0x0001);
    uint8_t buf[^4] = {0};
    int32_t n = peripheral_read(h, buf, 4);
    assert(n == 4);
    assert(buf[^0] == 0xA0);
    printf("buf[^0]=0x%02X buf[^1]=0x%02X\n", buf[^0], buf[^1]);
    peripheral_close(h);
    printf("PASS\n\n");
}

static void test_write(void) {
    printf("--- test_write ---\n");
    PeripheralHandle_t *h = peripheral_open(1, 0x0001);
    uint8_t data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    int32_t n = peripheral_write(h, data, 4);
    assert(n == 4);
    peripheral_close(h);
    printf("PASS\n\n");
}

static void test_ioctl_samplerate(void) {
    printf("--- test_ioctl_samplerate ---\n");
    PeripheralHandle_t *h = peripheral_open(1, 0x0001);
    uint32_t rate = 50;
    DriverStatus_t s = peripheral_ioctl(h, IOCTL_SET_SAMPLERATE, &rate);
    assert(s == DRIVER_OK);
    peripheral_close(h);
    printf("PASS\n\n");
}

static void test_null_handle(void) {
    printf("--- test_null_handle (error path) ---\n");
    int32_t n = peripheral_read(NULL, NULL, 0);
    assert(n == DRIVER_ERR_PARAM);
    printf("PASS\n\n");
}

int main(void) {
    test_open_close();
    test_read();
    test_write();
    test_ioctl_samplerate();
    test_null_handle();
    printf("=== All tests passed ===\n");
    return 0;
}
```

**Build and run on your PC:**

```bash
gcc -Wall -Wextra -o hw2_test drivers/peripheral_driver.c \
    drivers/peripheral_driver_test.c && ./hw2_test
```

All five tests must pass with zero compiler warnings before submission.

***

## Part 3 — Reflection (15%)

In your README, write **4–6 sentences** answering:

1. What was the hardest part of deciding where to draw module boundaries in your block diagram?
2. The `ioctl` function takes a `void *arg` — this is flexible but not type-safe. What is the tradeoff, and how might you mitigate it in a production driver?
3. If you needed to port your driver to a different microcontroller (e.g., swap from STM32 to RP2040), which files would change and which would stay the same? Why?

***

## Submission Checklist

Before zipping and submitting, confirm:

- [ ] `context_diagram.png` — single black-box system with labeled external actors and data flows
- [ ] `block_diagram.png` — layered software blocks with dependency arrows
- [ ] `peripheral_driver.h` — opaque handle, `DriverStatus_t` enum, all 5 function declarations
- [ ] `peripheral_driver.c` — all 5 functions implemented (stubs are fine; must compile)
- [ ] `peripheral_driver_test.c` — at least 5 test cases, all passing with `assert()`
- [ ] `README.md` — product definition, written analysis (Part 1.4), and reflection (Part 3)
- [ ] Zero compiler warnings with `gcc -Wall -Wextra`
- [ ] Consistent code style; all non-obvious lines commented

***

## Grading Rubric

| Criterion | Points |
| :-- | :-- |
| Context diagram — correct structure, labeled flows | 0.4 |
| Block diagram — layered, all peripherals represented | 0.5 |
| Written analysis of diagrams | 0.3 |
| `.h` file — opaque handle, enum, all 5 signatures correct | 0.4 |
| `.c` file — all 5 functions compile and run | 0.4 |
| Test file — ≥5 tests, all pass, error path covered | 0.5 |
| Reflection — depth and accuracy | 0.3 |
| **Total** | **2.8** |


***

## Study Tips for This Week

Re-read the *"Driver Interface: Open, Close, Read, Write, IOCTL"* and *"Adapter Pattern"* sections of Chapter 2 before writing your `.h` file — White explains exactly why this interface shape is the standard. The key insight is that the application layer calls `peripheral_read()` without ever knowing whether it's talking to a UART, an I2C sensor, or a stub returning fake data; that indirection is the entire point. When you hit trouble with the opaque struct pattern, search for "C opaque pointer pattern" — it is one of the most important C idioms for embedded driver design.[^4][^2][^1]

[^1]: https://www.oreilly.com/library/view/making-embedded-systems/9781098151539/ch02.html

[^2]: https://pdfcoffee.com/elecia-white-ox27reilly-making-embedded-systems-2011-pdf-free.html

[^3]: https://github.com/eleciawhite/making-embedded-systems/blob/main/Ch02_Architecture/README.md

[^4]: https://www.embeddedrelated.com/showarticle/1596.php

[^5]: https://www.oreilly.com/library/view/making-embedded-systems/9781449308889/ch02.html

[^6]: https://api.pageplace.de/preview/DT0400.9781098151515_A49445162/preview-9781098151515_A49445162.pdf

[^7]: https://www.geeksforgeeks.org/system-design/design-patterns-for-embedded-systems-in-c/

[^8]: https://www.scribd.com/document/392049040/Making-Embedded-Systems

[^9]: https://stackoverflow.com/questions/76156157/embedded-driver-singleton-to-handle-multiple-hardware-interfaces-of-the-same-ty

[^10]: https://api.pageplace.de/preview/DT0400.9781449320591_A24027464/preview-9781449320591_A24027464.pdf

[^11]: https://www.reddit.com/r/Scholar/comments/1hy88cm/book_making_embedded_systems_2nd_edition_2024_by/

[^12]: https://www.youtube.com/watch?v=S0ODfxXe2UU

[^13]: https://jaycarlson.net/embedded-linux/

[^14]: https://www.reddit.com/r/embedded/comments/1hjmejv/when_do_i_actually_need_to_write_bare_metal/

[^15]: https://batch.libretexts.org/print/Letter/Finished/eng-25595/Full.pdf

