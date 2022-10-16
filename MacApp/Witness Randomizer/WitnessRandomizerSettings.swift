//
//  WitnessRandomizerSettings.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Foundation

struct WitnessRandomizerSettings {
    enum Difficulty: String {
        case normal
        case expert

        var modeText: String {
            "\(self.rawValue.capitalized) Mode"
        }
    }
    var difficulty: Difficulty
    var doubleModeEnabled: Bool
    var colorblindMode: Bool
    var seedNumber: Int

    init(difficulty: Difficulty, doubleModeEnabled: Bool = false, colorblindMode: Bool = false, seedNumber: Int = 0) {
        self.difficulty = difficulty
        self.doubleModeEnabled = doubleModeEnabled
        self.colorblindMode = colorblindMode
        self.seedNumber = seedNumber
    }

    init(from userDefaults: UserDefaults) {
        if let difficultyString = userDefaults.string(forKey: "difficulty") {
            self.difficulty = Difficulty(rawValue: difficultyString) ?? .normal
        } else {
            self.difficulty = .normal
        }
        self.doubleModeEnabled = userDefaults.bool(forKey: "double-mode")
        self.colorblindMode = userDefaults.bool(forKey: "colorblind-mode")
        self.seedNumber = userDefaults.integer(forKey: "seed")
    }

    func save(to defaults: UserDefaults) {
        defaults.set(difficulty.rawValue, forKey: "difficulty")
        defaults.set(doubleModeEnabled, forKey: "double-mode")
        defaults.set(colorblindMode, forKey: "colorblind-mode")
        defaults.set(seedNumber, forKey: "seed")
    }
}

extension WitnessRandomizerSettings {
    static let defaultSettings = WitnessRandomizerSettings(difficulty: .normal)
    static let userDefaults = WitnessRandomizerSettings(from: .standard)
}
