#pragma once

//
// Created by Emanuel on 11.1.25.
//

#include <libpq-fe.h>
#include "cyclometer/server_cyclometer.h"

PGresult* query_db(const S_Cycle *cycles);
void init_db(void);
void close_db(void);
