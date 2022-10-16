//
//  ViewController.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Cocoa

class ViewController: NSViewController {
    @IBOutlet weak var difficultyRadioNormal: NSButton!
    @IBOutlet weak var difficultyRadioExpert: NSButton!
    @IBOutlet weak var difficultyLabel: NSTextField!

    var viewModel = RandomizerViewModel(settings: .defaultSettings)

    override func viewDidLoad() {
        super.viewDidLoad()
        viewModel.bind { [weak self] in
            self?.updateDifficultyText()
        }
        setInitialState()
    }

    @IBAction func onDifficultyModeSelect(_ sender: NSButton) {
        viewModel.setDifficulty(from: sender.title)
    }

    private func setInitialState() {
        difficultyRadioNormal.state = (viewModel.getDifficulty() == .normal ? .on : .off)
        difficultyRadioExpert.state = (viewModel.getDifficulty() == .expert ? .on : .off)
        updateDifficultyText()
    }

    private func updateDifficultyText() {
        switch viewModel.getDifficulty() {
        case .normal:
            difficultyLabel.stringValue = NSLocalizedString(
                "difficulty.normal.text",
                comment: "Normal difficulty text"
            )
        case .expert:
            difficultyLabel.stringValue = NSLocalizedString(
                "difficulty.expert.text",
                comment: "Expert difficulty text"
            )
        }
    }

}

