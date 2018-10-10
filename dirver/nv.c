

#include "nv.h"

/*********************************************************************
 * CONSTANTS
 */
 
#define NV_PAGE_SIZE       HAL_FLASH_PAGE_SIZE  // 每页的大小
#define NV_PAGE_USED       HAL_NV_PAGE_CNT      // 使用的页数
#define NV_PAGE_START      HAL_NV_PAGE_BEG      // 起始页 码
#define NV_PAGE_END       (NV_PAGE_START + NV_PAGE_USED - 1) // 结束页 码

#define NV_WORD_SIZE       HAL_FLASH_WORD_SIZE

#define NV_ACTIVE          0x00
#define NV_ERASED          0xFF
#define NV_ERASED_ID       0xFFFF  // 表明没被使用中
#define NV_ZEROED_ID       0x0000  // 表明投入使用
// Reserve MSB of Id to signal a search for the "old" source copy (new write interrupted/failed.)
#define NV_SOURCE_ID       0x8000

// In case pages 0-1 are ever used, define a null page value.
//定义一个空值,表明没使用
#define NV_PAGE_NULL       0

// In case item Id 0 is ever used, define a null item value.
// 定义一个空值,表明没使用
#define NV_ITEM_NULL       0

#define NV_PAGE_HDR_OFFSET 0 // 页头偏移位置 也就是0位置

/*********************************************************************
 * 确定NV总线的电压
 */
# define  NV_CHECK_BUS_VOLTAGE   1

#define NV_DATA_SIZE( LEN )                      \
  (((LEN) >= ((uint16_t)(65536UL - NV_WORD_SIZE))) ? \
             ((uint16_t)(65536UL - NV_WORD_SIZE))  : \
             ((((LEN) + NV_WORD_SIZE - 1) / NV_WORD_SIZE) * NV_WORD_SIZE))

#define NV_ITEM_SIZE( LEN )                                         \
  (((LEN) >= ((uint16_t)(65536UL - NV_WORD_SIZE - NV_ITEM_HDR_SIZE))) ? \
             ((uint16_t)(65536UL - NV_WORD_SIZE))                     : \
  (((((LEN) + NV_WORD_SIZE - 1) / NV_WORD_SIZE) * NV_WORD_SIZE) + NV_ITEM_HDR_SIZE))

#define COMPACT_PAGE_CLEANUP( COM_PG ) do { \
  /* In order to recover from a page compaction that is interrupted,\
   * the logic in NV_init() depends upon the following order:\
   * 1. State of the target of compaction is changed to ePgInUse.\
   * 2. Compacted page is erased.\
   */\
  setPageUse( pgRes, TRUE );  /* Mark the reserve page as being in use. */\
  erasePage( (COM_PG) ); \
  \
  pgRes = (COM_PG);           /* Set the reserve page to be the newly erased page. */\
}while(0)

/*********************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint16_t id;    //条目唯一识别码
  uint16_t len;   // Enforce Flash-WORD size on len.
  uint16_t chk;   // 条目数据 len长度的校验
  uint16_t stat;  // 条目状态
} NvItemHdr_t;
// Struct member offsets.
#define NV_ITEM_HDR_ID_OFFSET    0
#define NV_ITEM_HDR_LEN_OFFSET   2
#define NV_ITEM_HDR_CHK_OFFSET   4
#define NV_ITEM_HDR_STAT_OFFSET  6

#define NV_ITEM_HDR_SIZE  sizeof(NvItemHdr_t)  // 8 条目头大小 NvItemHdr_t结构体大小

typedef struct
{
  uint16_t active;    // 表明此页也被激活了,至于是否使用inuse决定
  uint16_t inUse;     // 使用中
  uint16_t xfer;      //有时候机器意外断电，而此时刚好有page在Xfer过程，那么page的xfer位就为非0xFFFF
  uint16_t spare;
} NvPageHdr_t;  //每一个Itemf都有一个8字节头部
// Struct member offsets.
#define NV_PAGE_HDR_ACTIVE_OFFSET 0
#define NV_PAGE_HDR_INUSE_OFFSET  2
#define NV_PAGE_HDR_XFER_OFFSET   4
#define NV_PAGE_HDR_SPARE_OFFSET  6

#define NV_PAGE_HDR_SIZE   sizeof(NvPageHdr_t) //8 页头大小 NvPageHdr_t

typedef enum
{
  eNvXfer,
  eNvZero
} eNvHdrEnum;

/*********************************************************************
 * LOCAL VARIABLES
 */

// Offset into the page of the first available erased space.
//每一个page      到空闲数据的的偏移量
static uint16_t pgOff[NV_PAGE_USED];

// Count of the bytes lost for the zeroed-out items.
//每一个page      被使用的字节数
static uint16_t pgLost[NV_PAGE_USED];

//item 压缩传输的 保留page,
static uint8_t pgRes;  // Page reserved for item compacting transfer.

// Saving ~100 code bytes to move a uint8_t* parameter/return value from findItem() to a global.
//用一个全局变量能节省100字节的空间，指示某一个item对应的page
static uint8_t findPg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8_t  initNV( void );

static void   setPageUse( uint8_t pg, uint8_t inUse );
static uint16_t initPage( uint8_t pg, uint16_t id, uint8_t findDups );
static void   erasePage( uint8_t pg );
static uint8_t  compactPage( uint8_t srcPg, uint16_t skipId );

