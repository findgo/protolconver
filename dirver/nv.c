

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


// 数据占用的字节数 字对齐
#define NV_DATA_SIZE( LEN )                      \
  (((LEN) >= ((uint16_t)(65536UL - NV_WORD_SIZE))) ? \
             ((uint16_t)(65536UL - NV_WORD_SIZE))  : \
             ((((LEN) + NV_WORD_SIZE - 1) / NV_WORD_SIZE) * NV_WORD_SIZE))

// 一个条目占用的字节数 字对齐
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
  setPageActive( pgRes );  /* Mark the reserve page as being active. */\
  erasePage( (COM_PG) ); \
  \
  pgRes = (COM_PG);           /* Set the reserve page to be the newly erased page. */\
}while(0)

/*********************************************************************
 * TYPEDEFS
 */
// 条目头
typedef struct
{
  uint16_t id;    //条目唯一识别码
  uint16_t len;   // Enforce Flash-WORD size on len.
  uint16_t chk;   // 条目数据校验
  uint16_t stat;  // 条目状态
} NvItemHdr_t;
// Struct member offsets.
#define NV_ITEM_HDR_ID_OFFSET    0
#define NV_ITEM_HDR_LEN_OFFSET   2
#define NV_ITEM_HDR_CHK_OFFSET   4
#define NV_ITEM_HDR_STAT_OFFSET  6

#define NV_ITEM_HDR_SIZE  sizeof(NvItemHdr_t)  // 8 条目头大小 NvItemHdr_t结构体大小

// 页头
typedef struct
{
  uint16_t active;     // 表明此页激活使用
  uint16_t xfer;       // 有时候机器意外断电，而此时刚好有page在Xfer过程，那么page的xfer位就为非0xFFFF
} NvPageHdr_t; 
// Struct member offsets.
#define NV_PAGE_HDR_ACTIVE_OFFSET 0
#define NV_PAGE_HDR_XFER_OFFSET   2

#define NV_PAGE_HDR_SIZE   sizeof(NvPageHdr_t) //4 页头大小 NvPageHdr_t

/*********************************************************************
 * LOCAL VARIABLES
 */

//每一个page      到空闲数据的的偏移量
static uint16_t pgOff[NV_PAGE_USED];
//每一个page      无效数据的字节数
static uint16_t pgLost[NV_PAGE_USED];

//当页满时,压缩无效item,并存有效item到保留page, 然后擦除旧页, 全局有且仅有一个保留页
static uint8_t pgRes;  // Page reserved for item compacting transfer.

// Saving ~100 code bytes to move a uint8_t* parameter/return value from findItem() to a global.
//用一个全局变量能节省100字节的空间，指示某一个item对应的page
static uint8_t findPg;

/*********************************************************************
 * LOCAL FUNCTIONS
 */
static uint8_t  initNV( void );

static void   setPageActive( uint8_t pg );
static uint16_t initPage( uint8_t pg, uint16_t id );
static void   erasePage( uint8_t pg );
static uint8_t  compactPage( uint8_t srcPg, uint16_t skipId );

static uint16_t findItem( uint16_t id );
static uint8_t  initItem( uint8_t flag, uint16_t id, void *buf, uint16_t len );
static void   setItemInvalid( uint8_t pg, uint16_t ItemDataoffset );
static uint16_t setChk( uint8_t pg, uint16_t ItemDataoffset, uint16_t chk );

static uint16_t calcChkBuff( uint8_t *buf, uint16_t len );
static uint16_t calcChkFlash( uint8_t pg, uint16_t ItemDataoffset, uint16_t len );

static void   writeWord( uint8_t pg, uint16_t offset, uint8_t *buf );
static void   writeHalfWord( uint8_t pg, uint16_t offset, uint8_t *buf );
static void   writeMulWord( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt );
static void   writeBuf( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t len );
static void   xferBuf( uint8_t srcPg, uint16_t srcOff, uint8_t dstPg, uint16_t dstOff, uint16_t len );

