#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "zlib/zlib.h"
#include "ziptool.h"

typedef char bool;
#define true 1
#define false 0

struct ZipLocalHeader
{
	// 'P' 'K' 03 04
	uint16_t ver; // version_needed_to_extract
	uint16_t flg; // general_purpose_bit_flag
	uint16_t mhd; // compression_method
	uint16_t tim; // last_modified_file_time
	uint16_t dat; // last_modified_file_date
	unsigned int crc; // crc32
	unsigned int csz; // compressed-size
	unsigned int usz; // uncompressed-size
	uint16_t fnl; // filename-len
	uint16_t exl; // extra_field_length

	char fnm[250];
	//	unsigned char ext[];
};

struct zipdir {
	char *name;
	int position;
};


enum ZipMethod
{
	Stored,         // 0
	Shrunk,                 // 1
	Reduced1,               // 2-5
	Reduced2,
	Reduced3,
	Reduced4,
	Imploded,               // 6
	Tokenized,              // 7 ( not supported )
	Deflated,               // 8
	EnhDeflated,        // 9 ( not supported )
	DclImploded,        //10 ( not supported )

	Err=178             // this value is used by xacrett (^^;
};

struct sf_entry
{
	uint16_t Code;
	unsigned char Value;
	unsigned char BitLength;
};

struct  sf_tree
{
	struct sf_entry entry[256];
	int entries;
	int MaxLength;
};

#define ZIPTOOL_CACHE_SIZE 16384

struct CZipTool
{

int open;
int zipnbrc;
struct zipdir *zipdirc;

char zipfile[256];

unsigned char *cached;
int cached_from;
int cached_max;


// CRC

int szip;

// VARIABLE

unsigned char* common_buf;

char lb[256];

unsigned int wrtcrc;

// bit-reader
unsigned long bitbuf;
int bits_left;
bool bits_eof;

unsigned int keys[3];

unsigned char *kzip, *kout;
int pzip, pout;
int mzip, mout;


};


void CZipToolInit(struct CZipTool *CZ, unsigned char *buffer, unsigned int size);

void ReadDir(struct CZipTool *CZ);

int kgetc(struct CZipTool *CZ);
int kread(struct CZipTool *CZ, unsigned char *dest, int count);
int kwrite(struct CZipTool *CZ, unsigned char *dest, int count);

void pathInit(struct CZipTool *CZ);


void pathSplit( struct CZipTool *CZ, const char* path, int* y, int* d );
const char* pathExt( struct CZipTool *CZ, const char* path );
const char* pathName( struct CZipTool *CZ, const char* path );


bool ReadFromZIP( struct CZipTool *CZ, char *filter, unsigned int *dsize, unsigned char **dBuffer);


void zipwrite( struct CZipTool *CZ, unsigned char* dat, int len );

int zipread( struct CZipTool *CZ, unsigned char* dat, int len );

void SortLengths( struct CZipTool *CZ, struct sf_tree* tree );
void GenerateTrees( struct CZipTool *CZ, struct sf_tree* tree );
void ReverseBits( struct CZipTool *CZ, struct sf_tree* tree );

void initbits(struct CZipTool *CZ);
int getbits( struct CZipTool *CZ, int n );
int fillbits( struct CZipTool *CZ, int n );

void Unstore( struct CZipTool *CZ, unsigned int usz, unsigned int csz );
void Inflate( struct CZipTool *CZ, unsigned int usz, unsigned int csz );
void Unshrink( struct CZipTool *CZ, unsigned int usz, unsigned int csz );

// unreduce
void Unreduce( struct CZipTool *CZ, unsigned int usz, unsigned int csz, int factor );
void LoadFollowers( struct CZipTool *CZ, unsigned char* Slen, unsigned char followers[][64] );

// explode
void Explode( struct CZipTool *CZ, unsigned int usz, unsigned int csz, bool _8k,bool littree );
void LoadTree( struct CZipTool *CZ, struct sf_tree* tree, int treesize );
int  ReadTree( struct CZipTool *CZ, struct sf_tree* tree );
void ReadLengths( struct CZipTool *CZ, struct sf_tree* tree );

