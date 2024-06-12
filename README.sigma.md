![](https://github.com/sigma144/witness-randomizer/blob/master/example.png)

## Witness Random Puzzle Generator

The Witness Random Puzzle Generator takes most of the puzzles in Jonathan Blow's "The Witness" and makes them into new, randomly generated puzzles. The mod requires the game in order to function. No game files will be altered, as the randomizer works enterly in RAM. This mod uses WinAPI, so it is only compatible with Microsoft Windows. There is a program you can use that might make it work with Linux; see https://marugawa.dk/witness-randomizer-on-linux for more details.

This project is a fork of jbzdarkid's puzzle randomizer at https://github.com/jbzdarkid/witness-randomizer. His discoveries about how puzzle data is encoded in The Witness were invaluable to this project, so a huge thank you to jbzdarkid for his contributions.

Shadows, Monastery, and Greenhouse are non-randomized in this mod. The lasers for those areas will be activated as you solve optional puzzles throughout the island. [Click here](https://github.com/sigma144/witness-randomizer/wiki/Activation-Triggers) for a more detailed description.

**How to use the Witness Random Puzzle Generator:**

1. Launch The Witness
2. Start a New Game
3. Launch WitnessRPG.exe (from any location)
4. Choose difficulty and seed (leave seed blank for random)
5. Click "Randomize" and wait for the process to complete
6. Enjoy your randomly created puzzles! Puzzles will revert to normal after closing the game.

**To resume where you left off after closing and relaunching the game:**

1. Load the save you were previously playing on, if it is not already loaded
2. Launch WitnessRPG.exe (from any location)
3. Click "Randomize" and wait for the process to complete. You don't have to enter the seed or difficulty again. This is because the seed and difficulty were stored in your save file when you randomized initially, so they will be automatically restored and used.


If you get stuck on a puzzle, the solver provided in the Solver folder might be able to help you. However I can't guarantee it will always work correctly, and there are some mechanics from Expert it doesn't handle. Another option if you get stuck is to exit and reopen the game, solve the puzzle the normal way, then run the randomizer again.

Please report any issues (such as unsolvable/broken puzzles) on the issues page. However, please read the **"Known Issues"** section in the wiki first, as your issue may be addressed there.

Thanks for playing this mod, and good luck!