static uint8_t writeItem( uint8_t pg, uint16_t id, void *buf, uint16_t len, uint8_t flag );
/*********************************************************************
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
    uint8_t pg;

    pgRes = NV_PAGE_NULL;

    //读取每一个page
    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ){
        // 读页头
        HalFlashRead(pg, NV_PAGE_HDR_OFFSET, (uint8_t *)(&pgHdr), NV_PAGE_HDR_SIZE);

        if ( pgHdr.active == NV_ERASED_ID ) { // 表明此页没被使用过
        
            if ( pgRes == NV_PAGE_NULL ) { // 无保留页
                pgRes = pg; // 作为保留页,用于压缩页面条目
            }
            else { // 激活此页
                setPageActive( pg );
            }
        }
        // An Xfer from this page was in progress. 表明上次压缩数据被中断了,需要重新处理数据
        else if ( pgHdr.xfer != NV_ERASED_ID ){
            oldPg = pg;
        }
    }

    // If a page compaction was interrupted before the old page was erased.
    if ( oldPg != NV_PAGE_NULL ) {
        // 出现压缩数据异常
        if ( pgRes != NV_PAGE_NULL ){
            /* Interrupted compaction before the target of compaction was put in use;
            * so erase the target of compaction and start again.
            */
         // 有保留页,数据压缩处理被中断或掉电了, 重新处理旧页数据
            erasePage( pgRes );
            (void)compactPage( oldPg, NV_ITEM_NULL );
        }
        else{
       /* Interrupted compaction after the target of compaction was put in use,
        * but before the old page was erased; so erase it now and create a new reserve page.
        */
        // 无保留页, 旧页数据已经压缩处理完毕了,但是oldpg来不及擦除,就被中断或掉电了. 导致保留页无. 直接做为保留页
            erasePage( oldPg );
            pgRes = oldPg;
        }
    }
     // 数据均正常
    else if ( pgRes != NV_PAGE_NULL ) {
        erasePage( pgRes );  // The last page erase could have been interrupted by a power-cycle.
    }
    /* else 为何没有保留页了?        COMPACT_PAGE_CLEANUP 擦除过程中 被中断了? 导致保留页丢失            如何处理? 查找一个页, 有重复条目
    * 并且条目都被设置为xfer. 因此 */
    /* else if there is no reserve page, COMPACT_PAGE_CLEANUP() must have succeeded to put the old
    * reserve page (i.e. the target of the compacted items) into use but got interrupted by a reset
    * while trying to erase the page to be compacted. Such a page should only contain duplicate items
    * (i.e. all items will be marked 'Xfer') and thus should have the lost count equal to the page
    * size less the page header.
    */

    // 扫描每一个页的每一个条目,校验每一个条目,计算每一页无效数据字节数和空闲数据偏移量
    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ) {
        // Calculate page offset and lost bytes
        ( void )initPage( pg, NV_ITEM_NULL );
    }

    // 无保留页处理
    if ( pgRes == NV_PAGE_NULL ){
        uint8_t idx, mostLost = 0;

        for ( idx = 0; idx < NV_PAGE_USED; idx++ ) {
            // 无效数据过多啦! 整页都无效的
            if (pgLost[idx] == (NV_PAGE_SIZE - NV_PAGE_HDR_SIZE)) {
                mostLost = idx;
                break;
            }
             /* 这个检查不需要,上面那个应该总是成功的 */
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
 * @brief   Set page header active
 *
 * @param   pg - Valid NV page to verify and init.
 *
 * @return  none
 */
static void setPageActive( uint8_t pg )
{
    NvPageHdr_t pgHdr;

    pgHdr.active = NV_ZEROED_ID;  // 激活此页

    //写active
    writeHalfWord( pg, NV_PAGE_HDR_OFFSET + NV_PAGE_HDR_ACTIVE_OFFSET, (uint8_t*)(&pgHdr.active) );
}

/*********************************************************************
 * @brief   Walk the page items; calculate checksums, lost bytes & page offset.
 *          扫描页面每一个条目,计算校验,丢失字节和页面偏移
 * @param   pg - Valid NV page to verify and init.
 * @param   id - Valid NV item Id to use function as a "findItem".
 *               If set to NULL then just perform the page initialization.
 *
 * @return  If 'id' is non-NULL and good checksums are found, return the offset
 *          of the data corresponding to item Id; else NV_ITEM_NULL.
 */
 //扫描页面中每一个条目,计算校验, 算出无效字节数和空闲数据的偏移
static uint16_t initPage( uint8_t pg, uint16_t id )
{
    uint16_t offset = NV_PAGE_HDR_SIZE;
    uint16_t sz, lost = 0;
    NvItemHdr_t hdr;

    do
    {
        HalFlashRead(pg, offset, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

        if ( hdr.id == NV_ERASED_ID ){  // 条目空闲,未使用
            break;
        }

        // 获得真实条目的数据长度  ,要求4字节对齐
        sz = NV_DATA_SIZE( hdr.len );

        // 条目超过存储大小,标识下lost 和 offset
        if (sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - offset)) {
            lost += (NV_PAGE_SIZE - offset);
            offset = NV_PAGE_SIZE; 
            break;
        }

        // 偏移到数据位置
        offset += NV_ITEM_HDR_SIZE;

        if ( hdr.stat == NV_ERASED_ID && hdr.id != NV_ITEM_NULL){
            // 数据有效
            if ( id != NV_ITEM_NULL ){
                if ( id == hdr.id ) {
                    return offset;
                }
            }
            // When invoked from the NV_init(), verify checksums and find & zero any duplicates.
            else {
                //校验数据是否正错
                if ( hdr.chk != calcChkFlash( pg, offset, hdr.len ) ){
                    // 校验错误,标志头状态条目无效
                    setItemInvalid( pg, offset );  // Mark bad checksum as invalid.
                    lost += (NV_ITEM_HDR_SIZE + sz);
                }
            }
        }
        else {
            // 数据无效
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
 * @return  TRUE if valid items from 'srcPg' are successully compacted onto the 'pgRes';
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
    NvItemHdr_t hdr;
    uint16_t sz, dstOff;

    // To minimize code size, only check for a clean page here where it's absolutely required.
    // 确保保留页是擦除的,否则压缩失败
    for (srcOff = 0; srcOff < NV_PAGE_SIZE; srcOff++) {
        HalFlashRead(pgRes, srcOff, &rtrn, 1);
        if (rtrn != NV_ERASED){
            erasePage(pgRes);
            
            return FALSE;
        }
    }

    srcOff = NV_PAGE_HDR_SIZE;
    rtrn = TRUE;

    //扫描源页每一个条目
    while ( srcOff < (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE ) )
    {
        dstOff = pgOff[pgRes - NV_PAGE_START];

        HalFlashRead(srcPg, srcOff, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

        if ( hdr.id == NV_ERASED_ID ){
            break;
        }

        // Get the actual size in bytes which is the ceiling(hdr.len)
        sz = NV_DATA_SIZE( hdr.len );

        if ( sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - srcOff) ) {
            break;
        }

        if ( sz > (NV_PAGE_SIZE - NV_ITEM_HDR_SIZE - dstOff) ) {
            rtrn = FALSE;
            break;
        }

        srcOff += NV_ITEM_HDR_SIZE;

        if ( (hdr.stat == NV_ERASED_ID) && (hdr.id != skipId) ) {
            if ( hdr.chk == calcChkFlash( srcPg, srcOff, hdr.len ) ) {
                /* Prevent excessive re-writes to item header caused by numerous, rapid, & successive
                * OSAL_Nv interruptions caused by resets.
                */

                if ( writeItem( pgRes, hdr.id, NULL, hdr.len,  FALSE ) ) {
                    dstOff += NV_ITEM_HDR_SIZE;
                    xferBuf( srcPg, srcOff, pgRes, dstOff, sz );
                    // Calculate and write the new checksum.
                    if (hdr.chk == calcChkFlash(pgRes, dstOff, hdr.len)) {
                        if ( hdr.chk != setChk( pgRes, dstOff, hdr.chk ) ) {
                            rtrn = FALSE;
                            break;
                        }
                    }
                    else{
                        rtrn = FALSE;
                        break;
                    }
                }
                else {
                    rtrn = FALSE;
                    break;
                }
            }
        }

        srcOff += sz;
    }

    if (rtrn == FALSE) {
        erasePage(pgRes);
    }
    else if (skipId == NV_ITEM_NULL) {
        COMPACT_PAGE_CLEANUP(srcPg);
    }
    // else invoking function must cleanup.

    return rtrn;
}

/*********************************************************************
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
 // 扫描所有页,找到指定id条目,返回页码和数据偏移地址 ItemDataoffset
static uint16_t findItem( uint16_t id )
{
    uint16_t off;
    uint8_t pg;

    for ( pg = NV_PAGE_START; pg <= NV_PAGE_END; pg++ ){
        off = initPage( pg, id );
        if ( off != NV_ITEM_NULL ) {
            findPg = pg;
            
            return off;
        }
    }
    
    findPg = NV_PAGE_NULL;
        
    return NV_ITEM_NULL;
}

/*********************************************************************
 * @brief   An NV item is created and initialized with the data passed to the function, if any.
 *
 * @param   flag - TRUE if the 'buf' parameter contains data for the call to writeItem().
 *                 (i.e. if invoked from nvItemInit() ).
 *
 *                 FALSE if writeItem() should just write the header and the 'buf' parameter
 *                 is ok to use as a return value of the page number to be cleaned with
 *                 COMPACT_PAGE_CLEANUP().
 *                 (i.e. if invoked from NV_write() ).
 * @param  *buf - Pointer to item initalization data. Set to NULL if none.
 * @param   id  - Valid NV item Id.
 * @param  *buf - Pointer to item initalization data. Set to NULL if none.
 * @param   len - Item data length.
 *
 * @return  The Nv page number if item write and read back checksums ok;
 *          NV_PAGE_NULL otherwise.
 */
 // 创建并初始化一个条目, 成功返回一个目标页
static uint8_t initItem( uint8_t flag, uint16_t id, void *buf, uint16_t len )
{
    uint16_t sz = NV_ITEM_SIZE( len );
    uint8_t rtrn = NV_PAGE_NULL;
    uint8_t cnt = NV_PAGE_USED;
    uint8_t pg = pgRes + 1;  // Set to 1 after the reserve page to even wear across all available pages.

    do {
        if (pg > NV_PAGE_END) {
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
                writeHalfWord( pg, NV_PAGE_HDR_XFER_OFFSET, (uint8_t*)(&sz) );
            }

            /* First the old page is compacted, then the new item will be the last one written to what
            * had been the reserved page.
            */
            if (compactPage( pg, id )) {
                if ( writeItem( pgRes, id, buf, len, flag ) ){
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
            if ( writeItem( pg, id, buf, len, flag ) ) {
                rtrn = pg;
            }
        }
    }

    return rtrn;
}

/*********************************************************************
<<<<<<< HEAD
 * @brief   Set an item status to mark its state.
=======
 * @brief   Set an item Id or status to mark its state.
>>>>>>> 060a33ffa128b77308a0a8d4d4def3e4810b7279
 *
 * @param   pg - Valid NV page.
 * @param   ItemDataoffset - Valid offset into the page of the item data - the header
 *                   offset is calculated from this.
 *
 * @return  none
 */
 // 将条目设为无效
static void setItemInvalid( uint8_t pg, uint16_t ItemDataoffset )
{
    NvItemHdr_t hdr;

    HalFlashRead(pg, ItemDataoffset - NV_ITEM_HDR_SIZE, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

    hdr.stat = NV_ZEROED_ID;
    /* Write status to flash.*/
    writeHalfWord( pg, ItemDataoffset - NV_ITEM_HDR_SIZE + NV_ITEM_HDR_STAT_OFFSET, (uint8_t*)(&(hdr.stat)) );

    pgLost[pg - NV_PAGE_START] += ((hdr.len + (NV_WORD_SIZE - 1)) / NV_WORD_SIZE) * NV_WORD_SIZE + NV_ITEM_HDR_SIZE;
}

/*********************************************************************
 * @brief   Set the item header checksum given the data buffer offset.
 *
 * @param   pg - Valid NV page.
 * @param   ItemDataoffset - Valid offset into the page of the item data - the header
 *                   offset is calculated from this.
 * @param   chk - The checksum to set.
 *
 * @return  The checksum read back.
 */
static uint16_t setChk( uint8_t pg, uint16_t ItemDataoffset, uint16_t chk )
{
    //计算到校验和的偏移
    ItemDataoffset -= NV_WORD_SIZE;
    writeHalfWord( pg, ItemDataoffset, (uint8_t *)&chk );
    HalFlashRead( pg, ItemDataoffset, (uint8_t *)(&chk), sizeof( chk ) );

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
 * @param   ItemDataoffset - 数据域偏移 字对齐  A valid offset into the page.
 * @param   len - 数据长度 Byte count of the data to be checksummed.
 *
 * @return  Calculated checksum of the data bytes.
 */
 // 计算页面flash内  长度为len数据的校验和 
static uint16_t calcChkFlash( uint8_t pg, uint16_t ItemDataoffset, uint16_t len )
{
    uint16_t chk = 0;
    uint8_t i, tmp[NV_WORD_SIZE];
    
    // 四字节对齐
    len = (len + (NV_WORD_SIZE - 1)) / NV_WORD_SIZE;

    while ( len-- )
    {
        HalFlashRead(pg, ItemDataoffset, tmp, NV_WORD_SIZE);
        ItemDataoffset += NV_WORD_SIZE;

        for ( i = 0; i < NV_WORD_SIZE; i++ ) {
            chk += tmp[i];
        }
    }

    return chk;
}
/*********************************************************************
 * @brief   Writes the half word to a Flash
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移,要求半字(2字节)对齐 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 *
 * @return  none
 */
 static void writeHalfWord( uint8_t pg, uint16_t offset, uint8_t *buf )
{
    HalFlashWriteHalfWord( pg, offset, (uint16_t) (((buf[1] & 0x00FF) << 8) + (buf[0] & 0x00FF)) );
}

/*********************************************************************
 * @brief   Writes the word to a Flash
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移,要求字(4字节)对齐 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 *
 * @return  none
 */
static void writeWord( uint8_t pg, uint16_t offset, uint8_t *buf )
{
    HalFlashWriteMulWord(pg, offset, buf, 1); 
}

/*********************************************************************
 * @brief   Writes multiple Flash-WORDs to NV.
 *
 * @param   pg - 页地址 A valid NV Flash page.
 * @param   offset - 页内偏移,要求字(4字节)对齐 A valid offset into the page.
 * @param   buf - 数据源 Pointer to source buffer.
 * @param   wordcnt - 数据大小 4字节块个数 Number of 4-byte blocks to write.
 *
 * @return  none
 */
static void writeMulWord( uint8_t pg, uint16_t offset, uint8_t *buf, uint16_t wordcnt )
{
    HalFlashWriteMulWord( pg, offset, buf, wordcnt );
}
/*********************************************************************
 * @fn      writeBuf
 *
 * @brief   Writes a data buffer to NV.
 *
 * @param   dstPg - A valid NV Flash page.
 * @param   dstOff - 偏移,要求4字节对齐 A valid offset into the page.
 * @param   buf  - The data to write.
 * @param   len  - Byte count of the data to write.
 *
 * @return  none
 */
static void writeBuf( uint8_t dstPg, uint16_t dstOff, uint8_t *buf, uint16_t len)
{
    uint8_t rem, idx;
    uint8_t tmp[NV_WORD_SIZE];
    
    rem = len % NV_WORD_SIZE;
    len /= NV_WORD_SIZE;

    // 写多个字
    if ( len ) {
        writeMulWord( dstPg, dstOff, buf, len );
        dstOff += NV_WORD_SIZE * len;
        buf += NV_WORD_SIZE * len;
    }
    
    // 最后补齐 剩余len长度 4字节对齐问题,余下补0xff
    if ( rem ) {
        for(idx = 0; idx < NV_WORD_SIZE; idx++){
            tmp[idx] = idx < rem ? (*buf++) : 0xff;
        }
        
        writeWord( dstPg, dstOff, tmp );
    }
}

/*********************************************************************
 * @brief   Xfers an NV buffer from one location to another, enforcing NV_WORD_SIZE writes.
 * srcOff dstOff 要求字对齐
 *
 * @return  none
 */
static void xferBuf( uint8_t srcPg, uint16_t srcOff, uint8_t dstPg, uint16_t dstOff, uint16_t len )
{
    uint8_t rem, idx;
    uint8_t tmp[NV_WORD_SIZE];

    rem = len % NV_WORD_SIZE;
    len /= NV_WORD_SIZE;

    while ( len-- )
    {
        HalFlashRead(srcPg, srcOff, tmp, NV_WORD_SIZE);
        srcOff += NV_WORD_SIZE;
        writeWord( dstPg, dstOff, tmp );
        dstOff += NV_WORD_SIZE;
    }

    // 最后补齐 剩余len长度 4字节对齐问题,余下补0xff
    if ( rem ) {
        HalFlashRead(srcPg, srcOff, tmp, NV_WORD_SIZE);
        
        for(idx = rem; idx < NV_WORD_SIZE; idx++){
            tmp[idx] = 0xff;
        }
        
        writeWord( dstPg, dstOff, tmp );
    }
}

/*********************************************************************
 * @brief   Writes an item header/data combo to the specified NV page.
 *
 * @param   pg - Valid NV Flash page.
 * @param   id - Valid NV item Id.
 * @param   buf  - The data to write. If NULL, no data/checksum write.
 * @param   len  - Byte count of the data to write.
 * @param   flag - TRUE if the checksum should be written, FALSE otherwise.
 *
 * @return  TRUE if header/data to write matches header/data read back, else FALSE.
 */
static uint8_t writeItem( uint8_t pg, uint16_t id, void *buf, uint16_t len, uint8_t flag )
{
    uint16_t offset = pgOff[pg - NV_PAGE_START];
    uint8_t rtrn = FALSE;
    NvItemHdr_t hdr;

    hdr.id = id;
    hdr.len = len;

    writeWord( pg, offset, (uint8_t *)&hdr );
    HalFlashRead(pg, offset, (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);

    if ( (hdr.id == id) && (hdr.len == len) ){
        if ( flag ){
            // 写checksum
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
 * @brief   Write a data item to NV. Function can write an entire item to NV
 *
 * @param   id  - Valid NV item Id.
 * @param  *buf - Data to write.
 * @param   len - Length of data to write.
 *
 * @return  NV_SUCCESS if successful, NV_ITEM_UNINIT if item did not
 *          exist in NV and offset is non-zero, NV_OPER_FAILED if failure.
 */
uint8_t nvItemWrite( uint16_t id, void *buf, uint16_t len )
{
    uint8_t rtrn = NV_SUCCESS;
    NvItemHdr_t hdr;
    uint16_t origOff,srcOff;
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
        // 找不到,则初始化一个新条目
        if ( initItem( TRUE, id, buf, len ) != NV_PAGE_NULL ){
            return NV_ITEM_UNINIT;
        }

        return NV_OPER_FAILED;
    }

    HalFlashRead(srcPg, (srcOff - NV_ITEM_HDR_SIZE), (uint8_t *)(&hdr), NV_ITEM_HDR_SIZE);
    if ( hdr.len < len ) {
        return NV_OPER_FAILED; // 操作失敗
    }

    //确认是否有数据发生改变,重新计算新数据校验和
    ptr = buf;
    cnt = len;
    chk = 0;
    while ( cnt-- )
    {
        uint8_t tmp;
        HalFlashRead(srcPg, srcOff, &tmp, 1);
        if ( tmp != *ptr ) {
            chk = 1;  // Mark that at least one byte is different.
            // Calculate expected checksum after transferring old data and writing new data.
            hdr.chk -= tmp;
            hdr.chk += *ptr;
        }
        srcOff++;
        ptr++;
    }

    // 数据发生变化,重新写新的数据
    // If the buffer to write is different in one or more bytes.
    if ( chk != 0 )  {  
        uint8_t comPg = NV_PAGE_NULL;
        uint8_t dstPg = initItem( FALSE, id, &comPg, hdr.len ); // 初始化一个新的条目头,并返回目标页,获得一个需要clean up的页

        if ( dstPg != NV_PAGE_NULL ) {
            uint16_t tmp = NV_DATA_SIZE( hdr.len );
            uint16_t dstOff = pgOff[dstPg - NV_PAGE_START] - tmp;
            srcOff = origOff;

            writeBuf( dstPg, dstOff, buf, len );

            if ( hdr.chk == calcChkFlash( dstPg, dstOff, hdr.len ) ){
                if ( hdr.chk != setChk( dstPg, dstOff, hdr.chk ) ) {
                    rtrn = NV_OPER_FAILED;
                }
            }
            else{
                rtrn = NV_OPER_FAILED;
            }
      }
      else {
            rtrn = NV_OPER_FAILED;
      }

    if ( comPg != NV_PAGE_NULL ) {
        /* Even though the page compaction succeeded, if the new item is coming from the compacted
         * page and writing the new value failed, then the compaction must be aborted.
         */
        if ( (srcPg == comPg) && (rtrn == NV_OPER_FAILED) ){
            erasePage( pgRes );
        }
        else {
            COMPACT_PAGE_CLEANUP( comPg );
        }
    }

      /* Zero of the old item must wait until after compact page cleanup has finished - if the item
       * is zeroed before and cleanup is interrupted by a power-cycle, the new item can be lost.
       */
      if ( (srcPg != comPg) && (rtrn != NV_OPER_FAILED) ) {
            setItemInvalid( srcPg, origOff );
      }
    }

  return rtrn;
}

/*********************************************************************
 * @brief   Read data from NV. This function can be used to read an entire item from NV or
 *          an element of an item by indexing into the item with an offset.
 *          Read data is copied into *buf.
 *
 * @param   id  - Valid NV item Id.
 * @param   ndx - Index offset into item
 * @param  *buf - Data is read into this buffer.
 * @param   len - Length of data to read.
 *
 * @return  NV_SUCCESS if NV data was copied to the parameter 'buf'.
 *          Otherwise, NV_OPER_FAILED for failure.
 */
uint8_t nvItemReadx( uint16_t id, uint16_t ndx, void *buf, uint16_t len )
{
    uint16_t offset;

    offset = findItem(id);
    if (offset == NV_ITEM_NULL) {
        return NV_OPER_FAILED;
    }
    
    HalFlashRead(findPg, offset + ndx, buf, len);

    return NV_SUCCESS;
}

/*********************************************************************
 * @fn      NV_delete
 *
 * @brief   Delete item from NV. This function will fail if the length
 *          parameter does not match the length of the item in NV.
 *
 * @param   id  - Valid NV item Id.
 *
 * @return  NV_SUCCESS if item was deleted,
 *          NV_ITEM_UNINIT if item did not exist in NV,
 *          NV_OPER_FAILED if attempted deletion failed.
 */
uint8_t nvItemDelete( uint16_t id )
{
    uint16_t offset;

    offset = findItem( id );
    if ( offset == NV_ITEM_NULL ){
        // NV item does not exist
        return NV_ITEM_UNINIT;
    }
    
    // Set item header stat to zero to 'delete' the item
    setItemInvalid( findPg, offset );

    // Verify that item has been removed
    offset = findItem( id );
    if ( offset != NV_ITEM_NULL ){
        // Still there
        return NV_OPER_FAILED;
    }
    else {
        // Yes, it's gone
        return NV_SUCCESS;
    }
}

/*********************************************************************
 */
