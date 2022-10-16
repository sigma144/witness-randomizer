//
//  RandomizerViewModel.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Foundation

class RandomizerViewModel {
    typealias UpdateHandlerType = () -> Void
    private var settings: WitnessRandomizerSettings = .defaultSettings {
        didSet {
            updateHandler?()
        }
    }
    private var updateHandler: UpdateHandlerType?

    init(settings: WitnessRandomizerSettings) {
        self.settings = settings
    }

    func bind(handler: @escaping () -> Void) {
        self.updateHandler = handler
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
}