bool read_header( struct CZipTool *CZ, struct ZipLocalHeader* hdr);

bool doHeader( struct CZipTool *CZ, struct ZipLocalHeader* hdr);



#define XACR_BUFSIZE (0x4000)
// must be bigger than 0x4000 !! ( for ZIP )

static int MyComp( const char* str1, const char* str2);

inline char tolower(const char toLower)
{
	if ((toLower >= 'A') && (toLower <= 'Z'))
		return (char)(toLower + 0x20);

	return toLower;
}

int MyComp( const char* str1, const char* str2)
{
	int i;

	i=0;

	while(1) {
		int c1,c2;

		c1=str1[i];
		c2=str2[i];

		if ((c1==0) && (c2==0)) {
			return 0;
		}

		c1=tolower(c1);
		c2=tolower(c2);

		if (c1=='\\') c1='/';
		if (c2=='\\') c2='/';

		if (c1>c2) {
			return 1;
		}
		if (c1<c2) {
			return -1;
		}
		i++;
	}

	return -1;
}

int compare( const void *arg1, const void *arg2 )
{
	struct zipdir *alb1, *alb2;

	alb1=(struct zipdir*)arg1;
	alb2=(struct zipdir*)arg2;

	return MyComp( alb1->name, alb2->name);
}


void CZipToolInit(struct CZipTool *CZ, unsigned char *buffer, unsigned int size)
{
	CZ->kzip=buffer;
	CZ->mzip=size;

	CZ->cached=NULL;

	CZ->zipdirc=NULL;
	CZ->zipnbrc=-1;

	CZ->kout=NULL;
	CZ->common_buf=NULL;

	CZ->szip=size;

	pathInit(CZ);
	CZ->common_buf = (unsigned char*)malloc(XACR_BUFSIZE*sizeof(unsigned char));

	ReadDir(CZ);

	CZ->open=1;
}




void ReadDir(struct CZipTool *CZ)
{
	int max=100;

	if (CZ->zipnbrc!=-1) {
		return;
	}

	CZ->zipdirc=(struct zipdir*)malloc(sizeof(struct zipdir)*max);

	CZ->zipdirc=NULL;
	CZ->zipnbrc=-1;

	struct ZipLocalHeader hdr;

	CZ->pzip=0;
	CZ->pout=0;

	CZ->zipnbrc=0;

	CZ->zipdirc=(struct zipdir*)malloc(sizeof(struct zipdir)*max);

	while(1)
	{
		int oldpos;

		oldpos=CZ->pzip;

		if (doHeader(CZ, &hdr)==0) break;
		if (CZ->zipdirc==NULL) {
			CZ->zipnbrc=0;
			break;
		}
		CZ->zipdirc[CZ->zipnbrc].name = (char*)malloc(strlen(hdr.fnm)+1);
		strcpy(CZ->zipdirc[CZ->zipnbrc].name, hdr.fnm);
		CZ->zipdirc[CZ->zipnbrc].position=oldpos;

		CZ->pzip = CZ->pzip + hdr.csz;
		CZ->zipnbrc++;

		if (CZ->zipnbrc>=max) {
			max+=100;
			CZ->zipdirc=(struct zipdir*)realloc(CZ->zipdirc, sizeof(struct zipdir)*max);
		}
	}

	qsort(CZ->zipdirc, CZ->zipnbrc, sizeof(struct zipdir), compare);

	return;
}

void CZipToolClean(struct CZipTool *CZ)
{
	if (CZ->common_buf != NULL) {
		free(CZ->common_buf);
	}
	if (CZ->cached != NULL) {
		free(CZ->cached);
	}
}

int kgetc(struct CZipTool *CZ)
{
	int a;

	if (CZ->pzip<CZ->mzip) {
		a=CZ->kzip[CZ->pzip];
		CZ->pzip++;
	} else {
		a=EOF;
	}

	return a;
}


