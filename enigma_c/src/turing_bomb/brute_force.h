#pragma once

//
// Created by Emanuel on 29.07.2024.
//

typedef struct Node Node;

// EnigmaConfiguration* pop(void);
void crack_enigma(const char *known_plaintext, char *encrypted_text);