static uint16_t findItem( uint16_t id );
static uint8_t  initItem( uint8_t flag, uint16_t id, void *buf, uint16_t len );
static void   setItem( uint8_t pg, uint16_t offset, eNvHdrEnum stat );
static uint16_t setChk( uint8_t pg, uint16_t offset, uint16_t chk );

static uint16_t calcChkBuff( uint8_t *buf, uint16_t len );
static uint16_t calcChkFlash( uint8_t pg, uint16_t offset, uint16_t len );

static void   writeWord( uint8_t pg, uint16_t offset, uint8_t *buf );
static void   writeWordH( uint8_t pg, uint16_t offset, uint8_t *buf );
static void   writeWordM( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t cnt );
static void   writeBuf( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t len );
static void   xferBuf( uint8_t srcPg, uint16_t srcOff, uint8_t dstPg, uint16_t dstOff, uint16_t len );

static uint8_t  writeItem( uint8_t pg, uint16_t id, uint16_t len, void *buf, uint8_t flag );
/*********************************************************************
 * @fn      initNV
 *
 * @brief   Initialize the NV flash pages.
 *
 * @param   none
 *
 * @return  TRUE
 */
static uint8_t initNV( void )
{
    NvPageHdr_t pgHdr;
    uint8_t oldPg = NV_PAGE_NULL;
    uint8_t findDups = FALSE;
    uint8_t pg;

    pgRes = NV_PAGE_NULL;

    //读取每一个page
    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ){
        // 读页头
        HalFlashRead(pg, NV_PAGE_HDR_OFFSET, (uint8_t *)(&pgHdr), NV_PAGE_HDR_SIZE);

        if ( pgHdr.active == NV_ERASED_ID ) { // 表明此页没被使用过
        
            if ( pgRes == NV_PAGE_NULL ) { // 无保留页
                pgRes = pg; // 设置一个保留页,作为压缩传输
            }
            else { // 有保留页,将此页激活并投入使用
                setPageUse( pg, TRUE );
            }
        }
        // An Xfer from this page was in progress.
        else if ( pgHdr.xfer != NV_ERASED_ID ){
            oldPg = pg;
        }
    }

    // If a page compaction was interrupted before the old page was erased.
    if ( oldPg != NV_PAGE_NULL ) {
        if ( pgRes != NV_PAGE_NULL ){
            /* Interrupted compaction before the target of compaction was put in use;
            * so erase the target of compaction and start again.
            */
            erasePage( pgRes );
            (void)compactPage( oldPg, NV_ITEM_NULL );
        }

        else{
       /* Interrupted compaction after the target of compaction was put in use,
        * but before the old page was erased; so erase it now and create a new reserve page.
        */
            erasePage( oldPg );
            pgRes = oldPg;
        }
    }
    else if ( pgRes != NV_PAGE_NULL ) {
        erasePage( pgRes );  // The last page erase could have been interrupted by a power-cycle.
    }
    /* else if there is no reserve page, COMPACT_PAGE_CLEANUP() must have succeeded to put the old
    * reserve page (i.e. the target of the compacted items) into use but got interrupted by a reset
    * while trying to erase the page to be compacted. Such a page should only contain duplicate items
    * (i.e. all items will be marked 'Xfer') and thus should have the lost count equal to the page
    * size less the page header.
    */

    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ) {
        // Calculate page offset and lost bytes - any "old" item triggers an N^2 re-scan from start.
        if ( initPage( pg, NV_ITEM_NULL, findDups ) != NV_ITEM_NULL ) {
            findDups = TRUE;
            pg = (NV_PAGE_START - 1);  // Pre-decrement so that loop increment will start over at zero.
            continue;
        }
    }

    if (findDups){
        // Final pass to calculate page lost after invalidating duplicate items.
        for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ) {
            (void)initPage( pg, NV_ITEM_NULL, FALSE );
        }
    }

    if ( pgRes == NV_PAGE_NULL ){
        uint8_t idx, mostLost = 0;

        for ( idx = 0; idx < NV_PAGE_USED; idx++ ) {
            // Is this the page that was compacted?
            if (pgLost[idx] == (NV_PAGE_SIZE - NV_PAGE_HDR_SIZE)) {
                mostLost = idx;
                break;
            }
            /* This check is not expected to be necessary because the above test should always succeed
            * with an early loop exit.
            */
            else if (pgLost[idx] > pgLost[mostLost]){
                mostLost = idx;
            }
        }

        pgRes = mostLost + NV_PAGE_START;
        erasePage( pgRes );  // The last page erase had been interrupted by a power-cycle.
    }

    return TRUE;
}

/*********************************************************************
 * @fn      setPageUse
 *
 * @brief   Set page header active/inUse state according to 'inUse'.
 *
 * @param   pg - Valid NV page to verify and init.
 * @param   inUse - Boolean TRUE if inUse, FALSE if only active.
 *
 * @return  none
 */
static void setPageUse( uint8_t pg, uint8_t inUse )
{
    NvPageHdr_t pgHdr;

    pgHdr.active = NV_ZEROED_ID;  // 激活此页

    if ( inUse ){
        pgHdr.inUse = NV_ZEROED_ID; // 投入使用
    }
    else{
        pgHdr.inUse = NV_ERASED_ID; // 仅激活它
    }

    //写页头
    writeWord( pg, NV_PAGE_HDR_OFFSET, (uint8_t*)(&pgHdr) );
}