int kread(struct CZipTool *CZ, unsigned char *dest, int count)
{
	if (CZ->pzip+count > CZ->mzip) {
		count=CZ->mzip-CZ->pzip;
	}

	memcpy(dest, CZ->kzip+CZ->pzip, count);
	CZ->pzip+=count;

	return count;
}

int kwrite(struct CZipTool *CZ, unsigned char *dest, int count)
{
	memcpy(CZ->kout+CZ->pout, dest, count);
	CZ->pout+=count;

	return count;
}

void initbits(struct CZipTool *CZ)
{
	CZ->bits_eof=false, CZ->bits_left=0, CZ->bitbuf=0;
}

int getbits( struct CZipTool *CZ, int n )
{
	if( n <= CZ->bits_left )
	{
		int c = (int)(CZ->bitbuf & ((1<<n)-1));
		CZ->bitbuf >>= n;
		CZ->bits_left -= n;
		return c;
	}
	return fillbits( CZ, n );
}

int fillbits( struct CZipTool *CZ, int n )
{
	unsigned char next;

	if( !zipread( CZ, &next,1 ) )
		CZ->bits_eof = true;
	else
	{
		CZ->bitbuf |= (next<<CZ->bits_left);
		CZ->bits_left += 8;

		if( zipread( CZ, &next,1 ) )
		{
			CZ->bitbuf |= (next<<CZ->bits_left);
			CZ->bits_left += 8;
		}
	}

	int c = (int)(CZ->bitbuf & ((1<<n)-1));
	CZ->bitbuf >>= n;
	CZ->bits_left -= n;
	return c;
}


int zipread( struct CZipTool *CZ, unsigned char* dat, int len )
{
	len = kread( CZ, dat, len);
	return len;
}


void zipwrite( struct CZipTool *CZ, unsigned char* dat, int len )
{
	kwrite( CZ, dat, len);
}



void pathInit(struct CZipTool *CZ)
{
}



void pathSplit( struct CZipTool *CZ, const char* path, int* y, int* d )
{
	*y=-1, *d=-1;
	for( const char* x=path; *x!='\0'; x++)
	{
		if( *x=='\\' || *x=='/' ) *y=x-path,*d=-1;
		else if( *x=='.' ) *d=x-path;
	}
}

const char* pathExt( struct CZipTool *CZ, const char* path )
{
	int y,d;
	pathSplit( CZ, path, &y, &d );
	return (d!=-1) ? path+d+1 : path+strlen(path);
}

const char* pathName( struct CZipTool *CZ, const char* path )
{
	int y,d;
	pathSplit( CZ, path,&y,&d );
	return path+y+1;
}


/*
   bool Check( const char* fname, unsigned long fsize )
   {
   const unsigned char* hdr = common_buf;
   unsigned long siz = (fsize>XACR_BUFSIZE ? XACR_BUFSIZE : fsize);
   //--------------------------------------------------------------------//

   int i;
   for( i=0; i<(signed)siz-30; i++ )
   {
                if( hdr[i+0] != 'P'  )continue;
                if( hdr[i+1] != 'K'  )continue;
                if( hdr[i+2] != 0x03 )continue;
                if( hdr[i+3] != 0x04 )continue;
                if( hdr[i+8]+(hdr[i+9]<<8) > 12 )continue;
                if( hdr[i+26]==0 && hdr[i+27]==0 )continue;
                break;
                }
                if( (unsigned)i+30>=siz )
                return false;

                  if( !(zip=fopen( fname, "rb" )) )
                  return false;
                  fseek( zip, i+4, SEEK_SET );
                  struct ZipLocalHeader zhdr;
                  bool ans = read_header( &zhdr );
                  fclose( zip );

                        return ans;
                        }
 */

