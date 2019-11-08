import Foundation

// https://medium.com/@mohit.bhalla/thread-safety-in-ios-swift-7b75df1d2ba6

public class Queue<T> {
    private var items: [T] = []
    private let access: DispatchQueue = DispatchQueue(label: "QueueAccess", attributes: .concurrent)
    
    public func put(_ item: T) {
        self.access.async(flags: .barrier) {
            self.items.insert(item, at: 0)
        }
    }
    
    public func get() -> T? {
        var item: T?
        self.access.async(flags: .barrier) {
            item = self.items.popLast()
        }
        return item
    }
    
    public var count: Int {
        var count = 0
        self.access.async(flags: .barrier) {
            count = self.items.count
        }
        return count
    }
}
