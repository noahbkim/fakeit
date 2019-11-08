import Cocoa

class FakeItViewController: NSViewController {
    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    @IBAction func kick(_ sender: NSButton) {
        DrumSampler.shared.kick()
    }
}
