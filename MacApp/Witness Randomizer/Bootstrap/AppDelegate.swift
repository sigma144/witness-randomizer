//
//  AppDelegate.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Cocoa

@main
class AppDelegate: NSObject, NSApplicationDelegate {
    static var appVM: RandomizerViewModel = .init(settings: .userDefaults)

    func applicationDidFinishLaunching(_ aNotification: Notification) {
        // Insert code here to initialize your application
    }

    func applicationWillTerminate(_ aNotification: Notification) {
        // Insert code here to tear down your application
        Self.appVM.saveToUserDefaults()
    }

    func applicationShouldTerminate(_ sender: NSApplication) -> NSApplication.TerminateReply {
        return .terminateNow
    }

    func applicationSupportsSecureRestorableState(_ app: NSApplication) -> Bool {
        return true
    }
}

