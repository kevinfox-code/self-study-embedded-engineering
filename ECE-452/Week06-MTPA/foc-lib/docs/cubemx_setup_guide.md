# CubeMX Setup Guide — STM32U5 + DRV8323 Sensorless FOC

This guide walks through configuring an STM32U5 project in STM32CubeMX (or the CubeIDE device configuration tool) so it works with `foc-lib`. The library consumes the peripherals CubeMX initializes — it never re-initializes them — so every setting here matters. Names used below match the macros in [`App/constants.h`](../../App/constants.h) (the application's bridge file, moved from `src/port/stm32u5/`); if you deviate from a pin or instance choice, update `constants.h` accordingly (that is the *only* file you should need to edit).

**Reference target:** STM32U575ZIT6Q (Nucleo-U575ZI-Q), 160 MHz, TIM1 PWM at 20 kHz, DRV8323 SPI gate driver, 3 low-side shunts.

---

## 1. Project settings

| Setting | Value | Why |
|---|---|---|
| Toolchain / IDE | Makefile or CMake (or STM32CubeIDE) | Library builds as `libfoc.a` via its own CMake; the app project links it |
| Generate peripheral initialization as pairs of `.c/.h` | **Yes** | Keeps `main.c` clean; library port layer includes `main.h` only through `constants.h` |
| Keep user code when regenerating | **Yes** | You will add calls inside `USER CODE` markers |
| Set all free pins as analog | Yes | Lowest power, avoids floating inputs |
| HAL timebase | **TIM6** (or any basic timer), *not* SysTick | FreeRTOS owns SysTick; HAL needs its own timebase when an RTOS is present |

> **Rule from the library:** never add code outside `USER CODE` markers, and the only library calls you add to Cube-generated files are `foc_app_init()` (after peripheral init) and the three ISR body hooks (Section 9).

---

## 2. Clock configuration

Target: **160 MHz** SYSCLK from PLL1 (matches `BSP_CPU_FREQ_HZ`).

1. **RCC**: HSE = *BYPASS Clock Source* on a Nucleo (8 MHz from ST-LINK MCO), or *Crystal/Ceramic* on a custom board. HSI16 also works if you have no crystal — the PLL settings below assume 16 MHz input either way.
2. In the Clock Configuration tab:
   - PLL1 source: HSE (or HSI16)
   - Set **SYSCLK = 160 MHz** (let CubeMX solve; typical: /1 M-div, ×20 N-mul from 16 MHz, /2 P-div)
   - AHB/APB1/APB2 prescalers: **/1** (all buses at 160 MHz — TIM1 and ADC kernel clocks derive from these)
3. **Voltage scaling**: Range 1 (required above 110 MHz). CubeMX sets this automatically when you request 160 MHz; verify in `SystemClock_Config`.
4. **ADC clock mux** (Clock Configuration tab): select **HCLK/2** (80 MHz) or an async source ≤ the datasheet max for the U5 ADC (ADC4/ADC1 differ — for ADC1 use ≤ 130 MHz kernel; 80 MHz with /4 internal prescaler is a safe, fast choice).

> If you change the CPU frequency, update `BSP_CPU_FREQ_HZ` **and** recompute `BSP_PWM_PERIOD_TICKS` (Section 3) — the library asserts the PWM math at init.

---

## 3. TIM1 — center-aligned PWM with complementary outputs

TIM1 generates the three-phase PWM and hardware-triggers the ADC. This is the most setting-dense peripheral; go slowly.

### 3.1 Mode panel

- **Channel1, Channel2, Channel3**: `PWM Generation CH1 CH1N` (and CH2/CH2N, CH3/CH3N) — complementary outputs
- **Channel4**: leave disabled (or `PWM Generation No Output` if you later want a movable ADC trigger point)
- Do **not** enable Channel4 output pins

Default Nucleo-friendly pins (verify against your board schematic and adjust in the Pinout view):

| Signal | Pin (example) |
|---|---|
| TIM1_CH1 / CH1N | PE9 / PE8 |
| TIM1_CH2 / CH2N | PE11 / PE10 |
| TIM1_CH3 / CH3N | PE13 / PE12 |
| TIM1_BKIN (optional, Section 3.4) | PE15 |

### 3.2 Counter settings

| Parameter | Value | Notes |
|---|---|---|
| Prescaler | **0** | Full 160 MHz timer clock |
| Counter Mode | **Center Aligned mode 1** | Up-down counting; update event at underflow — the low-side conduction center |
| Counter Period (ARR) | **4000** | 160 MHz ÷ (2 × 4000) = **20 kHz** PWM. Must equal `BSP_PWM_PERIOD_TICKS` |
| Repetition Counter (RCR) | **1** | With center-aligned mode, RCR=1 gives **one update event per full PWM period** (at the underflow only) instead of two — this paces the ADC trigger and fast loop at exactly 20 kHz |
| auto-reload preload | Enable | Glitch-free period changes (not used at runtime, but safe) |

### 3.3 PWM channel settings (all three channels)

| Parameter | Value |
|---|---|
| Mode | PWM mode 1 |
| Pulse | 2000 (50% — the library overwrites this; 50% = zero vector at init) |
| Output compare preload | **Enable** (mandatory — compare updates latch at update event, preventing mid-period glitches) |
| CH Polarity / CHN Polarity | High / High (check your gate-driver input polarity — DRV8323 INx are active-high in 6x mode) |
| CH Idle State / CHN Idle State | **Reset / Reset** (both FETs off when MOE drops — this is the safe state) |

### 3.4 Break and dead time

| Parameter | Value | Notes |
|---|---|---|
| **Dead Time** | ~**100 ns** worth of ticks | At 160 MHz, DTG ≈ 16 ticks = 100 ns. Start conservative (100–200 ns); the DRV8323 also enforces its own handshake-based dead time. Enter the raw DTG value CubeMX asks for and verify the computed ns readout |
| Break Input (BKIN) | Optional but recommended | Wire DRV8323 **nFAULT** to TIM1_BKIN as a *hardware* PWM kill path (faster than the EXTI software path). Polarity: **Low** (nFAULT is active-low) |
| Break Polarity | Low | |
| Automatic Output Enable (AOE) | **Disable** | The library re-enables MOE deliberately after a fault — never automatically |
| Off State (Run/Idle) | OSSR = Enable, OSSI = Enable | Outputs driven to their idle (off) level rather than Hi-Z when MOE is cleared, keeping gates actively low |
| Lock level | Off (or Level 1 once bring-up is done) | Level 1 write-protects dead-time/break config against runaway code |

### 3.5 Trigger output (ADC pacing)

| Parameter | Value |
|---|---|
| **Trigger Event Selection (TRGO)** | **Update Event** |

With center-aligned mode + RCR=1, the update event fires at the counter **underflow** — the center of the low-side conduction window, exactly where the shunt currents must be sampled.

### 3.6 NVIC (for TIM1)

- **TIM1 break interrupt**: enable if using BKIN, priority 0 (highest). Its ISR body routes to the library's fault raise (Section 9).
- Do **not** enable the TIM1 update interrupt — the fast loop runs from the ADC end-of-conversion interrupt instead, which fires only after all samples are ready.

---

## 4. ADC1 — injected conversions, hardware-triggered

Three phase currents + Vbus are converted as an **injected group** triggered by TIM1 TRGO. Injected conversions preempt any regular conversions and land in dedicated result registers the ISR reads directly — no DMA in the fast path.

### 4.1 Mode panel

Enable four injected channels. Example mapping (adjust to your board; then mirror in `constants.h` rank macros):

| Rank | Signal | Example channel/pin |
|---|---|---|
| 1 | Phase A current (`BSP_ADC_RANK_IA`) | IN1 / PC0 |
| 2 | Phase B current (`BSP_ADC_RANK_IB`) | IN2 / PC1 |
| 3 | Phase C current (`BSP_ADC_RANK_IC`) | IN3 / PC2 |
| 4 | Vbus divider (`BSP_ADC_RANK_VBUS`) | IN4 / PC3 |

Optional: a 5th rank (or a slow regular-group conversion) for board temperature.

### 4.2 Parameter settings

| Parameter | Value | Notes |
|---|---|---|
| Clock Prescaler | Async or sync div such that ADC kernel ≤ datasheet max | See Section 2 clock mux |
| Resolution | **12-bit** | Matches `BSP_ADC_FULLSCALE = 4096` |
| Data Alignment | Right | |
| Scan Conversion Mode | Enabled (implicit with ranks) | |
| Continuous Conversion | **Disabled** | One conversion set per trigger |
| Discontinuous / Injected Discontinuous | Disabled | All 4 ranks convert back-to-back per trigger |
| Overrun behaviour | Data overwritten | Latest sample always wins |

### 4.3 Injected group settings

| Parameter | Value | Notes |
|---|---|---|
| **External Trigger Source** | **Timer 1 Trigger Out event (TRGO)** | The critical link to Section 3.5 |
| External Trigger Edge | Rising | |
| Injected Number of Conversions | 4 | |
| Sampling Time (current channels) | Short — e.g. **6.5 or 12.5 cycles** | The DRV8323 CSA output is low-impedance; short sampling keeps all 4 conversions well inside the low-side window |
| Sampling Time (Vbus) | Longer OK — e.g. 24.5+ cycles | High-impedance divider needs it; it converts last |

**Timing sanity check:** 4 conversions × (sampling + 12.5 conv cycles) at the ADC kernel clock must be ≪ the low-side conduction time at your maximum modulation index. At 80 MHz kernel / 12-bit / 12.5-cycle sampling, 4 conversions ≈ 1.3 µs — comfortably inside the window the SVPWM min-pulse clamp guarantees.

### 4.4 NVIC (for ADC)

- **Enable the ADC1 (or ADC1_2) global interrupt**. The injected end-of-conversion (JEOC/JEOS) interrupt *is the fast loop* — its ISR body calls `foc_ctrl_fast_loop()` via the library's `isr_motor.c` handler.
- Priority: see the NVIC map in Section 8. It must be **numerically lower (higher urgency)** than `configMAX_SYSCALL_INTERRUPT_PRIORITY`? **No — the opposite.** It must be *at or below* (numerically ≥) the FreeRTOS syscall threshold because it issues `osThreadFlagsSet` every 20th cycle (library design decision, plan §9.1). Use priority **5** with the default `configMAX_SYSCALL_INTERRUPT_PRIORITY` of 5.

### 4.5 Calibration

Do not add manual calibration code — `board_support.c` runs `HAL_ADCEx_Calibration_Start()` during its init, and the library's CALIBRATE state measures the CSA/ADC offsets afterward. Just make sure nothing in Cube-generated user code starts conversions before `foc_app_init()` runs.

---

## 5. SPI — DRV8323 register access

Any SPI instance works; SPI1 is assumed (`BSP_SPI_DRV_HANDLE`).

| Parameter | Value | Notes |
|---|---|---|
| Mode | Full-Duplex Master | |
| Hardware NSS | **Disabled** — use a GPIO for nSCS | DRV8323 needs nSCS framing per 16-bit word; GPIO gives explicit control |
| Frame size | **16 bits** | DRV8323 frames are 16-bit (1 R/W + 4 addr + 11 data) |
| First bit | MSB first | |
| Prescaler | ≤ **10 MHz** SCLK (e.g. 160 MHz/16 = 10 MHz; /32 = 5 MHz is safer for bring-up) | DRV8323 max SPI clock is 10 MHz |
| CPOL / CPHA | **Low / 2nd edge** (SPI mode 1) | DRV8323 samples on the falling edge — mode 1 is required |
| NSSP | Disabled | |

Add a GPIO output for **nSCS** (e.g. PA4): output push-pull, pull-up, initial level **High**, speed Low. `drv8323_transport.c` toggles it around each 16-bit transfer.

---

## 6. GPIO — DRV8323 control and status

| Signal | Direction | CubeMX config | `constants.h` macro |
|---|---|---|---|
| DRV **ENABLE** | Output | Push-pull, no pull, initial **Low** (driver disabled at boot) | `BSP_GPIO_DRV_EN_PORT/PIN` |
| DRV **nFAULT** | Input + EXTI | **External Interrupt Mode, falling edge**, pull-up enabled | `BSP_GPIO_NFAULT_PORT/PIN`, `BSP_NFAULT_EXTI_IRQn` |
| DRV nSCS | Output | Push-pull, initial High (Section 5) | used by `drv8323_transport.c` |
| Debug/WCET pin (optional, recommended) | Output | Push-pull, initial Low — toggled around the fast loop for scope timing | — |

**nFAULT EXTI NVIC:** enable the corresponding `EXTIx` interrupt at priority **2** (above the RTOS-maskable range — see Section 8). Its ISR only writes timer registers (`hw_pwm_outputs_disable`) and sets an atomic fault bit; it makes **no RTOS calls**, which is why it may sit above the syscall threshold.

If you also wired nFAULT to TIM1_BKIN (Section 3.4) you get both a hardware kill (break) and the software latch (EXTI) — the recommended belt-and-suspenders configuration.

---

## 7. FreeRTOS (CMSIS-RTOS v2)

Enable **FREERTOS** in Middleware with the **CMSIS_V2** interface.

| Parameter | Value | Notes |
|---|---|---|
| Interface | CMSIS_V2 | The library uses only `osThreadNew`, `osMessageQueueNew`, `osEventFlags*`, `osMutex*`, `osThreadFlags*` |
| TICK_RATE_HZ | 1000 | 1 ms tick — the speed/current tasks use `osDelayUntil` at 1 ms |
| configMAX_SYSCALL_INTERRUPT_PRIORITY | **5** (CubeMX: "LIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY") | Defines the RTOS-maskable boundary used in the NVIC map below |
| USE_PREEMPTION | Enabled | |
| Memory scheme | heap_4 (Cube default) is fine | The *library* allocates nothing; only task/queue creation at init uses the RTOS heap |
| TOTAL_HEAP_SIZE | ≥ 8 KB | Three library tasks + queue + your app tasks |

**Do not create the motor tasks in CubeMX.** Leave only the Cube default task (or delete it). The library creates its own three tasks (`tsk_current`, `tsk_speed`, `tsk_supervisor`) with correct priorities and stack sizes inside `foc_app_init()` → `freertos_tasks.c`. Creating them in CubeMX would duplicate them.

Also verify (CubeMX sets these when FreeRTOS is enabled):
- SysTick used by FreeRTOS; **HAL timebase moved to TIM6** (Section 1)
- `PendSV` and `SysTick` NVIC priorities = 15 (lowest)

---

## 8. NVIC priority map (single source of truth)

Cortex-M33 on U5 uses 4 priority bits (0 = most urgent). With `configMAX_SYSCALL_INTERRUPT_PRIORITY = 5`:

| IRQ | Priority | RTOS calls allowed? | Role |
|---|---|---|---|
| TIM1 Break | **0** | No | Hardware PWM kill on nFAULT/BKIN |
| EXTI (nFAULT) | **2** | **No** (above threshold) | Software fault latch + emergency PWM disable |
| ADC1 (JEOC) — fast loop | **5** | Yes (at threshold) | `foc_ctrl_fast_loop()` + every-20th-cycle thread flag |
| SPI (if IRQ mode; polling is fine) | 10 | Yes | DRV8323 transfers (task context) |
| SysTick / PendSV | 15 | — | RTOS |

Enter these in the CubeMX NVIC tab exactly; the library's init self-check (plan §12 risk (e)) assumes this map.

---

## 9. Code generation and wiring the library in

After **Generate Code**:

1. **`constants.h`**: uncomment and fill every `BSP_*` macro with your generated handle names (`&htim1`, `&hadc1`, `&hspi1`), GPIO ports/pins, and EXTI IRQn. Add `#include "main.h"` at the marked spot.
2. **`main.c`** (inside `USER CODE BEGIN 2`, after all `MX_*_Init()` calls and before `osKernelStart()` — CubeMX generates kernel start when FreeRTOS is enabled):
   ```c
   /* USER CODE BEGIN 2 */
   foc_app_init();   /* creates RTOS objects + tasks, runs library init §6.8 */
   /* USER CODE END 2 */
   ```
3. **ISR routing** — in `stm32u5xx_it.c`, inside the generated handlers' `USER CODE` markers, call the library hooks from `isr_motor.c`:
   ```c
   void ADC1_IRQHandler(void)        { /* USER CODE */ isr_adc_eoc();     /* … */ HAL_ADC_IRQHandler(&hadc1); }
   void TIM1_BRK_IRQHandler(void)    { /* USER CODE */ isr_tim_break();   /* … */ }
   void EXTIx_IRQHandler(void)       { /* USER CODE */ isr_nfault_exti(); /* … */ }
   ```
   (Hook names from `isr_motor.c`: `isr_adc_eoc`, `isr_tim_break`, `isr_nfault_exti`. Call the library hook *before* the HAL generic handler so the PWM kill isn't delayed by HAL dispatch.)
4. Link `libfoc.a` (target CMake build) and add `foc-lib/include` to the include path.

---

## 10. Bring-up order (condensed)

Follow the full HIL checklist (`test/hil/hil_checklist.md`, plan §11.5). Condensed gate order — **do not skip ahead**:

1. **No motor, no power stage load**: verify DRV8323 SPI read-back (`drv8323_init` succeeds, all registers verify).
2. Scope the PWM pins: 20 kHz, center-aligned, dead time visible, outputs dead until MOE set.
3. Trigger a test EXTI/break: gates must drop within 2 µs.
4. Power stage + supply, still no motor: CALIBRATE passes (offsets in range, zero-vector residual < 2% rated).
5. Motor connected, ALIGN-only locked-rotor current step: stable, no oscillation — validates ADC scaling, `hw_cal_t` values, and PI gains end-to-end.
6. Full no-load startup to closed loop; record fast-loop WCET (< 25 µs budget) via the debug pin.

---

## Appendix A — settings that silently break the system

| Mistake | Symptom |
|---|---|
| Edge-aligned instead of center-aligned counter | ADC samples during high-side conduction → garbage currents, immediate OC faults |
| RCR = 0 | Fast loop and ADC trigger at 40 kHz (double rate) → WCET overrun, watchdog fault |
| Compare preload disabled | Duty glitches mid-period → acoustic noise, current spikes |
| TRGO not set to Update Event | ADC never triggers → init self-check fails ("first EOC within 2 PWM periods") |
| SPI mode 0 instead of mode 1 | DRV8323 read-back verify fails at init (`FOC_EFAULT`) |
| ADC interrupt priority above (numerically <) syscall threshold | Hard fault / `configASSERT` on the every-20th-cycle `osThreadFlagsSet` |
| HAL timebase left on SysTick with FreeRTOS | `HAL_Delay` hangs inside ISRs/early init |
| DRV ENABLE pin initial level High | Gate driver powers up before configuration — undefined gate state at boot |
| Idle states set to Set instead of Reset | Both FETs commanded on when MOE drops → shoot-through on the first fault |
