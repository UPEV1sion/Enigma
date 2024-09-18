
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Wall -Wextra -Werror -O3 -std=c17")

add_library(enigma_lib
    cyclometer/cyclometer.c
    cyclometer/cyclometer.h
    enigma/enigma.c
    enigma/enigma.h
    enigma/rotor/rotor.c
    enigma/rotor/rotor.h
    enigma/reflector/reflector.c
    enigma/reflector/reflector.h
    enigma/plugboard/plugboard.c
    enigma/plugboard/plugboard.h
    cli/cli.c
    cli/cli.h
    helper/helper.c
    helper/helper.h
        gui/enigma_gui.c
        gui/enigma_gui.h
        json/json.h
        json/json.c
        turing_bomb/brute_force.h
        turing_bomb/brute_force.c
        gui/bomb_out.c
        gui/bomb_out.h
        gui/start_gui.c
        gui/start_gui.h
        gui/bomb_gui.c
        gui/bomb_gui.h
        turing_bomb/diagonal_board.h
        turing_bomb/diagonal_board.c
        turing_bomb/cycle_finder.c
        turing_bomb/cycle_finder.h
        helper/hashmap.c
        helper/hashmap.h
        turing_bomb/turing_bomb.c
        turing_bomb/turing_bomb.h
)

target_include_directories(enigma_lib PUBLIC .)