/*********************************************************************
 * @fn      initPage
 *
 * @brief   Walk the page items; calculate checksums, lost bytes & page offset.
 *          扫描页面每一个条目,计算校验,丢失字节和页面偏移
 * @param   pg - Valid NV page to verify and init.
 * @param   id - Valid NV item Id to use function as a "findItem".
 *               If set to NULL then just perform the page initialization.
 * @param   findDups - TRUE on recursive call from initNV() to find and zero-out duplicate items
 *                     left from a write that is interrupted by a reset/power-cycle.
 *                     FALSE otherwise.
 *
 * @return  If 'id' is non-NULL and good checksums are found, return the offset
 *          of the data corresponding to item Id; else NV_ITEM_NULL.
 */
 //扫描页面中每一个条目,计算校验,被使用的字节数和页面偏移
static uint16_t initPage( uint8_t pg, uint16_t id, uint8_t findDups )
{
    uint16_t offset = NV_PAGE_HDR_SIZE;
    uint16_t sz, lost = 0;
    NvItemHdr_t hdr;

    do
    {
        // 读条目头
        HalFlashRead(pg, offset, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

        if ( hdr.id == NV_ERASED_ID ){  // 条目空闲,未使用
            break;
        }

        // Get the actual size in bytes which is the ceiling(hdr.len)
        // 获得真实条目的数据长度  ,要求4字节对齐
        sz = NV_DATA_SIZE( hdr.len );

        // A bad 'len' write has blown away the rest of the page.
        // 条目超过存储大小,标识下lost 和 offset
        if (sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - offset)) {
            lost += (NV_PAGE_SIZE - offset);
            offset = NV_PAGE_SIZE; 
            break;
        }

        // 偏移到数据位置
        offset += NV_ITEM_HDR_SIZE;

        if ( hdr.id != NV_ZEROED_ID ){
            // ID有效
          /* This trick allows function to do double duty for findItem() without
           * compromising its essential functionality at powerup initialization.
           */
            if ( id != NV_ITEM_NULL ){
                /* This trick allows asking to find the old/transferred item in case
                 * of a NV_SUCCESSful new item write that gets interrupted before the
                 * old item can be zeroed out.
                 */
                if ( (id & 0x7fff) == hdr.id ) {
                    if ( (((id & NV_SOURCE_ID) == 0) && (hdr.stat == NV_ERASED_ID)) ||
                        (((id & NV_SOURCE_ID) != 0) && (hdr.stat != NV_ERASED_ID)) ) {
                        return offset;
                    }
                }
            }
            // When invoked from the NV_init(), verify checksums and find & zero any duplicates.
            else {
                //校验数据是否正错
                if ( hdr.chk == calcChkFlash( pg, offset, hdr.len ) ){
                    if ( findDups ) {
                        if ( hdr.stat == NV_ERASED_ID ) {
                          /* The trick of setting the MSB of the item Id causes the logic
                           * immediately above to return a valid page only if the header 'stat'
                           * indicates that it was the older item being transferred.
                           */
                            uint16_t off = findItem( (hdr.id | NV_SOURCE_ID) );

                            if ( off != NV_ITEM_NULL ) {
                                setItem( findPg, off, eNvZero );  // Mark old duplicate as invalid.
                            }
                        }
                    }
                    // Any "old" item immediately exits and triggers the N^2 exhaustive initialization.
                    else if ( hdr.stat != NV_ERASED_ID ) {
                        return NV_ERASED_ID;
                    }
                } 
                else {
                    // 校验错误,标志头状态,并置ID 为 0
                    setItem( pg, offset, eNvZero );  // Mark bad checksum as invalid.
                    lost += (NV_ITEM_HDR_SIZE + sz);
                }
            }
        }
        else {
            // ID 为0 
            lost += (NV_ITEM_HDR_SIZE + sz);
        }
        //偏移到下一个条目
        offset += sz;
    } while (offset < (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE));

    pgOff[pg - NV_PAGE_START] = offset;
    pgLost[pg - NV_PAGE_START] = lost;

    return NV_ITEM_NULL;
}

/*********************************************************************
 * @fn      erasePage
 *
 * @brief   Erases a page in Flash.
 *
 * @param   pg - Valid NV page to erase.
 *
 * @return  none
 */
static void erasePage( uint8_t pg )
{
    HalFlashErase(pg);

    pgOff[pg - NV_PAGE_START] = NV_PAGE_HDR_SIZE;
    pgLost[pg - NV_PAGE_START] = 0;
}

/*********************************************************************
 * @fn      compactPage
 *
 * @brief   Compacts the page specified.
 *
 * @param   srcPg - Valid NV page to erase.
 * @param   skipId - Item Id to not compact.
 *
 * @return  TRUE if valid items from 'srcPg' are NV_SUCCESSully compacted onto the 'pgRes';
 *          FALSE otherwise.
 *          Note that on a failure, this could loop, re-erasing the 'pgRes' and re-compacting with
 *          the risk of infinitely looping on HAL flash failure.
 *          Worst case scenario: HAL flash starts failing in general, perhaps low Vdd?
 *          All page compactions will fail which will cause all NV_write() calls to return
 *          NV_OPER_FAILED.
 *          Eventually, all pages in use may also be in the state of "pending compaction" where
 *          the page header member NV_PAGE_HDR_XFER_OFFSET is zeroed out.
 *          During this "HAL flash brown-out", the code will run and OTA should work (until low Vdd
 *          causes an actual chip brown-out, of course.) Although no new NV items will be created
 *          or written, the last value written with a return value of NV_SUCCESS can continue to be
 *          read NV_SUCCESSfully.
 *          If eventually HAL flash starts working again, all of the pages marked as
 *          "pending compaction" may or may not be eventually compacted. But, initNV() will
 *          deterministically clean-up one page pending compaction per power-cycle
 *          (if HAL flash is working.) Nevertheless, one erased reserve page will be maintained
 *          through such a scenario.
 */
