void CC1101::writeCommand(uint8_t cmd) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        return; // Return a safe default
    }
    // ... existing logic
}

void CC1101::writeRegister(uint8_t reg, uint8_t value) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        return; // Return a safe default
    }
    // ... existing logic
}

uint8_t CC1101::readRegister(uint8_t reg) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        return 0; // Return a safe default
    }
    // ... existing logic
}

uint8_t CC1101::readRegisterMedian3(uint8_t reg) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        return 0; // Return a safe default
    }
    // ... existing logic
}

void CC1101::writeBurstRegister(uint8_t reg, uint8_t *values, uint8_t length) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        return; // Return a safe default
    }
    // ... existing logic
}

void CC1101::readBurstRegister(uint8_t reg, uint8_t *buffer, uint8_t length) {
    spi_waitMiso();
    if (!spi_waitMiso()) {
        deselect();
        memset(buffer, 0, length); // Fill buffer with 0 on timeout
        return; // Return without reset
    }
    // ... existing logic
}

#define CC1101_RX_WAIT_TIMEOUT_MS 2 // Tightened timeout

int CC1101::receiveData(uint8_t *buffer, size_t length) {
    if (rxBytes == 0) {
        return 0; // Return immediately if no data
    }
    while (rxBytes > 0 && rxBytes < length) {
        // ... existing logic for waiting
    }
    // ... existing overflow handling
}
