/* filename          /ImageBuilder.c
 * date              2018.11.06
 * last edit date    2018.11.06
 * author            NO.00[UNKNOWN]
 * brief             ImageBuilder source file
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define BYTESOFSECTOR  512

int AdjustInSectorSize( int iFd, int iSourceSize );
void WriteKernelInformation( int iTargetFd, int iKernelSectorCount );
int CopyFile( int iSourceFd, int iTargetFd );

int main(int argc, char* argv[])
{
    int iSourceFd;
    int iTargetFd;
    int iBootLoaderSize;
    int iKernel32SectorCount;
    int iSourceSize;
        
    // check command line options
    if ( argc < 3 )
    {
        fprintf( stderr, "[ERROR] ImageBuiler.exe BootLoader.bin Kernel32.bin\n" );
        exit( -1 );
    }
    
    // Build Disk.img
    if ( ( iTargetFd = open( "Disk.img", O_RDWR | O_CREAT |  O_TRUNC |
            O_BINARY, S_IREAD | S_IWRITE ) ) == -1 )
    {
        fprintf( stderr , "[ERROR] Disk.img open fail.\n" );
        exit( -1 );
    }

    printf( "[INFO] Copy boot loader to image file\n" );
    if ( ( iSourceFd = open( argv[ 1 ], O_RDONLY | O_BINARY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 1 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    // rest of file fill with 0x00 to fit 512 byte
    iBootLoaderSize = AdjustInSectorSize( iTargetFd , iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
            argv[ 1 ], iSourceSize, iBootLoaderSize );

    printf( "[INFO] Copy protected mode kernel to image file\n" );
    if ( ( iSourceFd = open( argv[ 2 ], O_RDONLY | O_BINARY ) ) == -1 )
    {
        fprintf( stderr, "[ERROR] %s open fail\n", argv[ 2 ] );
        exit( -1 );
    }

    iSourceSize = CopyFile( iSourceFd, iTargetFd );
    close( iSourceFd );
    
    // rest of file fill with 0x00 to fit 512 byte
    iKernel32SectorCount = AdjustInSectorSize( iTargetFd, iSourceSize );
    printf( "[INFO] %s size = [%d] and sector count = [%d]\n",
                argv[ 2 ], iSourceSize, iKernel32SectorCount );

    // Update kernel size information on bootloader
    printf( "[INFO] Start to write kernel information\n" );    
    // kernel size info on 5th byte of bootloader
    WriteKernelInformation( iTargetFd, iKernel32SectorCount );
    printf( "[INFO] Image file create complete\n" );

    close( iTargetFd );
    return 0;
}

/**
 *  function name : AdjustInSectorSize
 *  Parameters    : iFd(int) - target file
 *                  iSourceSize(int) - size of sector(byte)
 *  brief         : fill rest of file with 0x00
 */
int AdjustInSectorSize( int iFd, int iSourceSize )
{
    int i;
    int iAdjustSizeToSector;
    char cCh;
    int iSectorCount;

    iAdjustSizeToSector = iSourceSize % BYTESOFSECTOR;
    cCh = 0x00;
    
    if ( iAdjustSizeToSector != 0 )
    {
        iAdjustSizeToSector = 512 - iAdjustSizeToSector;
        printf( "[INFO] File size [%lu] and fill [%u] byte\n", iSourceSize, 
            iAdjustSizeToSector );
        for ( i = 0 ; i < iAdjustSizeToSector ; i++ )
            write( iFd , &cCh , 1 );
    }
    else printf( "[INFO] File size is aligned 512 byte\n" );
    
    
    // return number of sector
    iSectorCount = ( iSourceSize + iAdjustSizeToSector ) / BYTESOFSECTOR;
    return iSectorCount;
}

/**
 *  function name : WriteKernelInformation
 *  Parameters    : iTargetFd(int) - target file
 *                  iKernelSectorCount(int) - size of sector(byte)
 *  brief         : modify sector number information on bootloader
 */
void WriteKernelInformation( int iTargetFd, int iKernelSectorCount )
{
    unsigned short usData;
    long lPosition;
    
    // 5th byte is target
    lPosition = lseek( iTargetFd, 5, SEEK_SET );
    if ( lPosition == -1 )
    {
        fprintf( stderr, "lseek fail. Return value = %d, errno = %d, %d\n", 
            lPosition, errno, SEEK_SET );
        exit( -1 );
    }

    usData = ( unsigned short ) iKernelSectorCount;
    write( iTargetFd, &usData, 2 );

    printf( "[INFO] Total sector count except boot loader [%d]\n", 
        iKernelSectorCount );
}

/**
 *  function name : CopyFile
 *  Parameters    : iSourceFd(int) - source file
 *                  iTargetFd(int) - target file
 *  brief         : copy target file to source file and return its size
 */
int CopyFile( int iSourceFd, int iTargetFd )
{
    int iSourceFileSize;
    int iRead;
    int iWrite;
    char vcBuffer[ BYTESOFSECTOR ];

    iSourceFileSize = 0;
    while ( 1 )
    {
        iRead   = read( iSourceFd, vcBuffer, sizeof( vcBuffer ) );
        iWrite  = write( iTargetFd, vcBuffer, iRead );

        if ( iRead != iWrite )
        {
            fprintf( stderr, "[ERROR] iRead != iWrite.. \n" );
            exit(-1);
        }
        iSourceFileSize += iRead;
        
        if ( iRead != sizeof( vcBuffer ) ) break;
    }
    return iSourceFileSize;
} 