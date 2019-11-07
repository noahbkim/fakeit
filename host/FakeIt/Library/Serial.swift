import Foundation
import Darwin

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

public class Serial {
    public let path: String
    public let fd: Int32
    private var settings: termios

    public init(path: String, mode: SerialMode) {
        self.path = path
        self.fd = Darwin.open(self.path, mode.flag | O_NOCTTY | O_EXLOCK)
        self.settings = termios()
        tcgetattr(self.fd, &self.settings)
    }
    
    private func set() {
        tcsetattr(self.fd, TCSANOW, &self.settings)
    }
    
    public func set(speed: speed_t) {
        cfsetispeed(&self.settings, speed)
        cfsetospeed(&self.settings, speed)
        self.set()
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
        self.set()
    }

    public func set(parity: SerialParity) {
        self.settings.c_cflag |= parity.flag;
        self.set()
    }
}