bool ReadFromZIP( struct CZipTool *CZ, char *filter, unsigned int *dsize, unsigned char **dBuffer)
{
	struct ZipLocalHeader hdr;

	CZ->pzip=0;
	CZ->pout=0;

	int i,j,k;

	i=0;
	j=CZ->zipnbrc-1;
	k=0;

	if (CZ->zipnbrc<=0) {
		*dBuffer=NULL;
		*dsize=0;

		return 0;
	}

	while(i<=j) {
		k=(i+j)/2;

		if (!MyComp(CZ->zipdirc[k].name, filter)) {
			break;
		}

		if (MyComp(CZ->zipdirc[k].name, filter)>0) {
			j=k-1;
		} else {
			i=k+1;
		}
	}

	if (!MyComp(CZ->zipdirc[k].name, filter)) {
		CZ->pzip=CZ->zipdirc[k].position;

		doHeader(CZ, &hdr);

		CZ->kout=(unsigned char*)malloc(hdr.usz);
		CZ->pout=0;
		CZ->mout=0; // ??

		switch( hdr.mhd )
		{
		case Stored:
			Unstore( CZ, hdr.usz, hdr.csz );
			break;
		case Deflated:
			Inflate( CZ, hdr.usz, hdr.csz );
			break;
		case Shrunk:
			Unshrink( CZ, hdr.usz, hdr.csz );
			break;
		case Reduced1:
			Unreduce( CZ, hdr.usz, hdr.csz, 1 );
			break;
		case Reduced2:
			Unreduce( CZ, hdr.usz, hdr.csz, 2 );
			break;
		case Reduced3:
			Unreduce( CZ, hdr.usz, hdr.csz, 3 );
			break;
		case Reduced4:
			Unreduce( CZ, hdr.usz, hdr.csz, 4 );
			break;
		case Imploded:
			Explode( CZ, hdr.usz, hdr.csz,       0!=(hdr.flg&0x02), 0!=((hdr.flg)&0x04) );
			break;
		}

		if (CZ->pout!=0) {
			*dBuffer=CZ->kout;
			*dsize=CZ->pout;
		} else {
			*dBuffer=NULL;
			*dsize=0;
		}


	} else {
		*dBuffer=NULL;
		*dsize=0;
		return false;
	}


	return true;
}






bool read_header( struct CZipTool *CZ, struct ZipLocalHeader* hdr)
{
	if( 26 != kread(CZ, CZ->common_buf, 26) ) {
		return false;
	}

	hdr->ver = ((CZ->common_buf[ 0])|(CZ->common_buf[ 1]<<8));
	hdr->flg = ((CZ->common_buf[ 2])|(CZ->common_buf[ 3]<<8));
	hdr->mhd = ((CZ->common_buf[ 4])|(CZ->common_buf[ 5]<<8));
	hdr->tim = ((CZ->common_buf[ 6])|(CZ->common_buf[ 7]<<8));
	hdr->dat = ((CZ->common_buf[ 8])|(CZ->common_buf[ 9]<<8));
	hdr->crc = ((CZ->common_buf[10])|(CZ->common_buf[11]<<8)|(CZ->common_buf[12]<<16)|(CZ->common_buf[13]<<24));
	hdr->csz = ((CZ->common_buf[14])|(CZ->common_buf[15]<<8)|(CZ->common_buf[16]<<16)|(CZ->common_buf[17]<<24));
	hdr->usz = ((CZ->common_buf[18])|(CZ->common_buf[19]<<8)|(CZ->common_buf[20]<<16)|(CZ->common_buf[21]<<24));
	hdr->fnl = ((CZ->common_buf[22])|(CZ->common_buf[23]<<8));  // Longueur du filename
	hdr->exl = ((CZ->common_buf[24])|(CZ->common_buf[25]<<8));

	if (hdr->fnl>=256) {
		return false;
	}

	if( hdr->fnl!=kread(CZ, (unsigned char*)hdr->fnm, hdr->fnl) ) {  // common_buf <- filename
		return false;
	}

	hdr->fnm[hdr->fnl]=0;

	if( hdr->mhd > Deflated || hdr->mhd==Tokenized ) {
		return false;
	}
	if ((hdr->exl!=0) && (hdr->exl != kread(CZ, CZ->common_buf, hdr->exl)) ) {
		return false;
	}

	return true;
}

