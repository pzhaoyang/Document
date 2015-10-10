#include "goku_gps_socket.h"
#include "goku_gps_protocol_manager.h"

#ifndef WIN32
#define DEBUG_MODULE_ID UNIPRO_MODULE_MAIN
#include "unipro_debug.h"
#endif

#define GPS_BUFFER_SETTING_FILE_PATH_NAME       "/flash/unipro/gps/setting.ini"
#define GPS_BUFFER_FILE_PATH                    "/flash/unipro/gps"
#define GPS_BUFFER_SETTING_FILE_MAX_LENGTH      100
#define GPS_BUFFER_FILE_MAX_COUNT               100

#if 1
//#define GET_1TH_ARG(a1, ...) a1
//#define GET_OTHER_ARG(a1, ...) __VA_ARGS__

#define _GPS_PRINT(fmt, ...) \
    g_message("%s():%d  "fmt,__func__,__LINE__,__VA_ARGS__)

#define GPS_PRINT(...) \
    _GPS_PRINT( GET_1TH_ARG(__VA_ARGS__,""), GET_OTHER_ARG(__VA_ARGS__,"") )
#else
#define GPS_PRINT(...)
#endif
#define GPS_PRINT_ERROR     g_error("%s():%d  gps_buffer_error",__func__,__LINE__);
#define GPS_PRINT_ENTRY     g_message("%s():%d  entry",__func__,__LINE__);

static gboolean    g_b_buffer_init = FALSE;
static OS_MUTEX_ID g_gps_buffer_mutex = OS_MUTEX_INVALID_ID;
static GPS_FUFFER  g_gps_cache;
static UINT32      g_uRecvTotalbyte = 0;




static int goku_gps_buffer_file_write_files(UINT8* pBuffer,UINT32 nBufferSize,UINT32* pIndex);


static void goku_gps_recv_bytes_total(UINT32 recv_bytes)
{
    g_uRecvTotalbyte += recv_bytes;
    PRINT_FUNC_LINE("g_uRecvTotalbyte = %ld bytes",g_uRecvTotalbyte);
}


gboolean  goku_gps_recv(GPS_CMD_CMD_E cmd)
{
    gchar RecData[GPS_REC_DATA_BUF_LEN] = {0};
    gint RecDataLen = GPS_REC_DATA_BUF_LEN;
    cmdstr_info t_GPSinfo = {0};

    PRINT_FUNC_LINE("===================enter=========================");
    tp_os_mem_set(&t_GPSinfo,0x00,sizeof(t_GPSinfo));
    if(TRUE == unipro_socket_recv(ENUM_SOCKET_TYPE_GPS, RecData,&RecDataLen))
    {
        PRINT_FUNC_LINE("cmd = %d,RecData = %s,RecDataLen = %d",cmd,RecData,RecDataLen);
        
        if(0x2d == RecData[0]){ //0x2d 0x31 代表'-1' 
            PRINT_FUNC_LINE("Socket Recv Fail");
            return FALSE;
        }
        //接收流量统计
        goku_gps_recv_bytes_total((UINT32)RecDataLen);
        
        if(goku_gps_cmdstr_decode(RecData,&t_GPSinfo))
        {
            PRINT_FUNC_LINE(" goku_gps_msg_handle_notice :t_GPSinfo.cmdstr = %s\nt_GPSinfo.parm = %s",t_GPSinfo.cmdstr,t_GPSinfo.parm);
            if(cmd == goku_gps_get_cmd_by_cmdstr(t_GPSinfo.cmdstr))
            {
                PRINT_FUNC_LINE("V5 DEC");
                return goku_gps_cmdparm_decode(goku_gps_get_cmd_by_cmdstr(t_GPSinfo.cmdstr),(char*)&t_GPSinfo);
            }
            else
            {
                PRINT_FUNC_LINE("NO V5 DEC");
                return goku_gps_cmdparm_decode(goku_gps_get_cmd_by_cmdstr(t_GPSinfo.cmdstr),t_GPSinfo.parm);
            }
        }
        else if(cmd == SET_LOCAL_UPLOAD || cmd == SET_LOCAL_UPLOAD_SINGLE_POINT)
        {
            PRINT_FUNC_LINE("RecData[0] = 0x%x",RecData[0]);
            return TRUE;
        }
        else if(cmd == SET_LOCLA_BEAT_PACK)
        {
            if(goku_gps_dcode_config(RecData))
            {
                PRINT_FUNC_LINE("V6 Decode Success!");
            }
            else
            {
                PRINT_FUNC_LINE("V6 Decode Error!");
            }
            return TRUE;
        }
        else
        {
            PRINT_FUNC_LINE("command Error!");
            return FALSE;
        }
    }
    else
    {
        tp_os_mem_set(&RecData,0x00,GPS_REC_DATA_BUF_LEN);
        PRINT_FUNC_LINE("recv parm error!");
        return FALSE;
    }
}

