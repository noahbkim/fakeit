import Cocoa

// https://medium.com/@prasanna.aithal/multi-threading-in-ios-using-swift-82f3601f171c

let DEVICE_PREFIX = "cu."

class FakeItViewController: NSViewController {
    @IBOutlet weak var devicePopUp: NSPopUpButton!
    @IBOutlet weak var connectButton: NSButton!
    @IBOutlet var logText: NSTextView!
    
    private var receiver: Thread? = nil
    private var kill: Bool = false
    
    override func viewDidLoad() {
        super.viewDidLoad()
        self.scanDevices()
        self.logText.isEditable = false
        self.connectButton.title = "Connect"
    }
    
    private func scanDevices() {
        devicePopUp.removeAllItems()
        if let paths = try? FileManager.default.contentsOfDirectory(atPath: "/dev") {
            if (paths.count == 0) {
                devicePopUp.addItem(withTitle: "No devices found!")
            } else {
                for path in paths {
                    if (path.starts(with: DEVICE_PREFIX)) {
                        devicePopUp.addItem(withTitle: path)
                    }
                }
            }
        } else {
            devicePopUp.addItem(withTitle: "Error scanning devices!")
        }
        devicePopUp.addItem(withTitle: "Scan again")
    }
    
    private func logClear() {
        self.logText.string = ""
    }
    
    private func logPrint(_ string: String, end: String = "\n") {
        self.logText.string += string + end
    }
    
    @objc
    private func serialScan(_ device: String) {
        let path = "/dev/\(device)"
        DispatchQueue.main.async {
            self.logPrint("Connecting to \(path)")
        }
        defer {
            DispatchQueue.main.async {
                self.logPrint("Done receiving")
                self.connectButton.title = "Connect"
                self.kill = false
            }
        }
        
        guard let serial = try? Serial(path: path, mode: .r) else {
            DispatchQueue.main.async {
                self.logPrint("Failed to open serial")
            }
            return
        }
        serial.set(speed: speed_t(B9600))
        serial.set(dataSize: .eight, stopSize: .one)
        serial.set(minimumRead: 1)
        serial.set()

        defer {
            serial.close()
        }

        let buffer = UnsafeMutablePointer<UInt8>.allocate(capacity: 1)
        var read = 0
        while (!self.kill) {
            do {
                read = try serial.read(into: buffer, size: 1)
            } catch let error {
                DispatchQueue.main.async {
                    self.logPrint("Error reading into buffer: \(error)")
                }
                return
            }
            if read > 0 {
                DispatchQueue.main.async {
                    self.logPrint(String(bytes: [buffer[0]], encoding: .utf8) ?? "?", end: "")
                }
            }
        }
    }
    
    @IBAction func onSelectDevice(_ sender: NSPopUpButton) {
        guard let selectedItem = self.devicePopUp.selectedItem else { return }
        if selectedItem.title == "Scan again" {
            scanDevices()
        } else if selectedItem.title.starts(with: DEVICE_PREFIX) {
            self.connectButton.isEnabled = true
        } else {
            self.connectButton.isEnabled = false
        }
    }
    
    @IBAction func onConnect(_ sender: NSButton) {
        if let receiver = self.receiver {
            self.kill = true
            receiver.cancel()
            self.receiver = nil
        } else {
            if let selectedItem = self.devicePopUp.selectedItem, selectedItem.title.starts(with: DEVICE_PREFIX) {
                let receiver = Thread.init(target: self, selector: #selector(serialScan), object: selectedItem.title)
                self.receiver = receiver
                receiver.start()
            } else {
                self.logPrint("No device selected")
            }
            self.connectButton.title = "Disconnect"
        }
    }
    
    @IBAction func kick(_ sender: NSButton) {
        DrumSampler.shared.kick()
    }
}