bool doHeader( struct CZipTool *CZ, struct ZipLocalHeader* hdr)
{
	unsigned char key[4];

	kread(CZ, key, 4);

	if ( (key[0]=='P') && (key[1]=='K') && (key[2]==0x03) && (key[3]==0x04) )       {
		int x=CZ->pzip;
		if (read_header(CZ, hdr)) {
			return true;
		}
		CZ->pzip = x;
	}

	return false;
}

////////////////////////////////////////////////////////////////
// Store
////////////////////////////////////////////////////////////////

void Unstore( struct CZipTool *CZ, unsigned int usz, unsigned int csz )
{
	unsigned char* buf = CZ->common_buf;
	//--------------------------------------------------------------------

	int how_much;
	while( csz )
	{
		how_much = csz > XACR_BUFSIZE ? XACR_BUFSIZE : csz;
		if( 0>=(how_much=zipread( CZ, buf, how_much )) )
			break;
		zipwrite( CZ, buf, how_much );
		csz -= how_much;
	}
}

////////////////////////////////////////////////////////////////
// Deflate
////////////////////////////////////////////////////////////////

void Inflate( struct CZipTool *CZ, unsigned int usz, unsigned int csz )
{
	unsigned char* outbuf = CZ->common_buf;
	unsigned char*  inbuf = CZ->common_buf + (XACR_BUFSIZE/2);
	//--------------------------------------------------------------------//

	// zlib
	struct z_stream_s zs;
	zs.zalloc   = NULL;
	zs.zfree    = NULL;

	int outsiz = (XACR_BUFSIZE/2);
	zs.next_out = outbuf;
	zs.avail_out= outsiz;

	int insiz = zipread( CZ, inbuf, (XACR_BUFSIZE/2) > csz ? csz : (XACR_BUFSIZE/2) );
	if( insiz<=0 )
		return;
	csz        -= insiz;
	zs.next_in  = inbuf;
	zs.avail_in = insiz;

	inflateInit2( &zs, -15 );

	int err = Z_OK;
	while( csz )
	{
		while( zs.avail_out > 0 )
		{
			err = inflate( &zs,Z_PARTIAL_FLUSH );
			if( err!=Z_STREAM_END && err!=Z_OK )
				csz=0;
			if( !csz )
				break;

			if( zs.avail_in<=0 )
			{
				int insiz = zipread( CZ, inbuf, (XACR_BUFSIZE/2) > csz ? csz : (XACR_BUFSIZE/2) );
				if( insiz<=0 )
				{
					err = Z_STREAM_END;
					csz = 0;
					break;
				}

				csz        -= insiz;
				zs.next_in  = inbuf;
				zs.avail_in = insiz;
			}
		}

		zipwrite( CZ, outbuf, outsiz-zs.avail_out );
		zs.next_out  = outbuf;
		zs.avail_out = outsiz;
	}

	while( err!=Z_STREAM_END )
	{
		err = inflate(&zs,Z_PARTIAL_FLUSH);
		if( err!=Z_STREAM_END && err!=Z_OK )
			break;

		zipwrite( CZ, outbuf, outsiz-zs.avail_out );
		zs.next_out  = outbuf;
		zs.avail_out = outsiz;
	}

	inflateEnd(&zs);
}

////////////////////////////////////////////////////////////////
// Shrink LZW with partial_clear PkZip 0.x-1.x
////////////////////////////////////////////////////////////////

