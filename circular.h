//
// Created by Odysseus on 14/08/15.
//

#ifndef WEBSWITCH_CIRCULAR_H
#define WEBSWITCH_CIRCULAR_H

#include "throwable.h"

#define BUFFER_PROGRESS_OK          0
#define BUFFER_PROGRESS_STOP       -1

#define SERVER_STATUS_READY         0
#define SERVER_STATUS_BUSY          1
#define SERVER_STATUS_BROKEN       -1
/*
 * this is only an example structure to define a circular buffer over a set of
 * finite web servers
 */
typedef struct server {
    char *address;
    int  status;
} Server;

/*
 * ---------------------------------------------------------------------------
 * Structure        : typedef struct circular_buffer
 * Description      : This struct helps to manage a circular buffer of fixed length
 * ---------------------------------------------------------------------------
 */
typedef struct circular_buffer {
    struct circular_buffer *self;
    Server      *buffer;
    int         buffer_position;
    int         buffer_len;
    Server      *head;
    Server      *tail;

    Throwable *(*allocate_buffer)(Server **servers, int len);
    int       (*progress)();
    void      (*destroy_buffer)();
} Circular;

/*
 * ---------------------------------------------------------------------------
 * Function   : allocate_buffer
 * Description: This function is used to allocate a finite buffer of finite
 *              remote machines
 *
 * Param      :
 *   struct server array: to allocate the buffer
 *   integer len:         the length of the server array
 *
 * Return     :
 *   Throwable pointer
 * ---------------------------------------------------------------------------
 */
Throwable *allocate_buffer(Server **servers, int len);


/*
 * ---------------------------------------------------------------------------
 * Function   : progress
 * Description: This function is used to move along the circular buffer
 *
 * Param      :
 *
 * Return     :
 *   buffer progress status integer indicator
 * ---------------------------------------------------------------------------
 */
int progress(void);


/*
 * ---------------------------------------------------------------------------
 * Function   : destroy_buffer
 * Description: This function is used to deallocate the buffer
 *
 * Param      :
 * Return     :
 * ---------------------------------------------------------------------------
 */
void destroy_buffer();


/*
 * ---------------------------------------------------------------------------
 * Function   : new_circular
 * Description: This function is used to initialize the Circular singleton
 *
 * Param      :
 *
 * Return     :
 *   Circular pointer
 * ---------------------------------------------------------------------------
 */
Circular *new_circular(void);

/*
 * ---------------------------------------------------------------------------
 * Function   : get_circular
 * Description: This function is used to retrieve the Circular singleton
 *
 * Param      :
 *
 * Return     :
 *   Circular pointer
 * ---------------------------------------------------------------------------
 */
Circular *get_circular(void);

#endif
