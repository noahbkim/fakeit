import Foundation
import Darwin

// https://github.com/yeokm1/SwiftSerial/blob/master/Sources/SwiftSerial.swift

public enum SerialMode {
    case r
    case w
    case rw
    
    var flag: Int32 {
        switch self {
        case .r:
            return O_RDONLY
        case .w:
            return O_WRONLY
        case .rw:
            return O_RDWR
        }
    }
}

public enum SerialParity {
    case none
    case even
    case odd

    var flag: tcflag_t {
        switch self {
        case .none:
            return tcflag_t(0)
        case .even:
            return tcflag_t(PARENB)
        case .odd:
            return tcflag_t(PARENB | PARODD)
        }
    }
}

public enum SerialDataSize {
    case five
    case six
    case seven
    case eight

    var flag: tcflag_t {
        switch self {
        case .five:
            return tcflag_t(CS5)
        case .six:
            return tcflag_t(CS6)
        case .seven:
            return tcflag_t(CS7)
        case .eight:
            return tcflag_t(CS8)
        }
    }

}

public enum SerialStopSize {
    case one
    case two
}

public enum SerialError : Error {
    case serialPathInaccessible
    case serialClosed
    case serialDisconnected
}

typealias SerialSpecialCharacters = (
    VEOF: cc_t,
    VEOL: cc_t,
    VEOL2: cc_t,
    VERASE: cc_t,
    VWERASE: cc_t,
    VKILL: cc_t,
    VREPRINT: cc_t,
    spare1: cc_t,
    VINTR: cc_t,
    VQUIT: cc_t,
    VSUSP: cc_t,
    VDSUSP: cc_t,
    VSTART: cc_t,
    VSTOP: cc_t,
    VLNEXT: cc_t,
    VDISCARD: cc_t,
    VMIN: cc_t,
    VTIME: cc_t,
    VSTATUS: cc_t,
    spare: cc_t
)

func nullSerialSpecialCharacters() -> SerialSpecialCharacters {
    return (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}


public class Serial {
    public let path: String
    public let fd: Int32
    private var settings: termios
    private var characters: SerialSpecialCharacters
    private var open: Bool

    public init(path: String, mode: SerialMode) throws {
        // Set path, open file
        self.path = path
        self.fd = Darwin.open(self.path, mode.flag | O_NOCTTY | O_EXLOCK)
        
        // Check descriptor
        guard self.fd != -1 else {
            throw SerialError.serialPathInaccessible
        }
        self.open = true
        
        // Get settings defaults
        self.settings = termios()
        self.characters = nullSerialSpecialCharacters()
        tcgetattr(self.fd, &self.settings)
    }
    
    public func set() {
        settings.c_cc = self.characters
        settings.c_cflag |= tcflag_t(CREAD | CLOCAL)  // Turn on the receiver of the serial port
        settings.c_lflag &= ~tcflag_t(ICANON | ECHO | ECHOE | ISIG)  // Turn off canonical mode
        settings.c_oflag &= ~tcflag_t(OPOST)
        settings.c_iflag &= ~tcflag_t(IXON | IXOFF | IXANY)
        settings.c_cflag &= ~tcflag_t(CRTS_IFLOW)
        settings.c_cflag &= ~tcflag_t(CCTS_OFLOW)
        tcsetattr(self.fd, TCSANOW, &self.settings)
    }
    
    public func set(speed: speed_t) {
        cfsetispeed(&self.settings, speed)
        cfsetospeed(&self.settings, speed)
    }

    public func set(dataSize: SerialDataSize, stopSize: SerialStopSize) {
        settings.c_cflag &= ~tcflag_t(CSIZE)
        settings.c_cflag |= dataSize.flag
        switch (stopSize) {
        case .one:
           settings.c_cflag &= ~tcflag_t(CSTOPB)
        case .two:
           settings.c_cflag |= tcflag_t(CSTOPB)
        }
    }

    public func set(parity: SerialParity) {
        self.settings.c_cflag |= parity.flag;
    }
    
    public func set(minimumRead: Int) {
        self.characters.VMIN = cc_t(minimumRead)
    }
    
    public func set(timeout: Int) {
        self.characters.VTIME = cc_t(timeout)
    }
    
    public func read(into buffer: UnsafeMutablePointer<UInt8>, size: Int) throws -> Int {
        guard self.open else {
            throw SerialError.serialClosed
        }
        
        var status = stat()
        fstat(self.fd, &status)
        if status.st_nlink != 1 {
            self.open = false
            throw SerialError.serialDisconnected
        }

        return Darwin.read(self.fd, buffer, size)
    }
    
    public func close() {
        Darwin.close(self.fd)
        self.open = false
    }
}