static uint8_t compactPage( uint8_t srcPg, uint16_t skipId )
{
  uint16_t srcOff;
  uint8_t rtrn;

  // To minimize code size, only check for a clean page here where it's absolutely required.
  for (srcOff = 0; srcOff < NV_PAGE_SIZE; srcOff++)
  {
    HalFlashRead(pgRes, srcOff, &rtrn, 1);
    if (rtrn != NV_ERASED)
    {
      erasePage(pgRes);
      return FALSE;
    }
  }

  srcOff = NV_PAGE_HDR_SIZE;
  rtrn = TRUE;

  while ( srcOff < (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE ) )
  {
    NvItemHdr_t hdr;
    uint16_t sz, dstOff = pgOff[pgRes-NV_PAGE_START];

    HalFlashRead(srcPg, srcOff, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

    if ( hdr.id == NV_ERASED_ID )
    {
      break;
    }

    // Get the actual size in bytes which is the ceiling(hdr.len)
    sz = NV_DATA_SIZE( hdr.len );

    if ( sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - srcOff) )
    {
      break;
    }

    if ( sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - dstOff) )
    {
      rtrn = FALSE;
      break;
    }

    srcOff += NV_ITEM_HDR_SIZE;

    if ( (hdr.id != NV_ZEROED_ID) && (hdr.id != skipId) )
    {
      if ( hdr.chk == calcChkFlash( srcPg, srcOff, hdr.len ) )
      {
        /* Prevent excessive re-writes to item header caused by numerous, rapid, & NV_SUCCESSive
         * OSAL_Nv interruptions caused by resets.
         */
        if ( hdr.stat == NV_ERASED_ID )
        {
          setItem( srcPg, srcOff, eNvXfer );
        }

        if ( writeItem( pgRes, hdr.id, hdr.len, NULL, FALSE ) )
        {
          dstOff += NV_ITEM_HDR_SIZE;
          xferBuf( srcPg, srcOff, pgRes, dstOff, sz );
          // Calculate and write the new checksum.
          if (hdr.chk == calcChkFlash(pgRes, dstOff, hdr.len))
          {
            if ( hdr.chk != setChk( pgRes, dstOff, hdr.chk ) )
            {
              rtrn = FALSE;
              break;
            }
          }
          else
          {
            rtrn = FALSE;
            break;
          }
        }
        else
        {
          rtrn = FALSE;
          break;
        }
      }
    }

    srcOff += sz;
  }

  if (rtrn == FALSE)
  {
    erasePage(pgRes);
  }
  else if (skipId == NV_ITEM_NULL)
  {
    COMPACT_PAGE_CLEANUP(srcPg);
  }
  // else invoking function must cleanup.

  return rtrn;
}

/*********************************************************************
 * @fn      findItem
 *
 * @brief   Find an item Id in NV and return the page and offset to its data.
 *
 * @param   id - Valid NV item Id.
 *
 * @return  Offset of data corresponding to item Id, if found;
 *          otherwise NV_ITEM_NULL.
 *
 *          The page containing the item, if found;
 *          otherwise no valid assignment made - left equal to item Id.
 *
 */
 // 扫描所有页,扫找指定id条目,返回页和偏移
static uint16_t findItem( uint16_t id )
{
    uint16_t off;
    uint8_t pg;

    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ){
        if ( (off = initPage( pg, id, FALSE )) != NV_ITEM_NULL ) {
            findPg = pg;
            
            return off;
        }
    }

    // Now attempt to find the item as the "old" item of a failed/interrupted NV write.
    if ( (id & NV_SOURCE_ID) == 0 ){
        return findItem( id | NV_SOURCE_ID );
    }
    else {
        findPg = NV_PAGE_NULL;
        
        return NV_ITEM_NULL;
    }
}

/*********************************************************************
 * @fn      initItem
 *
 * @brief   An NV item is created and initialized with the data passed to the function, if any.
 *
 * @param   flag - TRUE if the 'buf' parameter contains data for the call to writeItem().
 *                 (i.e. if invoked from NV_item_init() ).
 *                 FALSE if writeItem() should just write the header and the 'buf' parameter
 *                 is ok to use as a return value of the page number to be cleaned with
 *                 COMPACT_PAGE_CLEANUP().
 *                 (i.e. if invoked from NV_write() ).
 * @param   id  - Valid NV item Id.
 * @param   len - Item data length.
 * @param  *buf - Pointer to item initalization data. Set to NULL if none.
 *
 * @return  The OSAL Nv page number if item write and read back checksums ok;
 *          NV_PAGE_NULL otherwise.
 */