static void goku_gps_buffer_print_cache()
{
    GPS_PRINT("nTurnFlag = %d CurrentBufferLen = %d BufCnt = %d",
        g_gps_cache.nTurnFlag,g_gps_cache.uCacheLen,goku_gps_buffer_get_count());
}

void goku_gps_buffer_get_print_cache(char* pBuffer)
{
    sprintf(pBuffer,"\r\nBuffer:\r\nTunFlg:%d Len = %d Cnt = %d\r\n",
        g_gps_cache.nTurnFlag,g_gps_cache.uCacheLen,goku_gps_buffer_get_count());
}


static gboolean goku_gps_buffer_file_check_disk()
{//判断文件系统剩余容量是否足够，待实现
    return  TRUE;
}

static UINT32 goku_gps_buffer_get_mem_used_size()
{
    UINT32   nMem;
    
    nMem = g_gps_cache.nTurnFlag*g_gps_cache.uCacheLen + (g_gps_cache.pWrite-g_gps_cache.pRead);
    GPS_PRINT("used size=%d",nMem);
    
    return nMem;
}

static UINT32 goku_gps_buffer_get_file_used_size()
{
    return g_gps_cache.nFileBufferSize;
}

static UINT32 goku_gps_buffer_get_used_size_ex()
{
    UINT32   nMem;
    
    nMem = goku_gps_buffer_get_mem_used_size()+goku_gps_buffer_get_file_used_size();
    
    return nMem;
}

static UINT32 goku_gps_buffer_get_mem_unused_size()
{
    UINT32   nMem;
    
    nMem = g_gps_cache.uCacheLen-goku_gps_buffer_get_used_size_ex();
//    GPS_PRINT("unused size=%d",nMem);
    
    return nMem;
}

static void goku_gps_buffer_file_save_setting()
{
    FILE*   pFile   = NULL;
    int nLength;
    int nWriteLength;
    char    chBuffer[50];
  
    GPS_PRINT_ENTRY
    
    pFile   = tp_fopen(GPS_BUFFER_SETTING_FILE_PATH_NAME, "w");
    if( pFile == NULL )
    {
        GPS_PRINT_ERROR
        return;
    }

    sprintf( chBuffer, "%u,%u,%u", g_gps_cache.nFileIndexRead, 
        g_gps_cache.nFileIndexWrite, g_gps_cache.nFileBufferSize );
    nLength = strlen(chBuffer);
    nWriteLength    = tp_fwrite( chBuffer, 1, nLength, pFile );
    tp_fclose(pFile);
    if( nWriteLength != nLength )
    {
        GPS_PRINT_ERROR
        return;
    }
}

static int goku_gps_buffer_file_read_buffer(char* pPathName,UINT8* pBuffer,UINT32 nBufferSize)
{
    FILE*   pFile   = NULL;
    int nFileLength;
    int nReadLength;

    GPS_PRINT("pPathName=%s,nBufferSize=%d",pPathName,nBufferSize);
        
    pFile   = tp_fopen( pPathName, "r" );
    if( pFile == NULL )
    {
        GPS_PRINT_ERROR
        return  0;
    }

    // read buffer  {
    nFileLength = tp_flength(pFile);
    if( nFileLength <= 0 || nFileLength > nBufferSize )
    {
        tp_fclose(pFile);
        GPS_PRINT_ERROR
        return  0;
    }
    
    nReadLength = tp_fread( pBuffer, 1, nFileLength, pFile );
    tp_fclose(pFile);
    if( nFileLength != nReadLength )
    {
        GPS_PRINT_ERROR
        return  0;
    }  
    // read buffer  }    

    GPS_PRINT("nReadLength=%d",nReadLength);
    return  nReadLength;
}

