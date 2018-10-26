#include "message.h"

#define MSGBOX_CNT(msgbox_ptr)      (((msgbox_t *) (msgbox_ptr))->count)
#define MSGBOX_CAP(msgbox_ptr)      (((msgbox_t *) (msgbox_ptr))->capacity)
#define MSGBOX_QHEAD(msgbox_ptr)      (((msgbox_t *) (msgbox_ptr))->qhead)

#define MSG_HDR_MARK(msg_ptr)      (((msg_hdr_t *) (msg_ptr) - 1)->mark)
#define MSG_HDR_SPARE(msg_ptr)      (((msg_hdr_t *) (msg_ptr) - 1)->spare)
#define MSG_HDR_LEN(msg_ptr)      (((msg_hdr_t *) (msg_ptr) - 1)->len)


// 消息队列结构体
typedef struct
{
    uint16_t count; 
    uint16_t capacity;
    msg_q_t qhead;
} msgbox_t;

void *msgalloc( uint16_t len )
{
    msg_hdr_t *hdr;

    if ( len == 0 )
        return (void *)( NULL );

    hdr = ( msg_hdr_t *) mo_malloc( (size_t)(sizeof( msg_hdr_t ) + len ) );
    if ( hdr ) {
        //init it
        hdr->next = NULL;
        hdr->len = len;
        hdr->mark = FALSE; // not on qbox list
        hdr->spare = 0;
        
        return ( (void *) (hdr + 1) ); // point to the data
    }

    return (void *)( NULL );
}

int msgdealloc( void *msg_ptr )
{
    if ( msg_ptr == NULL )
        return ( MSG_INVALID_POINTER );

    // don't deallocate msg buffer when it on the list
    if ( MSG_HDR_MARK( msg_ptr ) == TRUE )
        return ( MSG_BUFFER_NOT_AVAIL );

    mo_free(  (void *)((uint8_t *)msg_ptr - sizeof( msg_hdr_t )) );

    return ( MSG_SUCCESS );
}

uint16_t msglen(void * msg_ptr)
{
    if(msg_ptr == NULL)
        return 0;

    return MSG_HDR_LEN(msg_ptr);
}

int msgsetspare(void * msg_ptr, uint8_t val)
{
    if ( msg_ptr == NULL )
        return ( MSG_INVALID_POINTER );

    MSG_HDR_SPARE(msg_ptr) = val;
    
    return MSG_SUCCESS;
}

uint8_t msgspare(void * msg_ptr)
{
    if ( msg_ptr == NULL )
        return 0;

    return MSG_HDR_SPARE(msg_ptr);
}

msgboxhandle_t msgBoxNew(uint16_t MaxCap)
{
    msgbox_t *pNewmsgbox;

    pNewmsgbox = ( msgbox_t * ) mo_malloc( sizeof( msgbox_t ) );

    if(pNewmsgbox){
        pNewmsgbox->capacity = MaxCap;
        pNewmsgbox->count = 0;
        pNewmsgbox->qhead = NULL;
    }

    return (msgboxhandle_t )pNewmsgbox;
}

msgboxhandle_t msgBoxAssign(msgboxstatic_t *pmsgboxBuffer,uint16_t MaxCap)
{
    msgbox_t *pNewmsgbox = ( msgbox_t * )pmsgboxBuffer;

    if( pNewmsgbox ){
        pNewmsgbox->capacity = MaxCap;
        pNewmsgbox->count = 0;
        pNewmsgbox->qhead = NULL;
    }

    return (msgboxhandle_t )pNewmsgbox;
}

uint16_t msgBoxcnt( msgboxhandle_t msgbox )
{
    if ( msgbox == NULL )
        return 0;

    return MSGBOX_CNT(msgbox);
}

uint16_t msgBoxIdle( msgboxhandle_t msgbox )
{
    if ( msgbox == NULL )
        return 0;

    
    return (MSGBOX_CAP(msgbox) - MSGBOX_CNT(msgbox));
}

void *msgBoxaccept( msgboxhandle_t msgbox )
{
    // no message on the list
    if(MSGBOX_CNT(msgbox) == 0)
        return NULL;

    MSGBOX_CNT(msgbox)--;
    
    return msgQpop(&MSGBOX_QHEAD(msgbox));
}

void *msgBoxpeek( msgboxhandle_t msgbox )
{
    // no message on the list
    if(MSGBOX_CNT(msgbox) == 0)
        return NULL;
    
    return msgQpeek(&MSGBOX_QHEAD(msgbox));
}

int msgBoxGenericpost(msgboxhandle_t msgbox, void *msg_ptr, uint8_t isfront )
{
    if ( msg_ptr == NULL || msgbox == NULL) {
        return ( MSG_INVALID_POINTER );
    }

    if(MSGBOX_CAP(msgbox) != MSGBOX_UNLIMITED_CAP && ((MSGBOX_CAP(msgbox) - MSGBOX_CNT(msgbox)) < 1) )
        return MSG_QBOX_FULL;
    
    // Check the message header ,not init it success, or message on the list
    if ( MSG_HDR_NEXT( msg_ptr ) != NULL || MSG_HDR_MARK( msg_ptr ) != FALSE ) {        
        return ( MSG_INVALID_POINTER );
    }

    MSGBOX_CNT(msgbox)++;
    msgQGenericput(&MSGBOX_QHEAD(msgbox), msg_ptr, isfront);

    return ( MSG_SUCCESS );
}

void msgQGenericput( msg_q_t *q_ptr, void *msg_ptr, uint8_t isfront )
{
    void *list;

    MSG_HDR_MARK( msg_ptr ) = TRUE; // mark on the list
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

//ok
void *msgQpop( msg_q_t *q_ptr )
{
    void *msg_ptr = NULL;

    if ( *q_ptr != NULL ) {
        // Dequeue message
        msg_ptr = *q_ptr;
        *q_ptr = MSG_HDR_NEXT( msg_ptr );
        MSG_HDR_NEXT( msg_ptr ) = NULL;
        MSG_HDR_MARK( msg_ptr ) = FALSE;
    }

  return msg_ptr;
}
//ok
void *msgQpeek( msg_q_t *q_ptr )
{
    return (void *)(*q_ptr);    
}

/*
void *findthe message you want(msg_q_t *q_ptr,...)
{
    void *list;
    void *prev = NULL;


    // find corresponding id message
    // Point to the top of the queue
    list = *q_ptr;

    // Look through the queue for a message that belongs to the asking task
    while ( list != NULL )
    {
        if ( ) {//message you find  
            break;
        }
        
        prev = list;
        list = MSG_HDR_NEXT( list );
    }

    // Did we find a message?
    if ( list != NULL ) {
        // Take out of the link list
        msgQextract( q_ptr, list, prev );
    }
    
    return ( ( uint8_t *) list );
}
*/
//ok
// Take out of the link list
void msgQextract( msg_q_t *q_ptr, void *msg_ptr, void *premsg_ptr )
{
    if ( msg_ptr == *q_ptr ) {
        // remove from first
        *q_ptr = MSG_HDR_NEXT( msg_ptr );
    }
    else {
        // remove from middle
        MSG_HDR_NEXT( premsg_ptr ) = MSG_HDR_NEXT( msg_ptr );
    }
    MSG_HDR_NEXT( msg_ptr ) = NULL;
    MSG_HDR_MARK( msg_ptr ) = FALSE;
}