static uint8_t initItem( uint8_t flag, uint16_t id, void *buf, uint16_t len )
{
    uint16_t sz = NV_ITEM_SIZE( len );
    uint8_t rtrn = NV_PAGE_NULL;
    uint8_t cnt = NV_PAGE_USED;
    uint8_t pg = pgRes+1;  // Set to 1 after the reserve page to even wear across all available pages.

    do {
        if (pg >= NV_PAGE_START + NV_PAGE_USED) {
            pg = NV_PAGE_START;
        }
        if ( pg != pgRes ){
            uint8_t idx = pg - NV_PAGE_START;
            if ( sz <= (NV_PAGE_SIZE - pgOff[idx] + pgLost[idx]) ){
                break;
            }
        }
        pg++;
    } while (--cnt);

    if (cnt) {
        // Item fits if an old page is compacted.
        if ( sz > (NV_PAGE_SIZE - pgOff[pg - NV_PAGE_START]) ) {
            NvPageHdr_t pgHdr;

            /* Prevent excessive re-writes to page header caused by numerous, rapid, & NV_SUCCESSive
            * OSAL_Nv interruptions caused by resets.
            */
            HalFlashRead(pg, NV_PAGE_HDR_OFFSET, (uint8_t *)(&pgHdr), NV_PAGE_HDR_SIZE);
            if ( pgHdr.xfer == NV_ERASED_ID ) {
                // Mark the old page as being in process of compaction.
                sz = NV_ZEROED_ID;
                writeWordH( pg, NV_PAGE_HDR_XFER_OFFSET, (uint8_t*)(&sz) );
            }

            /* First the old page is compacted, then the new item will be the last one written to what
            * had been the reserved page.
            */
            if (compactPage( pg, id )) {
                if ( writeItem( pgRes, id, len, buf, flag ) ){
                    rtrn = pgRes;
                }

                if ( flag == FALSE ) {
                    /* Overload 'buf' as an OUT parameter to pass back to the calling function
                    * the old page to be cleaned up.
                    */
                    *(uint8_t *)buf = pg;
                }
                else {
                    /* Safe to do the compacted page cleanup even if writeItem() above failed because the
                    * item does not yet exist since this call with flag==TRUE is from NV_item_init().
                    */
                    COMPACT_PAGE_CLEANUP( pg );
                }
            }
        }
        else {
            if ( writeItem( pg, id, len, buf, flag ) ) {
                rtrn = pg;
            }
        }
    }

    return rtrn;
}

/*********************************************************************
 * @fn      setItem
 *
 * @brief   Set an item Id or status to mark its state.
 *
 * @param   pg - Valid NV page.
 * @param   offset - Valid offset into the page of the item data - the header
 *                   offset is calculated from this.
 * @param   stat - Valid enum value for the item status.
 *
 * @return  none
 */
