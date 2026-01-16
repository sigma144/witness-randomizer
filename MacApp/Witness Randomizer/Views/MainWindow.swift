//
//  MainWindow.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/16/22.
//

import Cocoa

class MainWindow: NSWindowController {
    @IBAction func onClickGenerate(_ sender: NSToolbarItem) {
        updateSubtitleIfPresent(to: "To be implemented")
    }

    func updateSubtitleIfPresent(to text: String) {
        if #available(macOS 11.0, *) {
            window?.subtitle = text
        }
    }
}