static gboolean goku_gps_buffer_file_load_done(int nIndex)
{
    FILE*   pFile   = NULL;
    char    chFilePathName[MAX_PATH];
    int nFileLength;
    int nReadLength;
    gboolean    bRes;

    GPS_PRINT("nIndex=%d",nIndex);
        
    sprintf( chFilePathName, "%s/%05d",GPS_BUFFER_FILE_PATH, nIndex );
    nReadLength    = goku_gps_buffer_file_read_buffer(chFilePathName,
        g_gps_cache.pCache,GPS_BUFFER_LEN);
    if( nReadLength <= 0 )
    {
        return  FALSE;
    }
    //删除文件，该文件内的数据已导入到内存
    tp_fdelete(chFilePathName);
    
    g_gps_cache.uCacheLen   = nReadLength;
    g_gps_cache.pRead       = g_gps_cache.pCache;
    if( nReadLength == GPS_BUFFER_LEN )
    {//内存写满
        g_gps_cache.pWrite  = g_gps_cache.pCache;
        g_gps_cache.nTurnFlag = 1;
    }
    else
    {
        g_gps_cache.pWrite  = g_gps_cache.pCache + nReadLength;
        g_gps_cache.nTurnFlag = 0;
    }

    //读取成功后，减去buffer大小
    g_gps_cache.nFileBufferSize -= nReadLength;

    return  TRUE;
}

//把数据从一个文件中读取到内存
static gboolean goku_gps_buffer_file_load_one_file()
{
    gboolean    bRes;
    
    GPS_PRINT_ENTRY

    if( g_gps_cache.nFileIndexRead == 0 )
    {//文件中没有数据，直接返回
        GPS_PRINT("nFileIndexRead == 0");
        return  TRUE;
    }
    
    bRes    = goku_gps_buffer_file_load_done(g_gps_cache.nFileIndexRead);
    if( !bRes  )
    {//文件加载失败
        g_gps_cache.nFileIndexRead      = 0;
        g_gps_cache.nFileIndexWrite     = 0;
        g_gps_cache.nFileBufferSize     = 0;
    }
    else
    {
        g_gps_cache.nFileIndexRead++;
        if( g_gps_cache.nFileIndexRead > g_gps_cache.nFileIndexWrite )
        {//文件内的数据已经没有了，清除标记
            g_gps_cache.nFileIndexRead  = 0;
            g_gps_cache.nFileIndexWrite = 0;
            g_gps_cache.nFileBufferSize = 0;
        }
    }
    goku_gps_buffer_file_save_setting();

    return  bRes;
}

static gboolean goku_gps_buffer_file_load()
{
    FILE*   pFile   = NULL;
    int nFileLength;
    int nReadLength;
    char    chBuffer[GPS_BUFFER_SETTING_FILE_MAX_LENGTH+1];
    char* pTempStart;
    char* pTempEnd;
    int nValue;
    int nStartIndex;
    int nEndIndex;
    int nBufferSize;
  
    GPS_PRINT_ENTRY
    
    pFile   = tp_fopen(GPS_BUFFER_SETTING_FILE_PATH_NAME, "r");
    if( pFile == NULL )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }

    nFileLength = tp_flength(pFile);
    if( nFileLength <= 0 || nFileLength > GPS_BUFFER_SETTING_FILE_MAX_LENGTH )
    {
        tp_fclose(pFile);
        GPS_PRINT_ERROR
        return  FALSE;
    }

    memset( chBuffer, 0x00, sizeof(chBuffer) );
    nReadLength = tp_fread( chBuffer, 1, nFileLength, pFile);
    if( nFileLength != nReadLength )
    {
        tp_fclose(pFile);
        GPS_PRINT_ERROR
        return  FALSE;
    }
    chBuffer[nReadLength]   = 0x00;    
    tp_fclose(pFile);

    //  read file start index   {
    pTempStart  = chBuffer;
    pTempEnd    = strchr( pTempStart, ',' );
    if( pTempEnd == NULL )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    *pTempEnd   = 0x00;
    nValue  = atoi(pTempStart);
    if( nValue <= 0 || nValue > GPS_BUFFER_FILE_MAX_COUNT )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    nStartIndex  = nValue;
    //  read file start index   }
    
    //  read file end index     {
    pTempStart  = pTempEnd + 1;
    pTempEnd    = strchr( pTempStart, ',' );
    if( pTempEnd == NULL )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    *pTempEnd   = 0x00;
    nValue  = atoi(pTempStart);
    if( nValue <= 0 || nValue > GPS_BUFFER_FILE_MAX_COUNT )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    nEndIndex   = nValue;
    //  read file end index     }

    if( nEndIndex < nStartIndex )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }

    //  read file buffer size     {
    pTempStart  = pTempEnd + 1;
    nValue  = atoi(pTempStart);
    if( nValue <= 0 )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    nBufferSize   = nValue;
    //  read file buffer size     }
    
    g_gps_cache.nFileIndexRead      = nStartIndex;
    g_gps_cache.nFileIndexWrite     = nEndIndex;
    g_gps_cache.nFileBufferSize     = nBufferSize;
    GPS_PRINT("nStartIndex=%d,nEndIndex=%d,nBufferSize=%d",nStartIndex,nEndIndex,nBufferSize);

    return  goku_gps_buffer_file_load_one_file(); 
}

