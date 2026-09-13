# ESE-301 Week 02 — System Architecture

## Temp to SD Card: Drivers and Abstractions

``` mermaid
classDiagram
    class ApplicationLayer {
        TempLogger_read()
        sd_write()
    }
    class TempSensorDriver {
        +temp_open() Init I2C and power up
        +temp_close() Power down and release bus
        +temp_read() Read raw sensor register data
        +temp_write() Write config registers
        +temp_ioctl() Set resolution and rate
    }
    class SDCardDriver {
        +sd_open() Init SPI send CMD0 CMD8
        +sd_close() Flush and deselect CS
        +sd_read() Read 512 byte block
        +sd_write() Write 512 byte block
        +sd_ioctl() Get capacity and sync
    }
    class I2C2Registers {
        +GPIOx
        +I2C_CR1
        +I2C_TXDR/RXDR
    }
    class SPI1Registers {
        +CS_GPIO
        +SPI_DR
        +DMA_channel
    }
    class TempSensorIC
    class SDCardSlot

    ApplicationLayer --> TempSensorDriver : temp_read
    ApplicationLayer --> SDCardDriver : sd_write
    TempSensorDriver --> I2C2Registers : register access
    SDCardDriver --> SPI1Registers : register access
    I2C2Registers --> TempSensorIC : hardware
    SPI1Registers --> SDCardSlot : hardware
```

## System Architecture

``` mermaid
graph TD
    ISR["Timer ISR - Hardware tick 1Hz"]
    CTL["Controller - Fires on timer tick, tracks timestamps"]
    MDL["Model - TempLogger - read(), getLastSample(), degC = convert(raw)"]
    SEN["Temp sensor - I2C peripheral"]
    SRV["Serial debug view - Formats output, writes to UART log - REPLACE in real build"]
    UART["File: UART log - 25.4C at 00:00:01"]
    SDV["SD card view - Writes CSV rows - KEEP unmodified in sandbox"]
    SDFILE["File: SD log CSV - 00:00:01, 25.4, 0x1A3"]

    ISR -->|tick| CTL
    CTL -->|take reading| MDL
    MDL -.->|ready| CTL
    CTL -->|update display| SRV
    MDL -->|model changed| SRV
    SRV -.->|get current state| MDL
    MDL -->|model changes on tick| SDV
    SEN -->|raw temp register data| MDL
    SRV --> UART
    SDV --> SDFILE
```
