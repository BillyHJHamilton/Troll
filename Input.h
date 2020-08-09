#pragma once
#include "Geometry.h"

bool is_letter (int tk_code);
bool is_keyboard_key (int tk_code);

void clear_input ();

void handle_next_input ();
void blank_lowercase_input ();
void handle_input_uppercase (char letter);
void handle_input_walk (Vec2 dir);
void handle_input_close ();