void Unshrink( struct CZipTool *CZ, unsigned int usz, unsigned int csz )
{
	// 8192Bytes = 13bits
	unsigned char* stack     = CZ->common_buf;
	unsigned char* suffix_of = CZ->common_buf + 8192 + 1;
	signed int*  prefix_of = (signed int*)CZ->common_buf + (8192 + 1) * 2;

#define GetCode() getbits(CZ, codesize)
	int left=(signed)usz-1;
	//--------------------------------------------------------------------


	unsigned int codesize, maxcode, free_ent, offset, sizex;
	unsigned int code, stackp, finchar, oldcode, incode;

	initbits(CZ);
	codesize = 9;
	maxcode  = (1<<codesize) - 1;
	free_ent = 257;
	offset   = 0;
	sizex    = 0;

	for( code=(1<<13); code > 255; code-- )
		prefix_of[code] = -1;
	for( code=0; code <256; code++ )
	// for( code=255; code >= 0; code-- )
	{
		prefix_of[code] = 0;
		suffix_of[code] = code;
	}

	oldcode = GetCode();
	if( CZ->bits_eof )
		return;
	finchar = oldcode;
	unsigned char f=finchar;
	zipwrite( CZ, &f,1 );

	stackp = 8192;

	while( left>0 && !CZ->bits_eof )
	{
		code = GetCode();
		if( CZ->bits_eof )
			break;

		// clear!
		while( code == 256 )
		{
			switch( GetCode() )
			{
			case 1:
				codesize++;
				if( codesize == 13 )
					maxcode = (1 << codesize);
				else
					maxcode = (1 << codesize) - 1;
				break;

			case 2: // partial_clear !!
			{
				unsigned int pr,cd;

				for( cd=257; cd<free_ent; cd++ )
					prefix_of[cd] |= 0x8000;

				for( cd=257; cd<free_ent; cd++)
				{
					pr = prefix_of[cd] & 0x7fff;
					if( pr >= 257 )
						prefix_of[pr] &= 0x7fff;
				}

				for( cd=257; cd<free_ent; cd++ )
					if( (prefix_of[cd] & 0x8000) != 0 )
						prefix_of[cd] = -1;

				cd = 257;
				while( (cd < (1<<13)) && (prefix_of[cd] != -1) )
					cd++;
				free_ent = cd;
			}
			break;
			}

			code = GetCode();
			if( CZ->bits_eof )
				break;
		}
		if( CZ->bits_eof )
			break;

		incode = code;
		if( prefix_of[code] == -1 )
		{
			stack[--stackp] = finchar;
			code = oldcode;
		}

		while( code >= 257 )
		{
			stack[--stackp] = suffix_of[code];
			code = prefix_of[code];
		}
		finchar = suffix_of[code];
		stack[--stackp] = finchar;

		left -= (8192-stackp);
		zipwrite( CZ, stack+stackp, (8192-stackp) );
		stackp = 8192;


		code = free_ent;
		if( code < (1<<13) )
		{
			prefix_of[code] = oldcode;
			suffix_of[code] = finchar;

			do
				code++;
			while( (code < (1<<13)) && (prefix_of[code] != -1) );

			free_ent = code;
		}


		oldcode = incode;
	}

#undef GetCode
}

////////////////////////////////////////////////////////////////
// Reduce lz77 ; PkZip 0.x
////////////////////////////////////////////////////////////////

void LoadFollowers( struct CZipTool *CZ, unsigned char* Slen, unsigned char followers[][64] )
{
	for( int x=255; x>=0; x-- )
	{
		Slen[x] = getbits(CZ, 6);
		for( int i=0; i<Slen[x]; i++ )
			followers[x][i] = getbits(CZ, 8);
	}
}