static gboolean goku_gps_buffer_file_save_files(UINT32* pIndex)
{
    UINT8*  pTemp1  = NULL;
    UINT8*  pTemp2  = NULL;
    UINT32 nLength1    = 0;
    UINT32 nLength2    = 0;
    
    GPS_PRINT_ENTRY
        
    if( g_gps_cache.nTurnFlag )
    {
        pTemp1      = g_gps_cache.pRead;
        nLength1    = g_gps_cache.pCache + g_gps_cache.uCacheLen - g_gps_cache.pRead;
        pTemp2      = g_gps_cache.pCache;
        nLength2    = g_gps_cache.pWrite - g_gps_cache.pCache;
    }
    else
    {
        pTemp1      = g_gps_cache.pRead;
        nLength1    = g_gps_cache.pWrite - g_gps_cache.pRead;
    }

    if( pTemp1 && nLength1 > 0 )
    {
        goku_gps_buffer_file_write_files(pTemp1,nLength1,pIndex);
    }
    if( pTemp2 && nLength2 > 0 )
    {
        goku_gps_buffer_file_write_files(pTemp2,nLength2,pIndex);
    }
    
    return  TRUE;

}

static gboolean goku_gps_buffer_file_save_ex()
{
    UINT32 nIndex;
    UINT32* pIndex;
    
    GPS_PRINT_ENTRY
        
    if( g_gps_cache.nFileBufferSize == 0 )
    {//第一次写文件，文件下标从1开始
        g_gps_cache.nFileIndexRead      = 1;
        g_gps_cache.nFileIndexWrite     = 1;
        //从文件的写下标开始写文件，并改变文件的写下标
        pIndex  = &g_gps_cache.nFileIndexWrite;
    }
    else
    {    
        int nUsedSize;
        int nFileCount;
        
        nUsedSize   = goku_gps_buffer_get_mem_used_size();
        nFileCount  = (nUsedSize+GPS_BUFFER_LEN-1)/GPS_BUFFER_LEN;
        //往前写文件，因为内存里的数据比已写的文件中的数据产生的早
        g_gps_cache.nFileIndexRead  -= nFileCount;
        nIndex  = g_gps_cache.nFileIndexRead;
        //从文件的读下标开始写文件，但不改变文件的写下标和读下标
        pIndex  = &nIndex;//
    }
    
    return  goku_gps_buffer_file_save_files(pIndex);
}

static gboolean goku_gps_buffer_file_save()
{
    gboolean    bRes;
    int nUsedSize;

    GPS_PRINT_ENTRY
        
    nUsedSize   = goku_gps_buffer_get_mem_used_size();    
    if( nUsedSize == 0 )
    {//内存中没有数据需要写入到文件
        return  TRUE;
    }    
   
    if( !goku_gps_buffer_file_check_disk() )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }
    
    bRes    = goku_gps_buffer_file_save_ex();
    goku_gps_buffer_file_save_setting();

    return  bRes;
}

static int goku_gps_buffer_file_write_buffer(char* pFilePathName, UINT8* pBuffer,int nBufferSize, int nMaxFileSize)
{
    FILE*   pFile   = NULL;
    char    chFilePathName[MAX_PATH];
    int nFileLength;
    int nLength;
    int nWriteLength;

    GPS_PRINT("pFilePathName=%s,nBufferSize=%d",pFilePathName,nBufferSize);
        
    pFile   = tp_fopen( pFilePathName, "a" );
    if( pFile == NULL )
    {//返回-1，则写文件失败
        GPS_PRINT_ERROR
        return  -1;
    }

    //计算文件已有数据的长度
    tp_fseek(pFile, 0, SEEK_SET);
    nFileLength = tp_flength(pFile);
    GPS_PRINT("nFileLength=%d",nFileLength);
    if( nFileLength >= nMaxFileSize )
    {
        GPS_PRINT_ERROR
        tp_fclose(pFile);
        return  0;
    }

    //计算需要写入到文件的数据长度
    tp_fseek(pFile, 0, SEEK_END);
    if( (nFileLength + nBufferSize) > GPS_BUFFER_LEN )
    {
        nLength = GPS_BUFFER_LEN - nFileLength;
    }
    else
    {
        nLength = nBufferSize;
    }

    //把数据写入到指定的文件中
    nWriteLength = tp_fwrite( pBuffer, 1, nLength, pFile );
    tp_fclose(pFile);
    if( nLength != nWriteLength )
    {//NEED_TO_DO_ERROR
        //缺少容错处理
        GPS_PRINT_ERROR
    }      
    
    //成功，返回写入文件的buffer大小
    GPS_PRINT("nWriteLength=%d",nWriteLength);
    return  nWriteLength;
}

