import Cocoa

// https://medium.com/@prasanna.aithal/multi-threading-in-ios-using-swift-82f3601f171c

class FakeItViewController: NSViewController {
    @IBOutlet weak var devicePopUp: NSPopUpButton!
    @IBOutlet weak var connectButton: NSButton!
    @IBOutlet weak var logText: NSTextView!
    
    private var devices: [String] = []
    private var monitor: SerialMonitor? = nil
    private var thread: Thread? = nil
    
    override public func viewDidLoad() {
        super.viewDidLoad()
        self.logText.isEditable = false
        self.populateDevicePopUp()
    }
    
    override public func viewDidAppear() {
        super.viewDidAppear()
        self.view.window?.title = "FakeIt"
    }
    
    private func populateDevicePopUp() {
        self.devicePopUp.removeAllItems()
        if let paths = try? Serial.list() {
            self.devices = paths
            paths.forEach(self.devicePopUp.addItem)
            if paths.count == 0 {
                self.devicePopUp.addItem(withTitle: "No devices found")
            }
        } else {
            self.log("Could not scan for devices!")
        }
        self.updateControls()
    }
    
    private func updateControls() {
        self.connectButton.isEnabled = self.devices.count > 0
        self.connectButton.title = self.thread == nil ? "Connect" : "Disconnect"
    }
    
    @IBAction func onRefresh(_ sender: Any) {
        self.populateDevicePopUp()
    }
    
    @IBAction func onConnect(_ sender: NSButton) {
        if self.isMonitoring() {
            self.stopMonitor()
        } else if let selectedItem = self.devicePopUp.selectedItem {
            self.startMonitor("/dev/\(selectedItem.title)")
        } else {
            self.log("No device selected")
        }
        self.updateControls()
    }
}

// Serial
extension FakeItViewController {
    
    private func startMonitor(_ device: String) {
        let thread = Thread.init(target: self, selector: #selector(runMonitor), object: device)
        self.thread = thread
        thread.start()
    }
    
    private func createSerial(path: String) -> Serial? {
        var serial: Serial;
        
        // Create the serial socket
        do {
            serial = try Serial(path: path, mode: .r)
        } catch SerialError.serialOpenFailed(let errno) {
            self.logAsync("Failed to open serial: \(errno)")
            return nil
        } catch {
            self.logAsync("Failed to open serial")
            return nil
        }
        
        // Configure
        serial.set(speed: speed_t(B9600))
        serial.set(dataSize: .eight, stopSize: .one)
        serial.set(minimumRead: 1)
        
        // Commit
        do {
            try serial.set()
        } catch SerialError.serialBlockingFailed(let errno) {
            self.logAsync("Failed to configure serial: \(errno)")
            return nil
        } catch {
            self.logAsync("Failed to configure serial")
            return nil
        }
        
        return serial
    }

    /// List all devices with the given prefix
    @objc
    private func runMonitor(_ path: String) {
        self.logAsync("Connecting to \(path)")
        guard let serial = self.createSerial(path: path) else { return }
        defer { serial.close() }

        let monitor = SerialMonitor(serial)
        self.monitor = monitor
        monitor.listen() { byte in
            if byte == 107 {
                DispatchQueue.main.async {
                    DrumSampler.shared.kick()
                }
            }
        }
    }
    
    private func isMonitoring() -> Bool {
        return self.monitor != nil
    }
    
    private func stopMonitor() {
        if let monitor = self.monitor, let thread = self.thread {
            monitor.kill()
            thread.cancel()
            self.monitor = nil
            self.thread = nil
            self.updateControls()
            self.log("Done monitoring!")
        }
    }
}

// Logging
extension FakeItViewController {
    private func logClear() {
        self.logText.string = ""
    }
    
    private func logAsync(_ string: String, end: String = "\n") {
        DispatchQueue.main.async {
            self.log(string, end: end)
        }
    }
    
    private func log(_ string: String, end: String = "\n") {
        self.logText.string += string + end
    }
}
