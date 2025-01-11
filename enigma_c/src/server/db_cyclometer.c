#include "db_cyclometer.h"

#include <assert.h>

//
// Created by Emanuel on 1/11/25.
//

enum
{
    BUFFER_SIZE = 1024
};

static PGconn *conn;

#define QUERY_STR "SELECT * FROM cycles.catalogue WHERE \"1st_rotor_cycles\" @> '{%s}'" \
" and \"2nd_rotor_cycles\" @> '{%s}'" \
" and \"3rd_rotor_cycles\" @> '{%s}'"

static void get_cycle_str(char *buffer, const S_Cycle *cycle)
{
    if (cycle->length <= 0) return;
    size_t offset = 0;

    offset += snprintf(buffer, BUFFER_SIZE, "%d", cycle->cycle_values[0]);
    for (int i = 1; i < cycle->length; ++i)
    {
        offset += snprintf(buffer + offset, BUFFER_SIZE - offset, ",%d", cycle->cycle_values[i]);
    }
}

PGresult* query_db(const S_Cycle *cycles)
{
    // static PGconn *conn = PQconnectdb("user=escha password=es2903 dbname=cyclometer host=localhost"); //TODO insert credentials
    // static_assert(PQstatus(conn) != CONNECTION_OK);
    // if (PQstatus(conn) != CONNECTION_OK)
    // {
    //     fprintf(stderr, "Connection failed %s\n", PQerrorMessage(conn));
    //     PQfinish(conn);
    //     exit(1);
    // }

    char query[BUFFER_SIZE];
    char cycles_1_rotor[BUFFER_SIZE];
    char cycles_2_rotor[BUFFER_SIZE];
    char cycles_3_rotor[BUFFER_SIZE];
    get_cycle_str(cycles_1_rotor, cycles + 0);
    get_cycle_str(cycles_2_rotor, cycles + 1);
    get_cycle_str(cycles_3_rotor, cycles + 2);

    snprintf(query, BUFFER_SIZE, QUERY_STR, cycles_1_rotor, cycles_2_rotor, cycles_3_rotor);
    // puts(query);
    // fflush(stdout);
    PGresult *res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "Query failed %s\n", PQerrorMessage(conn));
        PQclear(res);
        PQfinish(conn);
        return NULL;
    }

    const int nrows = PQntuples(res);
    for (int i = 0; i < nrows; ++i)
    {
        const char *first_cycles = PQgetvalue(res, i, 0);
        const char *second_cycles = PQgetvalue(res, i, 1);
        const char *third_cycles = PQgetvalue(res, i, 2);
        const char *rotor_positions = PQgetvalue(res, i, 3);
        const char *rotor_order = PQgetvalue(res, i, 4);
    }


    return NULL;
}

void init_db(void)
{
    conn = PQconnectdb("dbname=cyclometer host=localhost");//TODO add credentials
    assertmsg(PQstatus(conn) == CONNECTION_OK, "Couldn't connect");
}


void close_db(void)
{
    PQfinish(conn);
}