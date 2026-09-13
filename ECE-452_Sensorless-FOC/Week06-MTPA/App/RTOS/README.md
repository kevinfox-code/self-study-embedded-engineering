# App/RTOS — app-owned RTOS extensions

`FreeRTOSConfig.h` is **CubeMX-generated into the board profile**
(root-level `Profiles/NucleoU575/Core/Inc/`) — do not hand-place one here.
This folder holds only application-owned RTOS extensions, currently
`rtos_hooks.c` (stack-overflow / malloc-failed safe-stop hooks).

## Required FreeRTOS configuration (set in CubeMX, guide §7)

| Setting | Value |
|---|---|
| Interface | CMSIS_V2 |
| TICK_RATE_HZ | 1000 (1 ms — library tasks use `osDelayUntil`) |
| configMAX_SYSCALL_INTERRUPT_PRIORITY | 5 (NVIC map anchor, guide §8) |
| Memory scheme / heap | heap_4, TOTAL_HEAP_SIZE ≥ 8 KB |
| HAL timebase | TIM6 (FreeRTOS owns SysTick) |
| configCHECK_FOR_STACK_OVERFLOW | 2 (enables hook in `rtos_hooks.c`) |
| configUSE_MALLOC_FAILED_HOOK | 1 (enables hook in `rtos_hooks.c`) |

The three motor tasks (`tsk_current`, `tsk_speed`, `tsk_supervisor`)
are created by the library inside `foc_app_init()` — never create them
in CubeMX or here.