static int goku_gps_buffer_file_write_one_file(int nIndex, UINT8* pBuffer,UINT32 nBufferSize)
{
    char    chFilePathName[MAX_PATH];
    int nWriteLength;

    GPS_PRINT("nIndex=%d,nBufferSize=%d",nIndex,nBufferSize);
        
    //文件名固定格式约定
    sprintf( chFilePathName, "%s/%05d",GPS_BUFFER_FILE_PATH, nIndex );    
    nWriteLength    = goku_gps_buffer_file_write_buffer( chFilePathName, 
        pBuffer, nBufferSize,GPS_BUFFER_LEN );
    if( nWriteLength > 0 )
    {//写入成功后，累加buffer大小
        g_gps_cache.nFileBufferSize += nWriteLength;
    }
    
    return  nWriteLength;
}

static int goku_gps_buffer_file_write_files(UINT8* pBuffer,UINT32 nBufferSize,UINT32* pIndex)
{//pIndex:起始的index值
    int nReMainLength;//剩下待写入的数据长度
    int nError  = 0;
    int nWriteLength;//写入到文件的数据长度
    int nIndex;

    nIndex  = *pIndex;
    GPS_PRINT("nBufferSize=%d,nIndex=%d",nBufferSize,nIndex);
    
    nReMainLength = nBufferSize;
    while( nReMainLength > 0 )
    {
        nWriteLength    = goku_gps_buffer_file_write_one_file( nIndex, 
            pBuffer+nBufferSize-nReMainLength, 
            nReMainLength );
        if( nWriteLength < 0 )
        {//写文件出错
            break;
        }
        
        nReMainLength -= nWriteLength;
        if( nReMainLength == 0 )
        {//已写完
            break;
        }
        
        //把buffer继续写到下一个文件
        nIndex++;
    }

    *pIndex  = nIndex;//赋值为最后写的文件下标
    GPS_PRINT("write length=%d,nIndex=%d",(nBufferSize - nReMainLength),nIndex);
    //返回写到文件的buffer大小
    return  nBufferSize - nReMainLength;
}

static int goku_gps_buffer_file_write_ex(UINT8* pBuffer,UINT32 nBufferSize)
{
    int nWriteLength;//写入到文件的数据长度

    GPS_PRINT("nBufferSize=%d",nBufferSize);

    nWriteLength    = goku_gps_buffer_file_write_files(pBuffer,nBufferSize,
        &g_gps_cache.nFileIndexWrite);
    
    return  nWriteLength;
}

static gboolean goku_gps_buffer_file_write(UINT8* pBuffer,UINT32 nBufferSize)
{
    int nLength;
    int nStartIndex;
    int nEndIndex;
    
    GPS_PRINT("nBufferSize=%d",nBufferSize);
    if( !goku_gps_buffer_file_check_disk() )
    {
        GPS_PRINT_ERROR
        return  FALSE;
    }

    if( g_gps_cache.nFileBufferSize == 0 )
    {//第一次写文件，文件下标从GPS_BUFFER_MAX_COUNT+1开始
        g_gps_cache.nFileIndexRead      = GPS_BUFFER_MAX_COUNT+1;
        g_gps_cache.nFileIndexWrite     = GPS_BUFFER_MAX_COUNT+1;
    }
    
    nLength    = goku_gps_buffer_file_write_ex(pBuffer,nBufferSize);    
    if( g_gps_cache.nFileBufferSize == 0 )
    {//写失败了，文件下标重置为无效值0
        g_gps_cache.nFileIndexRead      = 0;
        g_gps_cache.nFileIndexWrite     = 0;
    }    
    goku_gps_buffer_file_save_setting();
    
    if( nLength != nBufferSize )
    {//NEED_TO_DO_ERROR
        //目前写了多少算多少，写失败的丢弃，后续优化
        GPS_PRINT_ERROR
        return  FALSE;
    }
    
    return  TRUE;
}

static gboolean goku_gps_buffer_decrease()
{
    GPS_PRINT_ENTRY

    GPS_FREE(g_gps_cache.pCache);
    g_gps_cache.pCache  = (UINT8 *)tp_os_mem_malloc(GPS_BUFFER_LEN);
    if( NULL == g_gps_cache.pCache )
    {//NEED_TO_DO_ERROR
        GPS_PRINT("Not Enough Memory!");
        return FALSE;
    }

    g_gps_cache.pRead       = g_gps_cache.pCache;
    g_gps_cache.pWrite      = g_gps_cache.pCache;
    g_gps_cache.uCacheLen   = GPS_BUFFER_LEN;
    g_gps_cache.nTurnFlag   = 0;
    return TRUE;    
}