static void setItem( uint8_t pg, uint16_t offset, eNvHdrEnum stat )
{
    NvItemHdr_t hdr;

    offset -= NV_ITEM_HDR_SIZE;
    HalFlashRead(pg, offset, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

    if ( stat == eNvXfer ) {
        hdr.stat = NV_ACTIVE;
        /* Write status to flash. Total of 4 bytes written to Flash 
        * ( 2 bytes of checksum and 2 bytes of status) 
        */
        writeWord( pg, offset + NV_ITEM_HDR_CHK_OFFSET, (uint8_t*)(&(hdr.chk)) );
    }
    else {// if ( stat == eNvZero )
    
        uint16_t sz = ((hdr.len + (NV_WORD_SIZE-1)) / NV_WORD_SIZE) * NV_WORD_SIZE + NV_ITEM_HDR_SIZE;
        hdr.id = 0;
        writeWord( pg, offset, (uint8_t *)(&hdr) );
        pgLost[pg - NV_PAGE_START] += sz;
    }
}

/*********************************************************************
 * @fn      setChk
 *
 * @brief   Set the item header checksum given the data buffer offset.
 *
 * @param   pg - Valid NV page.
 * @param   offset - Valid offset into the page of the item data - the header
 *                   offset is calculated from this.
 * @param   chk - The checksum to set.
 *
 * @return  The checksum read back.
 */
static uint16_t setChk( uint8_t pg, uint16_t offset, uint16_t chk )
{
    //偏移到校验和
    offset -= NV_WORD_SIZE;
    writeWordH( pg, offset, (uint8_t *)&chk );
    HalFlashRead( pg, offset, (uint8_t *)(&chk), sizeof( chk ) );

    return chk;
}

/*********************************************************************
 * @brief   Calculates the data checksum over the 'buf' parameter.
 *          
 * @param   buf - Data buffer to be checksummed.
 * @param   len - Byte count of the data to be checksummed.
 *
 * @return  Calculated checksum of the data bytes.
 */
 //计算buff的校验和          4字节对齐
static uint16_t calcChkBuff( uint8_t *buf, uint16_t len )
{
    uint8_t fill = len % NV_WORD_SIZE;
    uint16_t chk;

    if ( !buf ) {
        chk = len * NV_ERASED;
    }
    else{
        chk = 0;
        while ( len-- )
        {
            chk += *buf++;
        }
    }

    // calcChkFlash() will calculate over NV_WORD_SIZE alignment.
    //补充校验,按4字节对齐,补充值为0xff
    if ( fill ) {
        chk += (NV_WORD_SIZE - fill) * NV_ERASED;
    }

    return chk;
}

/*********************************************************************
 * @brief   Calculates the data checksum by reading the data bytes from NV.
 *          
 * @param   pg -   页面   A valid NV Flash page.
 * @param   offset - 数据域偏移  A valid offset into the page.
 * @param   len - 数据长度 Byte count of the data to be checksummed.
 *
 * @return  Calculated checksum of the data bytes.
 */
 // 计算页面flash内  长度为len数据的校验和 
static uint16_t calcChkFlash( uint8_t pg, uint16_t offset, uint16_t len )
{
    uint16_t chk = 0;
    uint8_t cnt, tmp[NV_WORD_SIZE];
    
    // 四字节对齐
    len = (len + (NV_WORD_SIZE - 1)) / NV_WORD_SIZE;

    while ( len-- )
    {
        HalFlashRead(pg, offset, tmp, NV_WORD_SIZE);
        offset += NV_WORD_SIZE;

        for ( cnt = 0; cnt < NV_WORD_SIZE; cnt++ ) {
            chk += tmp[cnt];
        }
    }

    return chk;
}

/*********************************************************************
 * @fn      writeWord
 *
 * @brief   Writes a Flash-WORD to NV.
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 *
 * @return  none
 */
 // 写一个字(4字节)
static void writeWord( uint8_t pg, uint16_t offset, uint8_t *buf )
{
    HalFlashWrite(pg, (offset / HAL_FLASH_WORD_SIZE) * HAL_FLASH_WORD_SIZE, buf, 1); 
}
/*********************************************************************
 * @fn      writeWordH
 *
 * @brief   Writes the 1st half of a Flash-WORD to NV (filling 2nd half with 0xffff).
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 *
 * @return  none
 */
 
// 写半个字(2字节)        实际还是写一个字
static void writeWordH( uint8_t pg, uint16_t offset, uint8_t *buf )
{
    uint8_t tmp[4];

    tmp[0] = buf[0];
    tmp[1] = buf[1];
    tmp[2] = NV_ERASED;
    tmp[3] = NV_ERASED;

    writeWord( pg, offset, tmp );
}

/*********************************************************************
 * @fn      writeWordM
 *
 * @brief   Writes multiple Flash-WORDs to NV.
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 * @param   cnt - 数据大小 4字节块个数 Number of 4-byte blocks to write.
 *
 * @return  none
 */
 
// 写多个字(字(4字节)对齐) cnt为字的个数.
static void writeWordM( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt )
{
    HalFlashWrite( pg, (offset / HAL_FLASH_WORD_SIZE) * HAL_FLASH_WORD_SIZE, buf, wordcnt);
}
/*********************************************************************
 * @fn      writeBuf
 *
 * @brief   Writes a data buffer to NV.
 *
 * @param   dstPg - A valid NV Flash page.
 * @param   offset - A valid offset into the page.
 * @param   len  - Byte count of the data to write.
 * @param   buf  - The data to write.
 *
 * @return  none
 */
static void writeBuf( uint8_t dstPg, uint16_t dstOff, uint8_t *buf, uint16_t len)
{
    uint8_t rem = dstOff % NV_WORD_SIZE;
    uint8_t tmp[NV_WORD_SIZE];

    // 先补齐偏移 4字节对齐问题
    if ( rem ) {
        dstOff = (dstOff / NV_WORD_SIZE) * NV_WORD_SIZE;
        HalFlashRead(dstPg, dstOff, tmp, NV_WORD_SIZE);

        while ( (rem < NV_WORD_SIZE) && len )
        {
            tmp[rem++] = *buf++;
            len--;
        }

        writeWord( dstPg, dstOff, tmp );
        dstOff += NV_WORD_SIZE;
    }

    rem = len % NV_WORD_SIZE;
    len /= NV_WORD_SIZE;

    // 写多个字
    if ( len ) {
        writeWordM( dstPg, dstOff, buf, len );
        dstOff += NV_WORD_SIZE * len;
        buf += NV_WORD_SIZE * len;
    }
    
    // 最后补齐 剩余len长度 4字节对齐问题
    if ( rem ) {
        uint8_t idx = 0;
        HalFlashRead(dstPg, dstOff, tmp, NV_WORD_SIZE);
        while ( rem-- )
        {
            tmp[idx++] = *buf++;
        }
        writeWord( dstPg, dstOff, tmp );
    }
}

/*********************************************************************
 * @fn      xferBuf
 *
 * @brief   Xfers an NV buffer from one location to another, enforcing NV_WORD_SIZE writes.
 *
 * @return  none
 */
static void xferBuf( uint8_t srcPg, uint16_t srcOff, uint8_t dstPg, uint16_t dstOff, uint16_t len )
{
    uint8_t rem = dstOff % NV_WORD_SIZE;
    uint8_t tmp[NV_WORD_SIZE];

    if ( rem ){
        dstOff -= rem;
        HalFlashRead(dstPg, dstOff, tmp, NV_WORD_SIZE);

        while ( (rem < NV_WORD_SIZE) && len )
        {
            HalFlashRead(srcPg, srcOff, tmp+rem, 1);
            srcOff++;
            rem++;
            len--;
        }

        writeWord( dstPg, dstOff, tmp );
        dstOff += NV_WORD_SIZE;
    }

    rem = len % NV_WORD_SIZE;
    len /= NV_WORD_SIZE;

    while ( len-- )
    {
        HalFlashRead(srcPg, srcOff, tmp, NV_WORD_SIZE);
        srcOff += NV_WORD_SIZE;
        writeWord( dstPg, dstOff, tmp );
        dstOff += NV_WORD_SIZE;
    }
    
    if ( rem ) {
        uint8_t idx = 0;
        HalFlashRead(dstPg, dstOff, tmp, NV_WORD_SIZE);
        while ( rem-- )
        {
            HalFlashRead(srcPg, srcOff, tmp+idx, 1);
            srcOff++;
            idx++;
        }
        writeWord( dstPg, dstOff, tmp );
    }
    }

/*********************************************************************
 * @fn      writeItem
 *
 * @brief   Writes an item header/data combo to the specified NV page.
 *
 * @param   pg - Valid NV Flash page.
 * @param   id - Valid NV item Id.
 * @param   len  - Byte count of the data to write.
 * @param   buf  - The data to write. If NULL, no data/checksum write.
 * @param   flag - TRUE if the checksum should be written, FALSE otherwise.
 *
 * @return  TRUE if header/data to write matches header/data read back, else FALSE.
 */
static uint8_t writeItem( uint8_t pg, uint16_t id, uint16_t len, void *buf, uint8_t flag )
{
    uint16_t offset = pgOff[pg-NV_PAGE_START];
    uint8_t rtrn = FALSE;
    NvItemHdr_t hdr;

    hdr.id = id;
    hdr.len = len;

    writeWord( pg, offset, (uint8_t *)&hdr );
    HalFlashRead(pg, offset, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

    if ( (hdr.id == id) && (hdr.len == len) ){
        if ( flag ){
            hdr.chk = calcChkBuff( buf, len );

            offset += NV_ITEM_HDR_SIZE;
            if ( buf != NULL ){
                writeBuf( pg, offset, buf, len );
            }
            
        if ( ( hdr.chk == calcChkFlash( pg, offset, len )) 
                &&  ( hdr.chk == setChk( pg, offset, hdr.chk ) )) {
            rtrn = TRUE;
        }


        }
        else {
            rtrn = TRUE;
        }

        len = NV_ITEM_SIZE( hdr.len );
    }
    else {
        len = NV_ITEM_SIZE( hdr.len );

        if (len > (NV_PAGE_SIZE - pgOff[pg - NV_PAGE_START])) {
            len = (NV_PAGE_SIZE - pgOff[pg - NV_PAGE_START]);
        }

        pgLost[pg - NV_PAGE_START] += len;
    }
    pgOff[pg - NV_PAGE_START] += len;

    return rtrn;
}

/*********************************************************************
 * @fn      NV_init
 *
 * @brief   Initialize NV service.
 *
 * @return  none
 */
void nvinit( void )
{
  (void)initNV();  // Always returns TRUE after pages have been erased.
}

/*********************************************************************
 * @fn      NV_item_init
 *
 * @brief   If the NV item does not already exist, it is created and
 *          initialized with the data passed to the function, if any.
 *          This function must be called before calling NV_read() or
 *          NV_write().
 *
 * @param   id  - Valid NV item Id.
 * @param  *buf - Pointer to item initalization data. Set to NULL if none.
 * @param   len - Item length.
 *
 * @return  NV_ITEM_UNINIT - Id did not exist and was created NV_SUCCESSfully.
 *          NV_SUCCESS        - Id already existed, no action taken.
 *          NV_OPER_FAILED - Failure to find or create Id.
 */
uint8_t nvItemInit( uint16_t id, void *buf, uint16_t len )
{
    if ( !NV_CHECK_BUS_VOLTAGE ){
        return NV_OPER_FAILED;
    }

    if ( findItem(id) != NV_ITEM_NULL){
        // Re-populate the NV hot item data if the corresponding items are already established.
        
        return NV_SUCCESS;
    }

    // 找不到,则初始化一个新条目
    if ( initItem( TRUE, id, buf, len ) != NV_PAGE_NULL ){
        return NV_ITEM_UNINIT;
    }
    else {
        return NV_OPER_FAILED;
    }
}

/*********************************************************************
 * @fn      nvItemlen
 *
 * @brief   Get the data length of the item stored in NV memory.
 *
 * @param   id  - Valid NV item Id.
 *
 * @return  Item length, if found; zero otherwise.
 */
uint16_t nvItemlen( uint16_t id )
{
    NvItemHdr_t hdr;
    uint16_t offset;

    if ((offset = findItem(id)) == NV_ITEM_NULL){
        return 0;
    }

    HalFlashRead(findPg, (offset - NV_ITEM_HDR_SIZE), (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);
    return hdr.len;
}

/*********************************************************************
 * @fn      NV_write
 *
 * @brief   Write a data item to NV. Function can write an entire item to NV or
 *          an element of an item by indexing into the item with an offset.
 *
 * @param   id  - Valid NV item Id.
 * @param   ndx - Index offset into item
 * @param   len - Length of data to write.
 * @param  *buf - Data to write.
 *
 * @return  NV_SUCCESS if NV_SUCCESSful, NV_ITEM_UNINIT if item did not
 *          exist in NV and offset is non-zero, NV_OPER_FAILED if failure.
 */
uint8_t nvWrite( uint16_t id, uint16_t ndx, void *buf, uint16_t len )
{
    uint8_t rtrn = NV_SUCCESS;
    NvItemHdr_t hdr;
    uint16_t origOff, srcOff;
    uint16_t cnt, chk;
    uint8_t *ptr, srcPg;

    if ( !NV_CHECK_BUS_VOLTAGE ) {
        return NV_OPER_FAILED;
    }
    
    if(len == 0)
        return NV_SUCCESS;


    origOff = srcOff = findItem( id );
    srcPg = findPg;
    if ( srcOff == NV_ITEM_NULL ){
        return NV_ITEM_UNINIT; // 沒初始化
    }

    HalFlashRead(srcPg, (srcOff - NV_ITEM_HDR_SIZE), (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);
    if ( hdr.len < (ndx + len) ) {
        return NV_OPER_FAILED; // 操作失敗
    }

    srcOff += ndx;
    ptr = buf;
    cnt = len;
    chk = 0;
    while ( cnt-- )
    {
        uint8_t tmp;
        HalFlashRead(srcPg, srcOff, &tmp, 1);
        if ( tmp != *ptr )
        {
            chk = 1;  // Mark that at least one byte is different.
            // Calculate expected checksum after transferring old data and writing new data.
            hdr.chk -= tmp;
            hdr.chk += *ptr;
        }
        srcOff++;
        ptr++;
    }

    if ( chk != 0 )  // If the buffer to write is different in one or more bytes.
    {
      uint8_t comPg = NV_PAGE_NULL;
      uint8_t dstPg = initItem( FALSE, id, &comPg, hdr.len );

      if ( dstPg != NV_PAGE_NULL )
      {
        uint16_t tmp = NV_DATA_SIZE( hdr.len );
        uint16_t dstOff = pgOff[dstPg-NV_PAGE_START] - tmp;
        srcOff = origOff;

        /* Prevent excessive re-writes to item header caused by numerous, rapid, & NV_SUCCESSive
         * OSAL_Nv interruptions caused by resets.
         */
        if ( hdr.stat == NV_ERASED_ID )
        {
          setItem( srcPg, srcOff, eNvXfer );
        }

        xferBuf( srcPg, srcOff, dstPg, dstOff, ndx );
        srcOff += ndx;
        dstOff += ndx;

        writeBuf( dstPg, dstOff, buf, len );
        srcOff += len;
        dstOff += len;

        xferBuf( srcPg, srcOff, dstPg, dstOff, (hdr.len - ndx - len) );

        // Calculate and write the new checksum.
        dstOff = pgOff[dstPg-NV_PAGE_START] - tmp;

        if ( hdr.chk == calcChkFlash( dstPg, dstOff, hdr.len ) )
        {
          if ( hdr.chk != setChk( dstPg, dstOff, hdr.chk ) )
          {
            rtrn = NV_OPER_FAILED;
          }
        }
        else
        {
          rtrn = NV_OPER_FAILED;
        }
      }
      else
      {
        rtrn = NV_OPER_FAILED;
      }

      if ( comPg != NV_PAGE_NULL )
      {
        /* Even though the page compaction succeeded, if the new item is coming from the compacted
         * page and writing the new value failed, then the compaction must be aborted.
         */
        if ( (srcPg == comPg) && (rtrn == NV_OPER_FAILED) )
        {
          erasePage( pgRes );
        }
        else
        {
          COMPACT_PAGE_CLEANUP( comPg );
        }
      }

      /* Zero of the old item must wait until after compact page cleanup has finished - if the item
       * is zeroed before and cleanup is interrupted by a power-cycle, the new item can be lost.
       */
      if ( (srcPg != comPg) && (rtrn != NV_OPER_FAILED) )
      {
        setItem( srcPg, origOff, eNvZero );
      }
    }

  return rtrn;
}

/*********************************************************************
 * @fn      NV_read
 *
 * @brief   Read data from NV. This function can be used to read an entire item from NV or
 *          an element of an item by indexing into the item with an offset.
 *          Read data is copied into *buf.
 *
 * @param   id  - Valid NV item Id.
 * @param   ndx - Index offset into item
 * @param   len - Length of data to read.
 * @param  *buf - Data is read into this buffer.
 *
 * @return  NV_SUCCESS if NV data was copied to the parameter 'buf'.
 *          Otherwise, NV_OPER_FAILED for failure.
 */
uint8_t nvRead( uint16_t id, uint16_t ndx, void *buf, uint16_t len )
{
    uint16_t offset;

    if ((offset = findItem(id)) == NV_ITEM_NULL) {
        return NV_OPER_FAILED;
    }
    else {
        HalFlashRead(findPg, offset + ndx, buf, len);

        return NV_SUCCESS;
    }
}

/*********************************************************************
 * @fn      NV_delete
 *
 * @brief   Delete item from NV. This function will fail if the length
 *          parameter does not match the length of the item in NV.
 *
 * @param   id  - Valid NV item Id.
 * @param   len - Length of item to delete.
 *
 * @return  NV_SUCCESS if item was deleted,
 *          NV_ITEM_UNINIT if item did not exist in NV,
 *          NV_BAD_ITEM_LEN if length parameter not correct,
 *          NV_OPER_FAILED if attempted deletion failed.
 */
uint8_t nvDelete( uint16_t id, uint16_t len )
{
    uint16_t length;
    uint16_t offset;

    offset = findItem( id );
    if ( offset == NV_ITEM_NULL ){
        // NV item does not exist
        return NV_ITEM_UNINIT;
    }

    length = nvItemlen( id );
    if ( length != len ) {
        // NV item has different length
        return NV_BAD_ITEM_LEN;
    }

    // Set item header ID to zero to 'delete' the item
    setItem( findPg, offset, eNvZero );

    // Verify that item has been removed
    offset = findItem( id );
    if ( offset != NV_ITEM_NULL ){
        // Still there
        return NV_OPER_FAILED;
    }
    else
    {
        // Yes, it's gone
        return NV_SUCCESS;
    }
}

/*********************************************************************
 */
