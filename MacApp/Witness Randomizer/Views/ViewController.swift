//
//  ViewController.swift
//  Witness Randomizer
//
//  Created by Marquis Kurt on 10/15/22.
//

import Cocoa

class ViewController: NSViewController {
    // MARK: - Storyboard Properties
    @IBOutlet weak var difficultyRadioNormal: NSButton!
    @IBOutlet weak var difficultyRadioExpert: NSButton!
    @IBOutlet weak var difficultyLabel: NSTextField!
    @IBOutlet weak var checkColorblindMode: NSButton!
    @IBOutlet weak var checkDoubleMode: NSButton!
    @IBOutlet weak var textRandomSeed: NSTextField!

    // MARK: - Stored Properties
    var viewModel: any RandomizerViewModelType = AppDelegate.appVM

    override func viewDidLoad() {
        super.viewDidLoad()
        viewModel.onChange { [weak self] in
            self?.updateDifficultyText()
        }
        setInitialState()
    }

    // MARK: - Storyboard Actions
    @IBAction func onDifficultyModeSelect(_ sender: NSButton) {
        viewModel.setDifficulty(from: sender.title)
    }

    @IBAction func onColorblindToggle(_ sender: Any) {
        viewModel.setColorblindMode(activated: checkColorblindMode.state == .on)
    }

    @IBAction func onDoubleToggle(_ sender: Any) {
        viewModel.setDoubleRandomizedMode(activated: checkDoubleMode.state == .on)
    }

    @IBAction func onRandomSeedChange(_ sender: NSTextField) {
        guard let seed = Int(sender.stringValue) else { return }
        viewModel.setRandomSeed(to: seed)
    }

    @IBAction func onSubmitGenerate(_ sender: NSButton) {
        // TODO: Call the view model or other code responsible for the actual randomization here.
    }

    // MARK: - Methods
    private func setInitialState() {
        difficultyRadioNormal.state = (viewModel.getDifficulty() == .normal ? .on : .off)
        difficultyRadioExpert.state = (viewModel.getDifficulty() == .expert ? .on : .off)
        updateDifficultyText()
        checkColorblindMode.state = (viewModel.getColorblindMode() ? .on : .off)
        checkDoubleMode.state = (viewModel.getDoubleRandomizedMode() ? .on : .off)
        textRandomSeed.stringValue = String(viewModel.getRandomSeed())
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

