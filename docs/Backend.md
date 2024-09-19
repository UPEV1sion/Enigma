# Backend

The whole project is written in C and using CMake as its build system.
This is the version WITH GUI and JSON support.
To use a version WITHOUT the need for any dependencies,
click [here](https://github.com/UPEV1sion/Enigma_CLI).
A small introduction is given in the docs folder on how to build the project using CMake. 

## Modes
The backend can be viewed as three smaller programs.
The core of all three parts is the enigma component.
All three modes use this enigma component to provide different behaviors, controlled by command-line arguments. 
Running `./build/enigma` or `./build/enigma -h/--help` lists all available arguments.

### CLI
The CLI component was created to have an easy way to debug the functionality of the enigma component. It is possible to configure an entire enigma using command-line arguments or using the interactive mode. The interactive mode is activated by running the program like `enigma -i/--interactive`. You will be asked for each configurable option on how you want to set it.
Another way to use CLI is by configuring all options using CLI arguments. An example is provided in the root of the project. 

To learn more about the CLI read the [documentation](CLI.md).


### Cyclometer
The cyclometer component creates the cycles and counts them for the given enigma configurations. 

To learn more about the Cyclometer read the [documentation](Cyclometer.md).

### Turing Bomb
The turing bomb component is based on the [polish bomba](https://en.wikipedia.org/wiki/Bomba_(cryptography)). 
It takes advantage of the peculiarities of the Enigma — for example, a letter is never depicted on itself. 

To learn more about the Turing Bomb, read the [documentation](TuringBomb.md).

## Enigma 
To learn more about the inner workings of the Enigma and how it is implemented in C: click [here](Enigma.md)

