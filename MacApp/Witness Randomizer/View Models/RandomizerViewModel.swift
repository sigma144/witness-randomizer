//
//  RandomizerViewModel.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Foundation

protocol ViewModelType: AnyObject {
    var updateHandler: (() -> Void)? { get set }
    func onChange(handler: @escaping () -> Void)
}

protocol RandomizerVMAttributes {
    var settings: WitnessRandomizerSettings { get set }
    func saveToUserDefaults()
    func getDifficulty() -> WitnessRandomizerSettings.Difficulty
    func setDifficulty(from senderName: String)
    func getColorblindMode() -> Bool
    func setColorblindMode(activated: Bool)
    func getDoubleRandomizedMode() -> Bool
    func setDoubleRandomizedMode(activated: Bool)
    func getRandomSeed() -> Int
    func setRandomSeed(to seedNumber: Int)
}

typealias RandomizerViewModelType = ViewModelType & RandomizerVMAttributes

class RandomizerViewModel {
    typealias UpdateHandlerType = () -> Void
    internal var settings: WitnessRandomizerSettings = .defaultSettings {
        didSet {
            updateHandler?()
        }
    }
    var updateHandler: UpdateHandlerType?

    init(settings: WitnessRandomizerSettings) {
        self.settings = settings
    }
}

extension RandomizerViewModel: ViewModelType {
    func onChange(handler: @escaping UpdateHandlerType) {
        self.updateHandler = handler
    }
}

extension RandomizerViewModel: RandomizerVMAttributes {
    func getColorblindMode() -> Bool {
        return settings.colorblindMode
    }

    func getDoubleRandomizedMode() -> Bool {
        return settings.doubleModeEnabled
    }

    func getRandomSeed() -> Int {
        return settings.seedNumber
    }

    func saveToUserDefaults() {
        settings.save(to: .standard)
    }

    func getDifficulty() -> WitnessRandomizerSettings.Difficulty {
        return settings.difficulty
    }

    func setDifficulty(from senderName: String) {
        switch senderName {
        case WitnessRandomizerSettings.Difficulty.normal.modeText:
            settings.difficulty = .normal
        case WitnessRandomizerSettings.Difficulty.expert.modeText:
            settings.difficulty = .expert
        default:
            break
        }
    }

    func setColorblindMode(activated: Bool) {
        settings.colorblindMode = activated
    }

    func setDoubleRandomizedMode(activated: Bool) {
        settings.doubleModeEnabled = activated
    }

    func setRandomSeed(to seed: Int) {
        settings.seedNumber = seed
    }
}
