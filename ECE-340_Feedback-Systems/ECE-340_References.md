# ECE-340 — Vendor And Reference Documentation

The course text and vendor manuals are **not stored in this repository**.
`ECE-340_Feedback-Systems/books/` is git-ignored; download the documents below
into it if you want them locally.

## Course Text

- Karl Johan Åström and Richard M. Murray, *Feedback Systems: An Introduction
  for Scientists and Engineers*, Princeton University Press, 2008
  (ISBN 978-0-691-13576-2).
  - [Electronic edition v2.10b (22 Feb 2009)](https://www.cds.caltech.edu/~murray/books/AM05/pdf/am08-complete_22Feb09.pdf)
    — the version whose page numbers the syllabus cites. PDF page = book page + 12.
  - [Princeton University Press](https://press.princeton.edu/titles/8701.html)
    — print edition. Copyright is held by the publisher; the electronic edition
    is for personal use.

  See [ECE-340_Feedback-Systems_Syllabus.md](ECE-340_Feedback-Systems_Syllabus.md).

## Companion Material

| Resource | Covers |
| --- | --- |
| [FBSwiki (first edition)](http://www.cds.caltech.edu/~murray/amwiki/) | Per-chapter exercises, FAQs, errata, and lecture notes |
| [Companion MATLAB sources](http://www.cds.caltech.edu/~murray/books/AM05/matlab/) | Scripts behind the book's figures. `intro/cruise_response.m` produces Figure 1.13; `cruisedyn.m` is the §3.1 vehicle model |
| [fbsbook.org](http://fbsbook.org) | Second edition of the text (different page numbering) |

## Target Hardware (Weeks 11–15)

| Document | ID | Covers |
| --- | --- | --- |
| [STM32U575/585 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf) | RM0456 | General-purpose timers for the fixed-rate control interrupt; USART for logging |
| [NUCLEO-U575ZI-Q User Manual](https://www.st.com/resource/en/user_manual/um2861-stm32u5-nucleo144-board-mb1549-stmicroelectronics.pdf) | UM2861 | Board pinout and ST-LINK virtual COM port |
| [Arm Cortex-M33 Devices Generic User Guide](https://developer.arm.com/documentation/100235/latest/) | Arm 100235 | FPU and NVIC behavior relevant to control-loop timing |

## Tools

- [GNU Octave](https://octave.org/) — all simulation and design work. The
  Week 1 scripts use only core Octave (`ode45`, `expm`, `fzero`); no packages
  are required.
- [Octave `control` package](https://gnu-octave.github.io/packages/control/)
  — optional from Week 8 on (`tf`, `bode`, `nyquist`, `margin`).
- [Arm GNU Toolchain (arm-none-eabi)](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
  — firmware in Weeks 11–15.
