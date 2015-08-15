//
//============================================================================
// Name             : circular.c
// Author           : Alessio Moretti
// Version          : 0.1
// Date created     : 14/08/2015
// Last modified    : 14/08/2015
// Description      : This file contains an implementation of the circular
//                    buffer data structure with some example structures (to
//                    simulate an implementation over remote machines)
// ===========================================================================
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "circular.h"
#include "helper.h"

// initializing the singleton
Circular *singleton_circular = NULL;

// allocating circular buffer structures
Throwable *allocate_buffer(Server **servers, int len) {

    if (*servers == NULL || len == 0) {
        return (*get_throwable()).create(STATUS_ERROR, get_error_by_errno(errno), "allocate_buffer");
    }

    // allocating the buffer and setting params
    Circular *circular = get_circular();

    circular->buffer = *servers;
    circular->buffer_len = len;

    circular->head = circular->buffer;
    circular->tail = circular->buffer + len - 1;

    return (*get_throwable()).create(STATUS_OK, NULL, "allocate_buffer");
}

// managing the buffer progress updates
// WARNING: in this implementation there are two main features:
// - tail is useless 'cause the head will continue to check for all the servers (polling behaviour)
//   but it can be used to retrieve externally the ready server (when the function returns a PROGRESS_OK)
// - the circular buffer thread is the only one accessing the circular buffer memory
int progress() {
    Circular *circular = get_circular();

    // checking for remote machine status
    switch(circular->head->status) {
        case SERVER_STATUS_BUSY:
            circular->buffer_position = (circular->buffer_position + 1) % circular->buffer_len;
            circular->head            = circular->buffer + circular->buffer_position;
            return BUFFER_PROGRESS_STOP;
        case SERVER_STATUS_BROKEN:
            circular->buffer_position = (circular->buffer_position + 1) % circular->buffer_len;
            circular->head            = circular->buffer + circular->buffer_position;
            // TODO: using the log to monitor the server status or send a message to the main thread
            return BUFFER_PROGRESS_STOP;
        default: // SERVER_STATUS_READY
            circular->head->status    = SERVER_STATUS_BUSY;
            // recomputing tail, head and buffer position
            circular->tail            = circular->head;
            circular->buffer_position = (circular->buffer_position + 1) % circular->buffer_len;
            circular->head            = circular->buffer + circular->buffer_position;
            break;
    }
    return BUFFER_PROGRESS_OK;

}

// destroying the circular buffer structures
void destroy_buffer() {
    Circular *circular = get_circular();
    // freeing servers struct and ...
    int i;
    for (i = 0; i < circular->buffer_len; i++) {
        free(&circular->buffer[i]);
    }
    // ... finally
    free(circular->buffer);
    free(circular);
}




// setting up the circular buffer
Circular *new_circular(void) {
    Circular *circular = malloc(sizeof(Circular));
    if (circular == NULL) {
        fprintf(stderr, "Memory allocation error in new_circular().\n");
        exit(EXIT_FAILURE);
    }

    // set self "attribute"
    circular->self            = circular;
    circular->buffer_position = 0;

    // set "methods"
    circular->allocate_buffer = allocate_buffer;
    circular->progress        = progress;
    circular->destroy_buffer  = destroy_buffer;

    return circular;
}


// retrieving the singleton
Circular *get_circular(void) {
    if (singleton_circular == NULL) {
        singleton_circular = new_circular();
    }

    return singleton_circular;
}

// how to use it
int main(int argc, char *argv[]) {
    if (argc != 1) {
        fprintf(stdout, "Usage %s", argv[0]);
        return -1;
    }

    // initalizing a test array
    Server *servers = malloc(sizeof(Server)*3);
    char *names[3] = {"moretti0.org", "moretti1.org", "moretti2.org"};

    int i;
    for (i = 0; i < 3; i++) {
        servers[i].address = names[i];
        servers[i].status  = SERVER_STATUS_READY;
    }

    get_circular()->allocate_buffer(&servers, 3);

    int max_retry = 10;
    int retry = 0;
    while (retry <= max_retry) {
        if (get_circular()->progress() != BUFFER_PROGRESS_STOP) {
            fprintf(stdout, "BUFFERING: %s\n", get_circular()->tail->address);
        } else {
            retry++;
        }
    }
    fprintf(stdout, "STOPPED: max retries limit reached!\n");
    return 0;
}
