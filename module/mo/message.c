#include "message.h"

#if configSUPPORT_MSG == 1

#define MSG_HDR_ID(msg_ptr)      ((msg_hdr_t *) (msg_ptr) - 1)->id
#define MSG_HDR_LEN(msg_ptr)      ((msg_hdr_t *) (msg_ptr) - 1)->len
#define MSG_HDR_NEXT(msg_ptr)      ((msg_hdr_t *) (msg_ptr) - 1)->next

typedef struct
{
    uint16_t  id; 
    uint16_t len;
    void   *next;
} msg_hdr_t;

// local function
static uint8_t msg_put( uint16_t id, uint8_t *msg_ptr, uint8_t bpos );
static void msg_queueput( void **q_ptr, void *msg_ptr, uint8_t bpos );
static void *msg_queuepop( void **q_ptr );
static void msg_queueextract( void **q_ptr, void *msg_ptr, void *prev_ptr );

// local variable
static void * msg_qhead = NULL;


uint8_t *msg_allocate( uint16_t len )
{
    msg_hdr_t *hdr;

    if ( len == 0 )
        return ( NULL );

    hdr = ( msg_hdr_t *) mo_malloc( (size_t)(sizeof( msg_hdr_t ) + len ) );
    if ( hdr ) {
        //init it
        hdr->next = NULL;
        hdr->len = len;
        hdr->id = MSG_ID_NO_USED;
        
        return ( (uint8_t *) (hdr + 1) ); // point to the data
    }

    return ( NULL );
}

uint8_t msg_deallocate( void *msg_ptr )
{
    if ( msg_ptr == NULL )
        return ( MSG_INVALID_POINTER );

    // don't deallocate queued buffer
    if ( MSG_HDR_ID( msg_ptr ) != MSG_ID_NO_USED )
        return ( MSG_BUFFER_NOT_AVAIL );

    mo_free(  (void *)((uint8_t *)msg_ptr - sizeof( msg_hdr_t )) );

    return ( MSG_SUCCESS );
}

uint8_t msg_send( uint16_t id, uint8_t *msg_ptr )
{
    return msg_put(id, msg_ptr, FALSE);
}

uint8_t msg_send_front( uint16_t id, uint8_t *msg_ptr )
{
    return msg_put(id, msg_ptr, TRUE);
}


uint8_t *msg_receive( uint16_t id )
{
    msg_hdr_t *listHdr;
    msg_hdr_t *prevHdr = NULL;

    // pop the message from head
    if(id == MSG_ID_NO_USED)
        return msg_queuepop(&msg_qhead);

    // find corresponding id message
    // Point to the top of the queue
    listHdr = msg_qhead;

    // Look through the queue for a message that belongs to the asking task
    while ( listHdr != NULL )
    {
        if ( MSG_HDR_ID(listHdr) == id ) {
            break;
        }
        
        prevHdr = listHdr;
        listHdr = MSG_HDR_NEXT( listHdr );
    }
    
    // Did we find a message?
    if ( listHdr != NULL ) {
        // Take out of the link list
        msg_queueextract( &msg_qhead, listHdr, prevHdr );
    }

    return ( ( uint8_t *) listHdr );
}


static uint8_t msg_put( uint16_t id, uint8_t *msg_ptr, uint8_t bpos )
{
    if ( msg_ptr == NULL ) {
        return ( MSG_INVALID_POINTER );
    }

    if ( id == MSG_ID_NO_USED ) {
        msg_deallocate( msg_ptr );
        
        return ( MSG_INVALID_ID );
    }

    // Check the message header ,not init it success, or message on the list
    if ( MSG_HDR_NEXT( msg_ptr ) != NULL || MSG_HDR_ID( msg_ptr ) != MSG_ID_NO_USED ) {
        msg_deallocate( msg_ptr );
        
        return ( MSG_INVALID_POINTER );
    }

    MSG_HDR_ID( msg_ptr ) = id;

    msg_queueput(&msg_qhead,  msg_ptr, bpos);

    // Signal the event
    return ( MSG_SUCCESS );
}

static void msg_queueput( void **q_ptr, void *msg_ptr, uint8_t isfront )
{
    void *list;
    
    if(isfront == TRUE){ // put to front
        // Push message to head of queue
        MSG_HDR_NEXT( msg_ptr ) = *q_ptr;
        *q_ptr = msg_ptr;
    }
    else { // put to back
        // set nex to null
        MSG_HDR_NEXT( msg_ptr ) = NULL;
        // If first message in queue
        if ( *q_ptr == NULL ){
            *q_ptr = msg_ptr;
        }
        else {
            // Find end of queue
            for ( list = *q_ptr; MSG_HDR_NEXT( list ) != NULL; list = MSG_HDR_NEXT( list ) );

            // Add message to end of queue
            MSG_HDR_NEXT( list ) = msg_ptr;
        }
    }
}

static void *msg_queuepop( void **q_ptr )
{
    void *msg_ptr = NULL;

    if ( *q_ptr != NULL ) {
        // Dequeue message
        msg_ptr = *q_ptr;
        *q_ptr = MSG_HDR_NEXT( msg_ptr );
        MSG_HDR_NEXT( msg_ptr ) = NULL;
        MSG_HDR_ID( msg_ptr ) = MSG_ID_NO_USED;
    }

  return msg_ptr;
}

static void msg_queueextract( void **q_ptr, void *msg_ptr, void *prev_ptr )
{
    if ( msg_ptr == *q_ptr ) {
        // remove from first
        *q_ptr = MSG_HDR_NEXT( msg_ptr );
    }
    else {
        // remove from middle
        MSG_HDR_NEXT( prev_ptr ) = MSG_HDR_NEXT( msg_ptr );
    }
    MSG_HDR_NEXT( msg_ptr ) = NULL;
    MSG_HDR_ID( msg_ptr ) = MSG_ID_NO_USED;
}

#endif
