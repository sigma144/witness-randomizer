# Witness Archipelago Client

[Archipelago](https://archipelago.gg/) is a multiworld multi-game randomizer: it allows people to play games where their progress is linked to and controlled by other people playing other games simultaneously, coordinated by a central server.

This is a project that modifies 2016 puzzle game [The Witness](https://store.steampowered.com/app/210970/The_Witness/) so that it can be played as part of an Archipelago multiworld game.

## How to play

To play a randomizer version of The Witness with this project, you will need:

1. **A compatible version of The Witness**
   
   Officially supported versions are: Steam, Epic Games, Humble DRM-Free
   
   Supported Operating System: Windows, 64bit

2. **An executable version of this randomizer**

   Download a prebuilt-version from the [latest release](https://github.com/NewSoupVi/The-Witness-Randomizer-for-Archipelago/releases/latest) page.

   Check the notes on the download page to make sure the version you're downloading is compatible with the Archipelago game that you're joining.

3. **A running Archipelago multiworld to connect to**

   For information on how to create an Archipelago multiworld, see guides on the Archipelago website. In particular, refer to the [Witness game info page](https://archipelago.gg/games/The%20Witness/info/en) and [Witness setup guide](https://archipelago.gg/tutorial/The%20Witness/setup/en).

### Joining a game for the first time

1. Open your local version of The Witness
2. Select "Start a new game" from the in-game menu
   * **IMPORTANT:** Make sure to only randomize a fresh save, and do not solve any panels before the next step
3. Open the randomizer executable
4. Fill in the Archipelago connection details: server (including port number), slot name and (if necessary) slot password
   * These details will be provided by the Archipelago server
5. Click "Connect"
   * This will randomize your running version of the game and link its progress with the Archipelago servers
6. Play!

### Rejoining a previously started game

0. Check that the Archipelago server is running
   * For games hosted on archipelago.gg servers, the server will sleep after 2 hours of inactivity. If you're coming back to an old game, refreshing the room page on the Archipelago server will wake it up again so it can be connected to.
   * **Note** that in some cases, waking up an old game can change the port number of the server you need to connect to.
1. Open your local version of The Witness
2. Load the save from the previous session

   The game will autoload your most recent save, but you can also load a save from any previous multiworld.
3. Open the randomizer executable
4. Click "Load Credentials"
   * This will retrieve the connection data from your save file. Check that the server and port number are correct, as these can in some situations change
5. Click "Connect"
   * This will randomize your running version of the game and link its progress with the Archipelago servers
6. Play!

# Randomizer features

This project makes a number of changes to The Witness. Most of these are configured as part of the Archipelago game setup process. See the [Witness game settings page](https://archipelago.gg/games/The%20Witness/player-settings) on the Archipelago website for info on how to specify these settings for your game.

Some features are not yet supported by the released version of Archipelago, and will only work with beta versions of the server software. These features are indicated.

## Puzzle randomization

One of the main reasons to randomize The Witness is to be able to replay the game with fresh puzzles to solve.

Puzzle randomization is provided by Sigma144's randomizer, which this repository was originally forked from. The original README from this version is available at [README.sigma.md](README.sigma.md)

This randomizer currently supports:

* Sigma normal randomization, for an equivalent experience to the base game with a slight increase in difficulty.
* Sigma expert randomization, for a significantly more challenging experience to the base game with updated and new mechanics.
* No randomization, for the ability to play the base game puzzles, optionally with Archipelago progression and quality-of-life improvements. (beta only)

## Progression

The randomizer synchronizes with Archipelago servers to bring progression to The Witness.

These features will block off access to certain parts of the game until specific conditions have been met across the Archipelago multiworld (e.g. puzzles solved, items collected). Archipelago will ensure that although this is randomized there is always a logical order to complete every game.

As you complete puzzles and puzzle sets, the randomizer will display text on screen to show which item you unlocked, and which world that item belongs to if it wasn't yours.

As other players unlock items in your world, the randomizer will display text on screen to show you what has been unlocked.

### Symbol shuffle

With symbol shuffle on, individual panels are unavailable to solve until the symbols they require have been awarded as Archipelago items

If you don't yet have the symbols required to solve a panel, it will show a bright red background, the word "MISSING" and a graphic representation of the specific symbols that are missing. For some puzzles where a display would not be visible, a subtitle will be displayed with the missing symbol names when you're near to the panel instead.

### Door shuffle

In the base game, doors in The Witness open when a nearby panel is solved.

With door shuffle on, doors will not open based on their original triggers. Instead, each door is added to the Archipelago item pool, and opens when the corresponding item is received.

Depending on the game settings, doors may be received from Archipelago individually ("Complex doors") or as a group of related doors ("Simple doors", for example all doors in the Desert temple at once)

Additionally, with a "Panels" setting, the panels that activate doors, move bridges or trigger other effects such as changing water levels are not enabled until a corresponding item is received.

### Laser shuffle

With laser shuffle on, each laser is added as an item to the Archipelago pool. Lasers aren't activated by their regular mini-panel, instead they are activated remotely when the corresponding item is received from Archipelago.

Note that the mini-panels still send checks to Archipelago to unlock other items in the pool.

### Vault shuffle, discarded panel shuffle

Each of these settings enables additional panels as Archipelago checks:

* Vault shuffle adds the boxes found in the various locked vaults around the island.
* Discarded panels adds the various single discarded panels found scattered around the island.

Note that if these settings are disabled, the panels will still be solvable in the game, but solving them will have no effect on the multiworld.

### Disable non-randomized puzzles

There are some panels that (because of limitations in the game) it's not currently possible to randomize their puzzles.

These include puzzles in:
* Shadows
* Monastery
* Bunker
* Orchard

If this setting is on, these panels will be completely disabled so that you never have to solve a puzzle that's unrandomized.

Instead, the normal effects of these panels are moved to other panels around the world. You can find details of these changes in the [Sigma randomizer documentation](https://github.com/sigma144/witness-randomizer/wiki/Activation-Triggers).

### EP shuffle

With EP shuffle on, competing the games environmental puzzles can send checks to Archipelago. Depending on the settings, this might be a check for every individual EP, or it might be a check for completing every EP on a single face of the black obelisks around the island.

Settings of your Archipelago world also determine whether certain EPs are pre-completed. These will glow when hovering over their start point, unless they also don't do this in vanilla. The symbols on the respective Obelisk will also be lit up.

Hovering over the symbol of an EP on an Obelisk will display its location name.

In Sigma Rando, some EPs are impossible to solve because of the random puzzle generation. This includes the Keep Pressure Plates puzzles, the Mountain Light Bridge puzzles, and some others. These puzzles will automatically have their puzzle symbols removed upon solving them for the first time (have a free "Puzzle Skip" applied to them, essentially) , giving you full control over the solution path and allowing you to solve their associated EPs.

### Goal support

There are four possible goal settings for you to choose, completion of which will send the Victory flag to the Archipelago server.

* Mountaintop Box Short - Solve the Mountaintop Box to the right, usually requiring 7 lasers.
* Mountaintop Box Long - Solve the Mountaintop Box to the left, usually requiring 11 lasers.
* Elevator - Solve the Elevator start panel at the bottom of the Mountain.
* Challenge - Solve the Challenge Vault Box by doing the Challenge speedrun.

Unlike in vanilla, where getting to the Elevator or Challenge before getting 7 or 11 lasers respectively is impossible without glitches, the Record Player and Elevator start panels are hard-locked by the respective amount of required lasers. Having the required amount of lasers activated will immediately unlock those panels without needing to solve the Mountaintop Box.

In your Archipelago settings, it is also possible to change the amount of required lasers for Mountaintop Box and Elevator to any value from 1 to 7, and the amount of required lasers for Mountaintop Box Long and Challenge to any value from 1 to 11.

## Other Archipelago support

### Useful items, junk items, and traps

As well as progression items like symbols, doors and lasers, Archipelago will also award items that don't directly contribute to progression.

#### Puzzle skips

The randomizer will keep track of the number of "puzzle skip" items you have been awarded. You can spend a puzzle skip to replace any puzzle panel in the game with a trivially solvable version.

When standing in front of the panel to skip: Click the starting circle to draw on the panel, then press and hold the `T` button for one second to spend the skip.

Depending on the panel, this will either replace the panel with a "SKIPPED" display with a single line to solve, or may remove all symbols from the panel allowing you to draw any valid line.

<!-- Note about free skips for EP-related puzzles? -->

#### Speed boosts and slownesses

Being awarded a "Speed boost" will give you 20 seconds of movement at double normal speed.

Being awarded a "Slowness" will give you 20 seconds of movement at 60% normal speed.

These bonuses are cumulative: being awarded multiple of the same kind will add to the timer, and being awarded the opposite item will take time away.

This timer will be paused any time you are in "solve mode" - either actively solving a puzzle or even just looking at the scenery.

Any time you have remaining will also be saved along with your game and restored when you reload.

#### Functioning brain

The randomizer fully supports a functioning brain.

### `!collect` support

When players have completed their goals, Archipelago can allow them to "collect" their remaining items from the other worlds. This indicates to other players in the multiworld which locations they don't need to check any more (because the items they're blocking will be useless)

When `!collect` support is turned on, if an item in your world is collected by someone in another world, its panel will be replaced by a "COLLECTED" display with a single line to solve.

Collect support can be turned on by a checkbox in the randomizer client.

## Quality of life updates

As well as the Archipelago integration, the randomizer includes some quality-of-life improvements to make your Witness experience nicer.

### Elevator/Bridge speed

Some elevators and bridges are sped up.
This includes all the ones from Sigma Rando, as well as a few additional ones like the Bunker Elevator, Caves Elevator and Monastery Shutters.

### Boat speed

The boat is faster in The Witness randomizer. For example selecting the 2nd speed notch on the boat speed control will make the boat as fast as the 3rd speed notch did in the base game. The top speed is raised from 2.5 to 6.

### Audiolog hints

Archipelago has a built-in hint system that unlocks hints based on your progress. Additionally, activating audiolog recorders scattered around the island will display a subtitle that may give you information about one of the items or locations in your game.

The number of useful hints available in your game is controlled by your game settings. Not all hints will be useful.

There may be duplicates of each hint. This makes it easier for you to find all the available hints.

Activating an audio log will start playing the usual recording, as well as displaying the hint. You can click an audiolog a second time to stop the recording. The hint will display for exactly 12 seconds no matter what, meaning the subtitles will continue to display even if you immediately turn off the audio log again. Conversely, they will stop playing after some time even if you leave the audio log on.

### Disable challenge timer

The randomizer client allows you to disable the timer that resets the mountain challenge when the two songs finish playing.

Check the checkbox while playing a game to disable this timer.

The regular music will still play for the first 6½ minutes as normal, but you will be able to continue solving after that time.

# Credits

This is a project is based off of [sigma144's](https://github.com/sigma144/witness-randomizer) and [jbzdarkid's](https://github.com/jbzdarkid/witness-randomizer) Witness randomizer projects.

Most of the interesting code is currently in Source\Archipelago.
This project was originally started by [NewSoupVi](https://github.com/newsoupvi) and continued by [Jarno](https://github.com/Jarno458). Additional contributions have been made by [blastron](https://github.com/blastron), and [oddGarrett](https://github.com/gareth) wrote some words.