void Unreduce( struct CZipTool *CZ, unsigned int usz, unsigned int csz, int factor )
{
	unsigned char* outbuf = CZ->common_buf;
	unsigned char* outpos=outbuf;
	memset( outbuf,0,0x4000 );
	int left = (signed)usz;
#define RED_FLUSH() zipwrite(CZ, outbuf,outpos-outbuf), outpos=outbuf;
#define RED_OUTC(c) {(*outpos++)=c; left--; if(outpos-outbuf==0x4000) RED_FLUSH(); }
	//-------------------------------------------------------------------

	static const int Length_table[] = {0, 0x7f, 0x3f, 0x1f, 0x0f};
	static const int D_shift[] = {0, 0x07, 0x06, 0x05, 0x04};
	static const int D_mask[]  = {0, 0x01, 0x03, 0x07, 0x0f};
	static const int B_table[] = {
		8, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5,
		5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8
	};

	initbits(CZ);

	unsigned char followers[256][64];
	unsigned char Slen[256];
	LoadFollowers( CZ, Slen, followers );

	int l_stage = 0;
	int l_char  = 0;
	int ch, V=0, Len=0;

	while( !CZ->bits_eof && left>0 )
	{
		if( Slen[l_char] == 0 )
			ch = getbits(CZ, 8);
		else
		{
			if( getbits(CZ, 1) )
				ch = getbits(CZ, 8);
			else
			{
				int bitsneeded = B_table[ Slen[l_char] ];
				ch = followers[ l_char ][ getbits(CZ, bitsneeded) ];
			}
		}

		// Repeater Decode
		switch( l_stage )
		{
		case 0:
			if( ch == 0x90 )
				l_stage++;
			else
				RED_OUTC( ch )
				break;

		case 1:
			if( ch == 0x00 )
			{
				RED_OUTC(0x90)
				l_stage = 0;
			}
			else
			{
				V   = ch;
				Len = V & Length_table[factor];
				if( Len == Length_table[factor] )
					l_stage = 2;
				else
					l_stage = 3;
			}
			break;

		case 2:
			Len += ch;
			l_stage++;
			break;

		case 3:
		{
			int i = Len+3;
			int offset = (((V>>D_shift[factor]) & D_mask[factor]) << 8) + ch + 1;
			int op = (((outpos-outbuf)-offset) & 0x3fff);

			while( i-- )
			{
				RED_OUTC( outbuf[op++] );
				op &= 0x3fff;
			}

			l_stage = 0;
		}
		break;
		}
		l_char = ch;
	}

	RED_FLUSH();

#undef RED_FLUSH
#undef RED_OUTC
}

////////////////////////////////////////////////////////////////
// Implode lz77 + shanon-fano ? PkZip 1.x
////////////////////////////////////////////////////////////////

void Explode( struct CZipTool *CZ, unsigned int usz, unsigned int csz, bool _8k, bool littree )
{
	unsigned char* outbuf = CZ->common_buf;
	unsigned char* outpos=outbuf;
	memset( outbuf,0,0x4000 );
	int left = (signed)usz;

#define EXP_FLUSH() zipwrite(CZ, outbuf,outpos-outbuf), outpos=outbuf;
#define EXP_OUTC(c) {(*outpos++)=c; left--; if(outpos-outbuf==0x4000) EXP_FLUSH(); }
	//--------------------------------------------------------------------

	unsigned char ch;
	initbits(CZ);

	int dict_bits     = ( _8k ? 7 : 6);
	int min_match_len = (littree ? 3 : 2);
	struct sf_tree lit_tree;
	struct sf_tree length_tree;
	struct sf_tree distance_tree;

	if( littree )
		LoadTree( CZ, &lit_tree, 256 );
	LoadTree( CZ, &length_tree, 64 );
	LoadTree( CZ, &distance_tree, 64 );

	while( !CZ->bits_eof && left>0 )
	{
		if( getbits(CZ, 1) )
		{
			if( littree )
				ch = ReadTree( CZ, &lit_tree );
			else
				ch = getbits( CZ, 8);

			EXP_OUTC( ch );
		}
		else
		{
			int Distance = getbits(CZ, dict_bits);
			Distance |= ( ReadTree(CZ, &distance_tree) << dict_bits );

			int Length = ReadTree( CZ, &length_tree );

			if( Length == 63 )
				Length += getbits(CZ, 8);
			Length += min_match_len;

			int op = (((outpos-outbuf)-(Distance+1)) & 0x3fff);
			while( Length-- )
			{
				EXP_OUTC( outbuf[op++] );
				op &= 0x3fff;
			}
		}
	}

	EXP_FLUSH();

#undef EXP_OUTC
#undef EXP_FLUSH
}