static gboolean goku_gps_buffer_increase()
{
    GPS_FUFFER buf_tmp = {0};
    UINT32 wsft = 0;
    UINT32 rsft = 0;
    GPS_FUFFER *gps_buff;
    
    GPS_PRINT_ENTRY

    gps_buff    = &g_gps_cache;
    if( gps_buff->uCacheLen >= GPS_BUFFER_MAX_LEN )
    {
        GPS_PRINT_ERROR
        return FALSE;
    }

    tp_os_mem_cpy(&buf_tmp,gps_buff,sizeof(GPS_FUFFER));

    if(NULL == (buf_tmp.pCache = (UINT8 *)tp_os_mem_malloc(buf_tmp.uCacheLen+GPS_BUFFER_LEN)))
    {
        GPS_PRINT_ERROR
        return FALSE;
    }

    tp_os_mem_cpy(buf_tmp.pCache,gps_buff->pCache,gps_buff->uCacheLen);//复制buff到临时GPS_BUFF
    
    wsft = gps_buff->pWrite - gps_buff->pCache;

    if(buf_tmp.nTurnFlag)
    {
        if(wsft)
        {
            tp_os_mem_cpy(buf_tmp.pCache+buf_tmp.uCacheLen,buf_tmp.pCache,wsft);
        }
        buf_tmp.pWrite = buf_tmp.pCache + buf_tmp.uCacheLen + wsft;
        buf_tmp.nTurnFlag = 0;
    }
    else
    {
        buf_tmp.pWrite = buf_tmp.pCache + wsft;
    }
    rsft = gps_buff->pRead- gps_buff->pCache;
    buf_tmp.pRead = buf_tmp.pCache + rsft;
    
    buf_tmp.uCacheLen += GPS_BUFFER_LEN;

    GPS_FREE(gps_buff->pCache);
    
    tp_os_mem_cpy(gps_buff,&buf_tmp,sizeof(GPS_FUFFER));
    return TRUE;    
}

static gboolean goku_gps_buffer_init_ex()
{
    GPS_PRINT_ENTRY
    
    if( OS_MUTEX_INVALID_ID == g_gps_buffer_mutex )
    {
        GPS_PRINT("create mutex");
        g_gps_buffer_mutex = tp_os_mutex_create("gps_buffer_mutex",OS_TRUE);
        if( OS_MUTEX_INVALID_ID == g_gps_buffer_mutex )
        {
            GPS_PRINT_ERROR
            return FALSE;
        }
    }
    
    if(NULL == g_gps_cache.pCache)
    {
        GPS_PRINT("malloc cache");
        g_gps_cache.pCache = (UINT8*)tp_os_mem_malloc(GPS_BUFFER_LEN);
        if(NULL == g_gps_cache.pCache)
        {
            GPS_PRINT_ERROR
            return FALSE;
        }
        g_gps_cache.uCacheLen           = GPS_BUFFER_LEN;
        g_gps_cache.pRead               = g_gps_cache.pCache;
        g_gps_cache.pWrite              = g_gps_cache.pCache;
        g_gps_cache.nTurnFlag           = 0;
        g_gps_cache.nFileBufferSize     = 0;   
        g_gps_cache.nFileIndexRead      = 0;   
        g_gps_cache.nFileIndexWrite     = 0; 
        goku_gps_buffer_file_load();
    }


    GPS_PRINT("GPS buffer init OK!");
    return TRUE;
}

static gboolean goku_gps_buffer_init_check()
{
    gboolean    bInit;
    
    if( g_b_buffer_init )
    {
        return  TRUE;
    }
    bInit   = goku_gps_buffer_init_ex();
    if( bInit )
    {
        g_b_buffer_init = TRUE;
    }
    return  bInit;    
}

static void goku_gps_buffer_free_ex()
{
    GPS_PRINT_ENTRY
    
    if( NULL != g_gps_cache.pCache )
    {
        GPS_PRINT("free cache");
        goku_gps_buffer_file_save();
        tp_os_mem_free(g_gps_cache.pCache);
        memset(&g_gps_cache,0x00,sizeof(g_gps_cache));
    }   
    
    if(OS_MUTEX_INVALID_ID != g_gps_buffer_mutex)
    {
        GPS_PRINT("free mutex");
        tp_os_mutex_get(g_gps_buffer_mutex, OS_WAIT_FOREVER );
        tp_os_mutex_delete(g_gps_buffer_mutex);
        g_gps_buffer_mutex = OS_MUTEX_INVALID_ID;
    }
    g_b_buffer_init = FALSE;
}

