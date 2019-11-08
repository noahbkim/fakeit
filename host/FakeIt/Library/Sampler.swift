import Foundation
import AVFoundation

public enum Drum {
    case kick
    case snare
}

// https://stackoverflow.com/questions/34680007/simple-low-latency-audio-playback-in-ios-swift

public class Sampler {
    private let engine: AVAudioEngine
    private let playerNode: AVAudioPlayerNode
    private let mixerNode: AVAudioMixerNode
        
    public init() {
        self.engine = AVAudioEngine()
        self.playerNode = AVAudioPlayerNode()
        self.engine.attach(self.playerNode)
        self.mixerNode = self.engine.mainMixerNode
        self.engine.connect(playerNode, to: self.mixerNode, format: self.mixerNode.outputFormat(forBus: 0))
        
        do {
            try engine.start()
        } catch let error {
            print("Error starting engine: \(error.localizedDescription)")
        }
    }
    
    public func play(file: AVAudioFile) {
        self.engine.connect(self.playerNode, to: self.mixerNode, format: file.processingFormat)
        self.playerNode.scheduleFile(file, at: nil, completionHandler: nil)
        if engine.isRunning {
            self.playerNode.play()
        }
    }
}

public func loadAudioFile(_ forResource: String, _ withExtension: String) -> AVAudioFile? {
    let url = Bundle.main.url(forResource: forResource, withExtension: withExtension)!
    do {
        return try AVAudioFile(forReading: url)
    } catch let error {
        print("Error loading audio: \(error)")
        return nil
    }
}

public class DrumSampler : Sampler {
    private let kickFile = loadAudioFile("kick", ".wav")!
    private let snareFile = loadAudioFile("snare", ".wav")!
    
    public static let shared = DrumSampler()
    
    public func kick() {
        self.play(file: self.kickFile)
    }
    
    public func snare() {
        self.play(file: self.snareFile)
    }
}
