import Foundation

var serial: Serial;

// Create the serial socket
do {
    serial = try Serial(path: "/dev/tty.usbmodem14201", mode: .r)
} catch SerialError.serialOpenFailed(let errno) {
    print("Failed to open serial: \(errno)")
    exit(1)
} catch {
    print("Failed to open serial")
    exit(1)
}

// Configure
serial.set(speed: speed_t(B9600))
serial.set(dataSize: .eight, stopSize: .one)
serial.set(minimumRead: 1)

// Commit
do {
    try serial.set()
} catch SerialError.serialBlockingFailed(let errno) {
    print("Failed to configure serial: \(errno)")
    exit(1)
} catch {
    print("Failed to configure serial")
    exit(1)
}

defer { serial.close() }

let sampler = DrumSampler()

let monitor = SerialMonitor(serial)
monitor.listen() { byte in
    if byte == 107 {
        sampler.kick()
    }
}