static gboolean goku_gps_buffer_read_done(UINT8* pBuffer,UINT32 nBufferSize)
{
    UINT32 rsft = 0;
    UINT8* ptmp = NULL;
    UINT32   uUsed_mem;

    uUsed_mem = goku_gps_buffer_get_mem_used_size();
    GPS_PRINT("uUsed_mem = %d,nBufferSize = %d",uUsed_mem,nBufferSize);
    if(uUsed_mem < nBufferSize)
    {
        GPS_PRINT_ERROR
        return FALSE;
    }
    
    if(g_gps_cache.nTurnFlag)
    {
        rsft = g_gps_cache.pCache + g_gps_cache.uCacheLen - g_gps_cache.pRead;
        if(nBufferSize <= rsft)
        {
            tp_os_mem_cpy(pBuffer,g_gps_cache.pRead,nBufferSize);
            g_gps_cache.pRead += nBufferSize;
            if(nBufferSize == rsft)
            {
                g_gps_cache.pRead = g_gps_cache.pCache;
                g_gps_cache.nTurnFlag = 0;
            }
        }
        else
        {
            ptmp = (UINT8*)pBuffer;
            tp_os_mem_cpy(ptmp,g_gps_cache.pRead,rsft);
            ptmp += rsft;
            g_gps_cache.nTurnFlag = 0;
            g_gps_cache.pRead = g_gps_cache.pCache;
            tp_os_mem_cpy(ptmp,g_gps_cache.pRead,nBufferSize-rsft); 
            g_gps_cache.pRead += nBufferSize-rsft;
        }
    }
    else
    {
        tp_os_mem_cpy(pBuffer,g_gps_cache.pRead,nBufferSize);
        g_gps_cache.pRead += nBufferSize;
    }


    return TRUE;
}

static gboolean goku_gps_buffer_read_ex(UINT8* pBuffer,UINT32 nBufferSize)
{
    gboolean    bRes;
    UINT32   nUsedSize;
    UINT8* pTemp;
    UINT32 nRemainLength;

    GPS_PRINT("nBufferSize=%d",nBufferSize);

    pTemp           = pBuffer;
    nRemainLength   = nBufferSize;   
    
    nUsedSize       = goku_gps_buffer_get_mem_used_size();
    if( (nUsedSize < nRemainLength) && (nUsedSize > 0) )
    {//内存中数据不足,先把内存中的数据读完
        bRes    = goku_gps_buffer_read_done(pTemp,nUsedSize);
        if( !bRes )
        {
            return FALSE;
        }
        nRemainLength   -= nUsedSize;
        pTemp   += nUsedSize;
        nUsedSize       = goku_gps_buffer_get_mem_used_size();
    }
    
    if( (nUsedSize == 0) && g_gps_cache.uCacheLen > GPS_BUFFER_LEN )
    {//内存中的数据已空，释放多余内存
        if( !goku_gps_buffer_decrease() )
        {
            return  FALSE;
        }
    }
    
    if( nUsedSize == 0 )
    {//内存中没有数据，从文件中加载数据到内存
        goku_gps_buffer_file_load_one_file();
        nUsedSize       = goku_gps_buffer_get_mem_used_size();
    }

    if( nUsedSize < nRemainLength )
    {//内存中数据不足
        GPS_PRINT_ERROR
        return FALSE;
    }

    bRes    = goku_gps_buffer_read_done( pTemp, nRemainLength );
    GPS_PRINT("bRes=%d",bRes);

    return bRes;
}


static gboolean goku_gps_buffer_write_done(UINT8* pBuffer,UINT32 nBufferSize)
{
    UINT32 t_rmem = 0;
    UINT8*  pTemp = NULL;
   
    GPS_PRINT("nBufferSize=%d",nBufferSize);

    if(g_gps_cache.nTurnFlag)
    {
        tp_os_mem_cpy(g_gps_cache.pWrite,pBuffer,nBufferSize);
        g_gps_cache.pWrite += nBufferSize;
    }
    else
    {
        t_rmem = g_gps_cache.pCache + g_gps_cache.uCacheLen - g_gps_cache.pWrite;
        if(nBufferSize <= t_rmem)
        {
            tp_os_mem_cpy(g_gps_cache.pWrite,pBuffer,nBufferSize);
            g_gps_cache.pWrite += nBufferSize;
            if(nBufferSize == t_rmem)
            {
                g_gps_cache.pWrite = g_gps_cache.pCache;
                g_gps_cache.nTurnFlag = 1;
            }
        }
        else
        {
            pTemp = pBuffer;
            tp_os_mem_cpy(g_gps_cache.pWrite,pTemp,t_rmem);
            g_gps_cache.pWrite = g_gps_cache.pCache;
            g_gps_cache.nTurnFlag = 1;                
            pTemp += t_rmem;
            tp_os_mem_cpy(g_gps_cache.pWrite,pTemp,nBufferSize - t_rmem);
            g_gps_cache.pWrite += nBufferSize - t_rmem;
        }
    }
    return TRUE;
}