void SortLengths( struct CZipTool *CZ, struct sf_tree* tree )
{
	int gap,a,b;
	struct sf_entry t;
	bool noswaps;

	gap = tree->entries >> 1;

	do
	{
		do
		{
			noswaps = true;
			for( int x=0; x<=(tree->entries - 1)-gap; x++ )
			{
				a = tree->entry[x].BitLength;
				b = tree->entry[x + gap].BitLength;
				if( (a>b) || ((a==b) && (tree->entry[x].Value > tree->entry[x + gap].Value)) )
				{
					t = tree->entry[x];
					tree->entry[x] = tree->entry[x + gap];
					tree->entry[x + gap] = t;
					noswaps = false;
				}
			}
		} while (!noswaps);

		gap >>= 1;
	} while( gap > 0 );
}

void ReadLengths( struct CZipTool *CZ, struct sf_tree* tree )
{
	int treeBytes,i,num,len;

	treeBytes = getbits(CZ, 8) + 1;
	i = 0;

	tree->MaxLength = 0;

	while( treeBytes-- )
	{
		len = getbits(CZ, 4)+1;
		num = getbits(CZ, 4)+1;

		while( num-- )
		{
			if( len > tree->MaxLength )
				tree->MaxLength = len;
			tree->entry[i].BitLength = len;
			tree->entry[i].Value = i;
			i++;
		}
	}
}

void GenerateTrees( struct CZipTool *CZ, struct sf_tree* tree )
{
	uint16_t Code = 0;
	int CodeIncrement = 0, LastBitLength = 0;

	int i = tree->entries - 1;   // either 255 or 63
	while( i >= 0 )
	{
		Code += CodeIncrement;
		if( tree->entry[i].BitLength != LastBitLength )
		{
			LastBitLength = tree->entry[i].BitLength;
			CodeIncrement = 1 << (16 - LastBitLength);
		}

		tree->entry[i].Code = Code;
		i--;
	}
}

void ReverseBits( struct CZipTool *CZ, struct sf_tree* tree )
{
	for( int i=0; i<=tree->entries-1; i++ )
	{
		uint16_t o = tree->entry[i].Code,
		         v = 0,
		         mask = 0x0001,
		         revb = 0x8000;

		for( int b=0; b!=16; b++ )
		{
			if( (o&mask) != 0 )
				v = v | revb;

			revb = (revb >> 1);
			mask = (mask << 1);
		}

		tree->entry[i].Code = v;
	}
}

void LoadTree( struct CZipTool *CZ, struct sf_tree* tree, int treesize )
{
	tree->entries = treesize;
	ReadLengths( CZ, tree );
	SortLengths( CZ, tree );
	GenerateTrees( CZ, tree );
	ReverseBits( CZ, tree );
}

int ReadTree( struct CZipTool *CZ, struct sf_tree* tree )
{
	int bits=0, cur=0, b;
	uint16_t cv=0;

	while( true )
	{
		b = getbits(CZ, 1);
		cv = cv | (b << bits);
		bits++;

		while( tree->entry[cur].BitLength < bits )
		{
			cur++;
			if( cur >= tree->entries )
				return -1;
		}

		while( tree->entry[cur].BitLength == bits )
		{
			if( tree->entry[cur].Code == cv )
				return tree->entry[cur].Value;
			cur++;
			if( cur >= tree->entries )
				return -1;
		}
	}
}

// ----

/*
   void FS_zipgetFileList(FS_AddFile AddFile, char *zipfile, unsigned char *zipbuf, unsigned int zipsize)
   {
   CZipTool *SZ;
   char file[256];
   int i;
   SZ = new CZipTool(zipbuf, zipsize);
   for(i=0;i<SZ->zipnbrc;i++) {
    sprintf(file,"%s/%s", zipfile, SZ->zipdirc[i].name);
    strtolower(file);
    AddFile(1, file);
    }
   }
 */

unsigned char *unzip(unsigned char *zipbuf, unsigned int zipsize, char *filename, unsigned int *size)
{
	struct CZipTool CZ;
	unsigned char *dBuffer;

	CZipToolInit(&CZ, zipbuf, zipsize);
	ReadFromZIP(&CZ, filename, size, &dBuffer);
	CZipToolClean(&CZ);

	return dBuffer;
}

// -- INDEP TOOLS ----------------------------------------------------------------
