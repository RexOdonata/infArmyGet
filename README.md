# infArmyGet
Cross-platform CLI tool to make downloading infinity army data a little bit easier.

## Program Usage

Usage: infArmyGet [--help] [--version] [-id VAR...] [-meta] [-list]

Infinity Army Getter - CLI tool for downloading army data.
To launch UI do not use any arguments.


Optional arguments:
  -h, --help     shows help message and exits 
  -v, --version  prints version information and exits 
  -id            Enter ID(s) of faction as program argument(s) to skip UI [nargs: 0 or more] 
  -meta          Download and save army metadata 
  -list          Print list of factions with IDs 

## UI Usage

In UI mode type a command:

faction
list
meta
exit

If you type 'faction' you can start typing the name of a faction and press tab, the UI will suggest/autocomplete factions based on what you've already typed.

you can skip entering the faction menu by chaining commands, ex: 'faction starmada' instead of 'faction'->'starmada'

## Building

Very simple build with CMake:

1. cmake . (in folder where repo is downloaded)
2. make infArmyGet

I encourage installing CPR via a package manager if available as it takes a long time to build, and if downloaded to your cmake folder it will introduce a bunch of it's own build targets.