static gboolean goku_gps_buffer_write_ex(UINT8* pBuffer,UINT32 nBufferSize)
{
    gboolean    bRes;
    UINT8* pTemp = pBuffer;
    UINT32 nLength = nBufferSize;
    gboolean    bWriteFile  = FALSE;
    UINT32   uUnusedSize;
   
    GPS_PRINT("nBufferSize=%d",nBufferSize);
    
    uUnusedSize = goku_gps_buffer_get_mem_unused_size();
    if( (g_gps_cache.nFileBufferSize == 0) &&   //当前为写内存
        (uUnusedSize < nLength) 
        )
    {
        if( uUnusedSize < nLength )
        {//申请的内存不够
            if(!goku_gps_buffer_increase())
            {//重新申请内存失败后，开始写文件
                bWriteFile   = TRUE;
                //先把剩余的内存写满
                bRes    = goku_gps_buffer_write_done(pTemp,uUnusedSize);   
                pTemp   += uUnusedSize;
                nLength -= uUnusedSize;
            }
        }
    }
    
    if( g_gps_cache.nFileBufferSize > 0 || //一旦文件中有数据，则后续的数据都直接写到文件中
        bWriteFile//内存满了，开始写文件
        )
    {//把数据写入到文件中
        bRes    = goku_gps_buffer_file_write(pTemp,nLength);
    }
    else
    {//把数据写入到内存中
        bRes    = goku_gps_buffer_write_done(pTemp,nLength);
    }
    
    GPS_PRINT("bRes=%d",bRes);
    return bRes;
}

gboolean goku_gps_buffer_read(UINT8* pBuffer,UINT32 nBufferSize)
{
    gboolean bRes;
    
    if( pBuffer == NULL || nBufferSize <= 0 )
    {
        return  FALSE;
    }

    if( !goku_gps_buffer_init_check() )
    {
        return  FALSE;
    }
    
    tp_os_mutex_get(g_gps_buffer_mutex, OS_WAIT_FOREVER );

    bRes    = goku_gps_buffer_read_ex( pBuffer, nBufferSize );

     tp_os_mutex_put(g_gps_buffer_mutex);
    return bRes;
}

gboolean goku_gps_buffer_write(UINT8* pBuffer,UINT32 nBufferSize)
{
    gboolean bRes;
    
     if( pBuffer == NULL || nBufferSize <= 0 )
    {
        return  FALSE;
    }
   
    if( !goku_gps_buffer_init_check() )
    {
        return  FALSE;
    }
    
    tp_os_mutex_get(g_gps_buffer_mutex, OS_WAIT_FOREVER);

    bRes    = goku_gps_buffer_write_ex( pBuffer, nBufferSize);    

    tp_os_mutex_put(g_gps_buffer_mutex);
    return bRes;
}

UINT32 goku_gps_buffer_get_count()
{
    UINT32 nRet;

    if( !goku_gps_buffer_init_check() )
    {
        return  0;
    }

    tp_os_mutex_get(g_gps_buffer_mutex, OS_WAIT_FOREVER);
    
    nRet    = goku_gps_buffer_get_used_size_ex()/GPS_UPLOADCMD_LEN;   
    GPS_PRINT("count=%d",nRet);
    
    tp_os_mutex_put(g_gps_buffer_mutex);    
    return nRet;
}

UINT32 goku_gps_buffer_get_used_size()
{
    UINT32 nRet;

    if( !goku_gps_buffer_init_check() )
    {
        return  0;
    }

    tp_os_mutex_get(g_gps_buffer_mutex, OS_WAIT_FOREVER);
    
    nRet    = goku_gps_buffer_get_used_size_ex();   
    
    tp_os_mutex_put(g_gps_buffer_mutex);    
    return nRet;
}

void goku_gps_buffer_free()
{
    goku_gps_buffer_free_ex();
}

gboolean goku_gps_buffer_init()
{
    return  goku_gps_buffer_init_check();
}

UINT32 goku_gps_print_recv_bytes_total()
{
    PRINT_FUNC_LINE("g_uRecvTotalbyte = %ld bytes",g_uRecvTotalbyte);
    return g_uRecvTotalbyte;
}
void goku_gps_recv_bytes_total_reset()
{
    g_uRecvTotalbyte = 0;
}
