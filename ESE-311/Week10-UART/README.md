# ESE-311 Week 10 — UART

## Objectives

- Configure USART1 at the register level and route it to the ST-LINK V3E
  virtual COM port.
- Derive the baud-rate divisor from the active clock source rather than copying
  a magic number.
- Provide blocking character and string transmit, and retarget `printf`-style
  output over the serial link.

## What Was Built

- [`UART/Src/uart.c`](UART/Src/uart.c) — USART1 driver on PA9 (TX) and PA10 (RX),
  alternate function 7, clocked from the 16 MHz HSI16 system clock at 115200 baud.
  Blocking character send, string send, and character receive.
- [`UART/Inc/uart.h`](UART/Inc/uart.h) — `uart_init`, `uart_send_char`,
  `uart_send_string`, `uart_recv_char`.
- [`UART/Src/main.c`](UART/Src/main.c) — transmits a greeting every 250 ms while
  toggling the green LED.
- GPIO and SysTick modules carried forward from Weeks 7–8.

## Key Concepts

- **BRR and the clock source.** `USARTDIV = f_ck / baud` in oversample-by-16
  mode. If the clock the USART is actually fed differs from the one assumed,
  the framing is wrong even though the code looks right.
- **Alternate function selection.** `GPIOx_AFRL`/`AFRH` select AF7 for USART1;
  the mode register must also move the pin out of its reset input state.
- **TXE vs TC.** `TXE` means the data register accepted a byte; `TC` means the
  shift register has finished. Waiting on the wrong flag truncates the last
  character on a power-down or reconfigure.
- **Retargeting stdio.** `syscalls.c` supplies `_write`, so `printf` and
  `snprintf` output can be pushed through the driver.

## Build and Run

```bash
cd ESE-311/Week10-UART/UART
cmake -B build
cmake --build build
cmake --build build --target flash
```

Open a serial terminal on the ST-LINK virtual COM port at 115200 8N1.

## VS Code Debug

Requires the Cortex-Debug extension, with `arm-none-eabi-gdb` and `openocd` on
`PATH`. Open the `UART/` folder as the workspace and launch
**Debug UART (OpenOCD)**, which builds `build/uart.elf` via the `cmake: build` task.

## References

- Israel Gbati, *Bare-Metal Embedded C Programming* — USART chapter
- RM0456 §USART; UM2861 for the ST-LINK V3E VCP pin routing
- See [ESE-311_References.md](../ESE-311_References.md) for download links

## AI Assistance

None.
